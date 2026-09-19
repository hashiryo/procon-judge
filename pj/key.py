"""キーの計算。

このキーの記録が既にあれば実行をスキップする。整形だけの変更で走り直さないよう、
ハッシュを取る前に行末空白と空行を落とす。コメント除去は入れていない
(正しく作るには C++ の字句解析器が要る)。

提出のパスもキーに入れる。submission_hash は中身だけから作るので、バイト単位で
同じ提出が 2 本あると同じ値になる。提出ページはパスごとに描くので、パスが違えば
別の記録が要る。
"""

from __future__ import annotations

import hashlib
import json
from collections.abc import Sequence
from dataclasses import dataclass
from pathlib import Path

from .include import Closure, closure
from .problem import Problem

SEP = "\0"
EMPTY_HASH = hashlib.sha256(b"").hexdigest()


@dataclass(frozen=True)
class SubmissionKey:
    submission_hash: str
    includes: tuple[str, ...]
    unresolved: tuple[str, ...]


def normalize(text: str) -> str:
    """行末空白除去 + 空行除去。"""
    lines = (line.rstrip() for line in text.splitlines())
    return "\n".join(line for line in lines if line)


def _normalized_file(path: Path) -> str:
    try:
        return normalize(path.read_text(errors="replace"))
    except OSError:
        return ""


def _sha256(*parts: str) -> str:
    return hashlib.sha256(SEP.join(parts).encode()).hexdigest()


def submission_hash(
    source: Path, search_paths: Sequence[Path]
) -> SubmissionKey:
    """提出のソースと include 閉包からハッシュを作る。"""
    found: Closure = closure(source, search_paths)
    parts = [_normalized_file(source)]
    for label, path in zip(found.labels, found.files, strict=True):
        parts.append(label)
        parts.append(_normalized_file(path))
    return SubmissionKey(
        submission_hash=_sha256(*parts),
        includes=found.labels,
        unresolved=found.unresolved,
    )


def harness_hash(problem: Problem, search_paths: Sequence[Path] = ()) -> str:
    """base.cpp とその include 閉包からハッシュを作る。

    kind = "raw" の問題は base.cpp を持たないので空文字列のハッシュにする。

    閉包まで見るのは、共有のハーネスヘッダを書き換えたときに測り直しを
    起こすため。名指しで足していた問題ごとの common.hpp も、base.cpp が
    include していれば閉包から入る。
    """
    if problem.harness_kind != "base":
        return EMPTY_HASH
    found: Closure = closure(problem.base_cpp, search_paths)
    parts = [_normalized_file(problem.base_cpp)]
    for label, path in zip(found.labels, found.files, strict=True):
        parts.append(label)
        parts.append(_normalized_file(path))
    return _sha256(*parts)


def problem_hash(problem: Problem) -> str:
    """実行に影響する項目だけから作る。title のような表示用の項目は外す。

    既定値を埋めたあとの値を使う。problem.toml に既定値と同じ行を足しただけで
    走り直すのを避けるため。
    """
    payload = {
        "limits": {
            "tle_sec": problem.limits.tle_sec,
            "mle_mb": problem.limits.mle_mb,
        },
        "harness": {"kind": problem.harness_kind},
        "testdata": {
            "source": problem.testdata.source,
            "name": problem.testdata.name,
            "generator": problem.testdata.generator,
            "count": problem.testdata.count,
            "reference": problem.testdata.reference,
        },
        "compare": {"kind": problem.compare.kind},
    }
    canonical = json.dumps(
        payload, sort_keys=True, separators=(",", ":"), ensure_ascii=False
    )
    return hashlib.sha256(canonical.encode()).hexdigest()


def compute(
    *,
    submission: str,
    submission_hash: str,
    harness_hash: str,
    problem_hash: str,
    cases_hash: str,
    env: str,
    compiler_version: str,
    cxxflags: str,
    cpu_model: str,
) -> str:
    return _sha256(
        submission,
        submission_hash,
        harness_hash,
        problem_hash,
        cases_hash,
        env,
        compiler_version,
        cxxflags,
        cpu_model,
    )
