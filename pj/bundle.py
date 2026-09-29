"""pj bundle。提出とハーネスとライブラリを、判定サイトへ貼れる 1 ファイルに展開する。

引用符の include を、キーの材料にしている閉包 (include.py) と同じ探し方で解決して、その場に
中身を入れる。同じファイルは 1 回だけ入れ、`#pragma once` の行は落とす。山括弧の include は
そのまま残す。base の問題は base.cpp の SUBMISSION_HPP の選択を、提出を読む 1 行に置き換える。
raw の問題は提出そのものが入口になる。

先頭には Library の include/isa-pragma.hpp を置く。procon-judge が x64 でオプションとして
渡している土台の命令を、判定サイトではソースで宣言するためのもので、x86 の GCC のときだけ効く。
"""

from __future__ import annotations

import re
import shlex
import shutil
import subprocess
import sys
from collections.abc import Sequence
from dataclasses import dataclass
from pathlib import Path
from typing import TextIO

from . import build as build_mod
from . import include as include_mod
from . import repro as repro_mod
from .environment import CONSTEXPR_LIMITS, Environment, compiler_family
from .paths import CACHE_DIR, LIB_DIR, SIMDE_DIR
from .problem import Problem

BUNDLE_CACHE_DIR = CACHE_DIR / "bundle"
ISA_PRAGMA = LIB_DIR / "include" / "isa-pragma.hpp"
# --check で走らせるケースの数の既定。名前順の先頭から。
CHECK_CASES = 3

# 判定サイトのソースの大きさの上限 (バイト)。問題の id の接頭辞で引く。確かめたものだけを置く。
SOURCE_LIMITS = {
    "cf": ("Codeforces", 64 * 1024),
    "atcoder": ("AtCoder", 512 * 1024),
}

PRAGMA_ONCE_RE = re.compile(r"^[ \t]*#[ \t]*pragma[ \t]+once\b")
# base.cpp で提出を選ぶところ。既定の提出を置く #ifndef の囲みと、それを読む 1 行。
SELECT_DEFAULT_RE = re.compile(
    r"^[ \t]*#[ \t]*ifndef[ \t]+SUBMISSION_HPP[ \t]*\n"
    r"[ \t]*#[ \t]*define[ \t]+SUBMISSION_HPP\b[^\n]*\n"
    r"[ \t]*#[ \t]*endif\b[^\n]*\n",
    re.MULTILINE,
)
SELECT_INCLUDE_RE = re.compile(r"^[ \t]*#[ \t]*include[ \t]+SUBMISSION_HPP\b[^\n]*$", re.MULTILINE)
# 置き換えたあとに SUBMISSION_HPP が残っていないかを見る。コメントの中は数えない。
MACRO_RE = re.compile(r"^(?![ \t]*//).*\bSUBMISSION_HPP\b", re.MULTILINE)


class BundleError(Exception):
    """展開できないときに投げる。"""


@dataclass(frozen=True)
class Bundle:
    text: str
    # 中身を入れたファイル。入れた順で、入口は含まない。
    files: tuple[Path, ...]
    # 解決できずに、その行をそのまま残した引用符の include。
    unresolved: tuple[str, ...]

    @property
    def size(self) -> int:
        return len(self.text.encode())

    @property
    def crlf_size(self) -> int:
        """改行を CRLF で数えた大きさ。判定サイトの上限はこれと比べる。

        提出欄に貼ったソースは、ブラウザが改行を CRLF にして送る。AtCoder の Code Size は
        この大きさだった (2026-09-27、8,770 bytes の 259 行が 9,029 Byte)。
        """
        return self.size + self.text.count("\n")


def select_submission(base_text: str, submission: Path) -> str:
    """base.cpp の SUBMISSION_HPP の選択を、提出を読む 1 行に置き換える。

    `#ifndef SUBMISSION_HPP` から `#include SUBMISSION_HPP` までが
    `#include "submissions/x.hpp"` になる。#ifndef の囲みが無い base.cpp も受ける。
    ほかの場所でマクロを使っていると、測ったものと中身が変わるので止める。
    """
    text = base_text.replace("\r\n", "\n")
    text = SELECT_DEFAULT_RE.sub("", text, count=1)
    if len(SELECT_INCLUDE_RE.findall(text)) != 1:
        raise BundleError("base.cpp に #include SUBMISSION_HPP がちょうど 1 行ありません")
    text = SELECT_INCLUDE_RE.sub(f'#include "{submission.as_posix()}"', text)
    if MACRO_RE.search(text):
        raise BundleError("base.cpp が #include のほかでも SUBMISSION_HPP を使っているので展開できません")
    return text


def expand(
    entry_text: str,
    entry_dir: Path,
    search_paths: Sequence[Path],
    *,
    prelude: Sequence[Path] = (),
    entry: Path | None = None,
) -> Bundle:
    """入口の中身から引用符の include を辿って、その場に展開する。

    entry_dir は入口の中の include を探し始めるディレクトリで、コンパイラが翻訳単位の
    置き場所から探すのに合わせる。prelude は入口より前に置くファイル。entry は入口の
    ファイルそのもので、入口を include し返すものがあっても中身を二度入れない。
    """
    seen: set[Path] = set()
    files: list[Path] = []
    unresolved: list[str] = []
    out: list[str] = []
    if entry is not None:
        seen.add(entry.resolve())

    def emit(text: str, from_dir: Path) -> None:
        for line in text.replace("\r\n", "\n").split("\n"):
            if PRAGMA_ONCE_RE.match(line):
                continue
            m = include_mod.INCLUDE_RE.match(line)
            if m is None:
                out.append(line)
                continue
            target = m.group(1)
            path = include_mod.resolve(target, from_dir, search_paths)
            if path is None:
                # 引用符で書いた標準ライブラリ (#include "vector") はコンパイラが見つける。
                # 本当に欠けているなら --check の組み立てで分かる。
                if target not in unresolved:
                    unresolved.append(target)
                out.append(line)
                continue
            if path in seen:
                continue
            seen.add(path)
            files.append(path)
            emit(_read(path), path.parent)

    for path in prelude:
        path = path.resolve()
        if path in seen:
            continue
        seen.add(path)
        files.append(path)
        emit(_read(path), path.parent)
    emit(entry_text, entry_dir)

    text = "\n".join(out)
    if not text.endswith("\n"):
        text += "\n"
    return Bundle(text=text, files=tuple(files), unresolved=tuple(unresolved))


def _read(path: Path) -> str:
    try:
        return path.read_text()
    except (OSError, UnicodeDecodeError) as e:
        raise BundleError(f"{path} を読めません: {e}") from e


def bundle(problem: Problem, submission: Path, *, isa_pragma: Path | None = None) -> Bundle:
    """提出を 1 ファイルに展開する。submission は問題のディレクトリからの相対パス。"""
    isa_pragma = isa_pragma or ISA_PRAGMA
    if not isa_pragma.is_file():
        raise BundleError(
            f"{isa_pragma} がありません。先頭に置く土台の命令の宣言で、Library の include/ にあります"
        )
    source = problem.dir / submission
    if not source.is_file():
        raise BundleError(f"{problem.id} に提出 {submission.as_posix()} がありません")
    search_paths = build_mod.include_dirs(problem)
    if problem.harness_kind == "base":
        entry = problem.base_cpp
        text = select_submission(_read(entry), submission)
    else:
        entry = source
        text = _read(entry)
    return expand(
        text, entry.parent, search_paths, prelude=[isa_pragma], entry=entry
    )


def output_path(problem: Problem, submission: Path) -> Path:
    return BUNDLE_CACHE_DIR / problem.id / f"{submission.stem}.cpp"


def copy_to_clipboard(text: str) -> None:
    """クリップボードに入れる。macOS の pbcopy を使う。"""
    pbcopy = shutil.which("pbcopy")
    if pbcopy is None:
        raise BundleError("pbcopy がありません。--copy は macOS でだけ使えます")
    try:
        subprocess.run([pbcopy], input=text.encode(), check=True)
    except (OSError, subprocess.CalledProcessError) as e:
        raise BundleError(f"クリップボードに入れられません: {e}") from e


def source_limit(problem: Problem) -> tuple[str, int] | None:
    """元の問題の判定サイトの名前と、ソースの大きさの上限。分からなければ None。"""
    prefix = problem.id.partition("-")[0]
    return SOURCE_LIMITS.get(prefix)


def check_flags(env: Environment) -> list[str]:
    """--check で組むときのフラグ。判定サイトに近づけるため、-I も -march も付けない。

    環境の -D だけは残す。arm と macOS の環境は -DUSE_SIMDE で x86 の intrinsics を SIMDe に
    読み替えていて、落とすと x86 の intrinsics を使う提出が組めない。そのときだけ SIMDe の
    -I を足す。SIMDe は山括弧で読むので展開には入らず、判定サイトの x86 ではプリプロセッサが
    飛ばす分岐にある。

    constexpr の上限は判定サイトの最小に揃える (CONSTEXPR_LIMITS)。local の環境は c++ が
    macOS と ubuntu で別のコンパイラになり、environments.toml に書けないので、ここで
    コンパイラの系統を見て足す。
    """
    defines = [f for f in shlex.split(env.cxxflags) if f.startswith("-D")]
    flags = ["-std=gnu++23", "-O2", *CONSTEXPR_LIMITS[compiler_family(env)], *defines]
    if any(f.split("=", 1)[0] == "-DUSE_SIMDE" for f in defines):
        flags.append(f"-I{SIMDE_DIR}")
    return flags


def check(
    problem: Problem,
    submission: Path,
    path: Path,
    env: Environment,
    *,
    cases: int = CHECK_CASES,
    out: TextIO | None = None,
) -> int:
    """展開したファイルを 1 つで組み、手元のケースを先頭から cases 個走らせる。

    比べ方は pj repro と同じ。全部 AC なら 0、組めないか落ちたケースがあれば 1。
    知らせは既定で stderr に出す。標準出力には展開したものが流れているため。
    """
    out = out or sys.stderr
    work = BUNDLE_CACHE_DIR / problem.id / "check" / env.name / submission.stem
    flags = check_flags(env)
    print(f"{env.cxx} {shlex.join(flags)} で組みます", file=out)
    built = build_mod.build_standalone(path, env, flags, out_dir=work)
    if not built.ok:
        print(f"CE ({built.seconds:.1f}s)", file=out)
        print(built.log, file=out)
        return 1
    assert built.binary is not None
    print(f"コンパイル完了 ({built.seconds:.1f}s)", file=out)
    return repro_mod.run_binary(problem, built.binary, env, work, cases=cases, out=out)
