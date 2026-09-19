"""コンパイル。"""

from __future__ import annotations

import shlex
import shutil
import subprocess
import time
from dataclasses import dataclass
from pathlib import Path

from .environment import Environment
from .paths import BUILD_CACHE_DIR, LIB_DIR, ROOT, SIMDE_DIR
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


def effective_cxxflags(env: Environment, problem: Problem) -> str:
    """environments.toml のフラグに -I を足したもの。

    記録の cxxflags にはこの文字列をそのまま入れる。キーの一部なので、
    記録と実際のコンパイルがずれてはいけない。
    include 閉包を辿るときの探索パスもこれと同じにする。
    """
    flags = shlex.split(env.cxxflags)
    for directory in include_dirs(problem):
        flags.append(f"-I{_rel(directory)}")
    return shlex.join(flags)


def include_dirs(problem: Problem) -> list[Path]:
    """-I に渡すディレクトリ。閉包の解決もこの順で探す。

    third_party/simde は submodule を初期化していなくても外さない。有無で外すと
    cxxflags が変わってキーが変わり、手元と CI で別の記録が増える。存在しない
    -I はコンパイラが黙って無視する。
    """
    return [LIB_DIR, problem.dir, SIMDE_DIR]


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
