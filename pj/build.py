"""コンパイル。"""

from __future__ import annotations

import json
import shlex
import shutil
import subprocess
import sys
import tempfile
import time
from collections.abc import Sequence
from dataclasses import dataclass
from pathlib import Path

from . import libraries as lib_mod
from .environment import Environment
from .paths import BUILD_CACHE_DIR, HARNESS_DIR, LIB_DIR, PROBLEMS_DIR, ROOT, SIMDE_DIR
from .problem import Problem

# constexpr を重く回す実装や -flto のリンクで伸びるので上限を置く。超えたら CE。
COMPILE_TIMEOUT_SEC = 180

# コンパイラを 1 段はさんで起こす親。時間とメモリの山を測り、打ち切りもする。pj が直接
# 起こすと、Linux では子の ru_maxrss に pj 自身のピーク RSS が乗る (compile_meter.py)。
METER = Path(__file__).with_name("compile_meter.py")
# 親が打ち切れなかったとき (親そのものが固まったとき) の保険。
METER_GRACE_SEC = 30


@dataclass(frozen=True)
class BuildResult:
    ok: bool
    binary: Path | None
    cxxflags: str
    command: list[str]
    seconds: float
    log: str
    # コンパイラとその子 (cc1plus、lto1、リンカ) のうち、いちばん大きいもののピーク RSS。
    # 測れなかったら None。
    rss_kb: int | None = None


# キーの材料の中で、問題自身のディレクトリの -I を置き換える印。問題をどこに置くかを
# キーから切り離すためで、これが無いと problems/ の下でディレクトリを動かしただけで
# その問題の記録が全部測り直しになる。記録の cxxflags には実際のパスをそのまま残す。
PROBLEM_DIR_TOKEN = "@problem"


def effective_cxxflags(env: Environment, problem: Problem) -> str:
    """environments.toml のフラグに -I を足したもの。実際のコンパイルに使う。

    記録の cxxflags にはこの文字列をそのまま入れる。include 閉包を辿るときの探索パス
    もこれと同じにする。キーの材料は key_cxxflags の方で、問題のディレクトリが印に
    置き換わり、lib/ のほかのライブラリの -I が抜けている。
    """
    return _flags(env, problem, for_key=False)


def key_cxxflags(env: Environment, problem: Problem) -> str:
    """キーの材料にするフラグ。問題自身のディレクトリの -I を PROBLEM_DIR_TOKEN にしたもの。

    lib/ のほかのライブラリ (libraries.toml の dir) の -I は入れない。ライブラリの中身は
    閉包のハッシュを通してキーに入っているし、-I を足したことで解決先が変わるなら閉包が
    変わる。入れると、ライブラリを足した瞬間に全部のキーが変わって全部が測り直しになる。
    同じ理由で -Ilib は Library を畳んだあとも残す。

    run と Freshness の両方がこれを使う。二度書くと片方を直し忘れる。
    """
    return _flags(env, problem, for_key=True)


def _flags(env: Environment, problem: Problem, *, for_key: bool) -> str:
    flags = shlex.split(env.cxxflags)
    extras = set(lib_mod.extra_dirs()) if for_key else set()
    for directory in include_dirs(problem):
        if directory in extras:
            continue
        if for_key and directory == problem.dir:
            flags.append(f"-I{PROBLEM_DIR_TOKEN}")
        else:
            flags.append(f"-I{_rel(directory)}")
    return shlex.join(flags)


def include_dirs(problem: Problem) -> list[Path]:
    """-I に渡すディレクトリ。閉包の解決もこの順で探す。

    問題のディレクトリを harness より先に置く。問題ごとに同じ名前のヘッダを
    置いたら、そちらが勝つ方が使いやすい。

    problems/ を足してあるのは、問題をまたぐヘッダ (problems/_shared/) を
    `#include "_shared/gf2-64/_common.hpp"` の形で引くため。問題を何段掘って
    置いても同じ書き方で通る。

    third_party/simde は submodule を初期化していなくても外さない。有無で外すと
    cxxflags が変わってキーが変わり、手元と CI で別の記録が増える。存在しない
    -I はコンパイラが黙って無視する。

    lib/ のほかのライブラリ (libraries.toml の dir) は最後に足す。今ある探索のどれも
    横取りしないため。置き場が無くても外さないのは SIMDe と同じ理由。
    """
    return [LIB_DIR, problem.dir, HARNESS_DIR, PROBLEMS_DIR, SIMDE_DIR, *lib_mod.extra_dirs()]


def comparable_cxxflags(cxxflags: str) -> str:
    """lib/ のほかのライブラリの -I を落としたフラグ。記録の cxxflags どうしを比べるときに使う。

    その -I はキーの材料に入っていないので、有無だけでフラグが変わったとは言わない。
    ライブラリを足す前に測った記録には入っておらず、足したあとの記録には入っている。
    """
    extras = {f"-I{_rel(d)}" for d in lib_mod.extra_dirs()}
    try:
        flags = shlex.split(cxxflags)
    except ValueError:
        return cxxflags
    return shlex.join(f for f in flags if f not in extras)


def _rel(path: Path) -> str:
    try:
        return str(path.relative_to(ROOT))
    except ValueError:
        return str(path)


def build(
    problem: Problem,
    submission: Path,
    env: Environment,
    *,
    out_dir: Path | None = None,
    extra_flags: Sequence[str] = (),
) -> BuildResult:
    """翻訳単位を 1 つコンパイルする。

    kind = "base" なら base.cpp を、提出のパスを SUBMISSION_HPP に渡して組む。
    kind = "raw" なら提出そのものが翻訳単位。
    extra_flags は pj try が手元で試すときだけ足すもので、記録の cxxflags には入らない。
    """
    out_dir = out_dir or BUILD_CACHE_DIR / problem.id / env.name
    out_dir.mkdir(parents=True, exist_ok=True)
    binary = out_dir / (submission.stem + ".bin")
    if binary.exists():
        binary.unlink()

    cxxflags = effective_cxxflags(env, problem)
    cmd = [env.cxx, *shlex.split(cxxflags)]
    if problem.harness_kind == "base":
        cmd.append(f"-DSUBMISSION_HPP=\"{submission.as_posix()}\"")
        source = problem.base_cpp
    else:
        source = problem.dir / submission
    cmd += [*extra_flags, "-o", str(binary), str(source)]
    return _compile(cmd, binary, cxxflags)


def build_file(
    source: Path,
    env: Environment,
    *,
    out_dir: Path,
    extra_flags: Sequence[str] = (),
) -> BuildResult:
    """問題に属さない 1 ファイルを、環境のフラグと提出と同じ探索パスで組む (pj try 用)。

    問題のディレクトリが無いので、-I は lib、harness、problems、SIMDe と、lib/ のほかの
    ライブラリ。source は絶対パスで渡す。コンパイラは ROOT で動かすので、相対パスだとずれる。
    """
    out_dir.mkdir(parents=True, exist_ok=True)
    binary = out_dir / (source.stem + ".bin")
    if binary.exists():
        binary.unlink()
    flags = shlex.split(env.cxxflags)
    dirs = (LIB_DIR, HARNESS_DIR, PROBLEMS_DIR, SIMDE_DIR, *lib_mod.extra_dirs())
    flags += [f"-I{_rel(d)}" for d in dirs]
    cmd = [env.cxx, *flags, *extra_flags, "-o", str(binary), str(source)]
    return _compile(cmd, binary, shlex.join(flags))


def build_standalone(
    source: Path, env: Environment, flags: Sequence[str], *, out_dir: Path
) -> BuildResult:
    """1 ファイルを、環境のコンパイラと渡したフラグだけで組む (pj bundle --check 用)。

    environments.toml のフラグも -I も足さない。展開したファイルを判定サイトに近い形で
    組んで、include の取りこぼしを見るため。
    """
    out_dir.mkdir(parents=True, exist_ok=True)
    binary = out_dir / (source.stem + ".bin")
    if binary.exists():
        binary.unlink()
    cmd = [env.cxx, *flags, "-o", str(binary), str(source)]
    return _compile(cmd, binary, shlex.join(flags))


def _compile(cmd: list[str], binary: Path, cxxflags: str) -> BuildResult:
    def result(ok: bool, seconds: float, log: str, rss_kb: int | None = None) -> BuildResult:
        return BuildResult(
            ok=ok, binary=binary if ok else None, cxxflags=cxxflags, command=cmd,
            seconds=seconds, log=log, rss_kb=rss_kb,
        )

    timed_out = f"コンパイルが {COMPILE_TIMEOUT_SEC} 秒を超えました"
    t0 = time.monotonic()
    with tempfile.TemporaryDirectory(prefix="pj-meter-") as tmp:
        report_path = Path(tmp) / "report.json"
        wrapped = [sys.executable, str(METER), str(report_path), str(COMPILE_TIMEOUT_SEC), *cmd]
        try:
            proc = subprocess.run(
                wrapped, cwd=ROOT, capture_output=True, text=True, check=False,
                timeout=COMPILE_TIMEOUT_SEC + METER_GRACE_SEC,
            )
        except subprocess.TimeoutExpired:
            return result(False, time.monotonic() - t0, timed_out)
        except OSError as e:
            return result(False, time.monotonic() - t0, str(e))
        report = _read_report(report_path)

    seconds = report.get("seconds", time.monotonic() - t0)
    rss_kb = report.get("rss_kb")
    rss_kb = rss_kb if isinstance(rss_kb, int) and rss_kb > 0 else None
    if report.get("timed_out"):
        return result(False, seconds, timed_out, rss_kb)
    if "error" in report:
        # コンパイラが起こせなかった (見つからないなど)。
        return result(False, seconds, str(report["error"]))

    log = (proc.stdout + proc.stderr).strip()
    ok = proc.returncode == 0 and binary.is_file()
    return result(ok, seconds, log, rss_kb)


def summary(built: BuildResult) -> str:
    """ログに出すコンパイルの時間とメモリの山。例: 「8.7s, 485 MB」。"""
    if built.rss_kb is None:
        return f"{built.seconds:.1f}s"
    return f"{built.seconds:.1f}s, {built.rss_kb / 1024:.0f} MB"


def _read_report(path: Path) -> dict:
    """compile_meter.py が書いた結果。書かれていなければ空。"""
    try:
        report = json.loads(path.read_text())
    except (OSError, ValueError):
        return {}
    return report if isinstance(report, dict) else {}


def build_checker(source: Path, out: Path, env: Environment) -> Path | None:
    """チェッカをコンパイルする。

    計測対象ではないので environments.toml のフラグは使わない。testlib.h は
    警告を大量に出すうえ、-march や -flto を付ける意味がない。
    """
    if out.is_file() and out.stat().st_mtime >= source.stat().st_mtime:
        return out
    cxx = shutil.which(env.cxx) or env.cxx
    cmd = [
        cxx, "-std=c++17", "-O2", "-w",
        f"-I{source.parent}", "-o", str(out), str(source),
    ]
    try:
        proc = subprocess.run(
            cmd, capture_output=True, text=True, check=False,
            timeout=COMPILE_TIMEOUT_SEC,
        )
    except (subprocess.SubprocessError, OSError):
        return None
    if proc.returncode != 0 or not out.is_file():
        return None
    return out
