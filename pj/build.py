"""コンパイル。"""

from __future__ import annotations

import shlex
import shutil
import subprocess
import time
from dataclasses import dataclass
from pathlib import Path

from .environment import Environment
from .paths import BUILD_CACHE_DIR, HARNESS_DIR, LIB_DIR, PROBLEMS_DIR, ROOT, SIMDE_DIR
from .problem import Problem

# constexpr を重く回す実装や -flto のリンクで伸びるので上限を置く。超えたら CE。
COMPILE_TIMEOUT_SEC = 180


@dataclass(frozen=True)
class BuildResult:
    ok: bool
    binary: Path | None
    cxxflags: str
    command: list[str]
    seconds: float
    log: str


# キーの材料の中で、問題自身のディレクトリの -I を置き換える印。問題をどこに置くかを
# キーから切り離すためで、これが無いと problems/ の下でディレクトリを動かしただけで
# その問題の記録が全部測り直しになる。記録の cxxflags には実際のパスをそのまま残す。
PROBLEM_DIR_TOKEN = "@problem"


def effective_cxxflags(env: Environment, problem: Problem) -> str:
    """environments.toml のフラグに -I を足したもの。実際のコンパイルに使う。

    記録の cxxflags にはこの文字列をそのまま入れる。include 閉包を辿るときの探索パス
    もこれと同じにする。キーの材料は key_cxxflags の方で、問題のディレクトリだけが
    置き換わっている。
    """
    return _flags(env, problem, for_key=False)


def key_cxxflags(env: Environment, problem: Problem) -> str:
    """キーの材料にするフラグ。問題自身のディレクトリの -I を PROBLEM_DIR_TOKEN にしたもの。

    run と Freshness の両方がこれを使う。二度書くと片方を直し忘れる。
    """
    return _flags(env, problem, for_key=True)


def _flags(env: Environment, problem: Problem, *, for_key: bool) -> str:
    flags = shlex.split(env.cxxflags)
    for directory in include_dirs(problem):
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
    """
    return [LIB_DIR, problem.dir, HARNESS_DIR, PROBLEMS_DIR, SIMDE_DIR]


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
) -> BuildResult:
    """翻訳単位を 1 つコンパイルする。

    kind = "base" なら base.cpp を、提出のパスを SUBMISSION_HPP に渡して組む。
    kind = "raw" なら提出そのものが翻訳単位。
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
    cmd += ["-o", str(binary), str(source)]

    t0 = time.monotonic()
    try:
        proc = subprocess.run(
            cmd, cwd=ROOT, capture_output=True, text=True, check=False,
            timeout=COMPILE_TIMEOUT_SEC,
        )
    except subprocess.TimeoutExpired:
        return BuildResult(
            ok=False, binary=None, cxxflags=cxxflags, command=cmd,
            seconds=time.monotonic() - t0,
            log=f"コンパイルが {COMPILE_TIMEOUT_SEC} 秒を超えました",
        )
    except OSError as e:
        return BuildResult(
            ok=False, binary=None, cxxflags=cxxflags, command=cmd,
            seconds=time.monotonic() - t0, log=str(e),
        )
    seconds = time.monotonic() - t0

    log = (proc.stdout + proc.stderr).strip()
    if proc.returncode != 0 or not binary.is_file():
        return BuildResult(
            ok=False, binary=None, cxxflags=cxxflags, command=cmd,
            seconds=seconds, log=log,
        )
    return BuildResult(
        ok=True, binary=binary, cxxflags=cxxflags, command=cmd,
        seconds=seconds, log=log,
    )


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
