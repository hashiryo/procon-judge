"""pj try。手元で組んで、自分で用意した入力で走らせる。printf デバッグのための口。

pj repro が決まったテストデータで判定するのに対して、こちらは判定しない。標準入力は
ファイルか端末から渡し、stdout と stderr は捕まえずに端末へそのまま流す。記録は書かない。

Library の include/ を探索パスの最後に足し、__LOCAL を立てる。debug.hpp の debug(...) と、
Apple clang に無い bits/stdc++.h のシムがそのまま使える。SIMDe は third_party/simde の方が
先に見つかるように、include/ は最後に置く。
"""

from __future__ import annotations

import hashlib
import subprocess
import sys
import time
from pathlib import Path
from typing import TextIO

from . import build as build_mod
from .environment import Environment
from .paths import CACHE_DIR, LIB_DIR
from .problem import Problem

TRY_CACHE_DIR = CACHE_DIR / "try"


def extra_flags() -> list[str]:
    """手元で試すときだけ足すフラグ。記録の条件には入れない。"""
    flags = ["-D__LOCAL"]
    include = LIB_DIR / "include"
    if include.is_dir():
        flags.append(f"-I{include}")
    return flags


def build_file(source: Path, env: Environment) -> build_mod.BuildResult:
    """問題に属さない 1 ファイルを組む。source は絶対パス。"""
    # 名前が同じで場所の違うファイルが、組んだものを取り合わないようにする。
    tag = hashlib.sha256(str(source).encode()).hexdigest()[:8]
    out_dir = TRY_CACHE_DIR / "files" / f"{source.stem}-{tag}" / env.name
    return build_mod.build_file(source, env, out_dir=out_dir, extra_flags=extra_flags())


def build_submission(problem: Problem, submission: Path, env: Environment) -> build_mod.BuildResult:
    """提出をハーネスごと組む (base なら base.cpp に SUBMISSION_HPP を渡す)。"""
    out_dir = TRY_CACHE_DIR / problem.id / env.name
    return build_mod.build(problem, submission, env, out_dir=out_dir, extra_flags=extra_flags())


def run_built(built: build_mod.BuildResult, *, input_path: Path | None, err: TextIO = sys.stderr) -> int:
    """組めていれば走らせて終了コードを返す。組めなければ 1。

    input_path が無ければ、端末の標準入力をそのまま渡す (pj try x.cpp < in.txt でもよい)。
    """
    if not built.ok:
        print(f"CE ({built.seconds:.1f}s)", file=err)
        print(built.log, file=err)
        return 1
    assert built.binary is not None
    print(f"コンパイル完了 ({built.seconds:.1f}s)", file=err)
    if built.log:
        # 警告。組めたときも出す。
        print(built.log, file=err)
    err.flush()

    stdin = input_path.open("rb") if input_path is not None else None
    t0 = time.monotonic()
    try:
        proc = subprocess.run([str(built.binary)], stdin=stdin, check=False)
    except KeyboardInterrupt:
        print("\n中断しました", file=err)
        return 130
    finally:
        if stdin is not None:
            stdin.close()
    ms = int((time.monotonic() - t0) * 1000)
    code = proc.returncode
    if code < 0:
        print(f"\nシグナル {-code} で落ちました ({ms} ms)", file=err)
        return 128 - code
    print(f"\n終了コード {code} ({ms} ms)", file=err)
    return code
