"""採点レコード。1 レコードが 1 行の JSON。"""

from __future__ import annotations

import json
import subprocess
from dataclasses import asdict, dataclass, field
from datetime import UTC, datetime

from .paths import LIB_DIR, ROOT

STATUSES = ("AC", "WA", "TLE", "MLE", "RE", "CE")


@dataclass
class FailedCase:
    name: str
    status: str
    time_ms: int
    memory_kb: int
    detail: str


@dataclass
class Record:
    key: str
    problem: str
    submission: str
    status: str
    env: str
    cpu_arch: str
    cpu_model: str
    compiler_version: str
    cxxflags: str
    cases_hash: str
    case_count: int
    submission_hash: str
    includes: list[str]
    library_sha: str | None
    judge_sha: str | None
    time_max_ms: int
    time_total_ms: int
    algo_time_max_ns: int | None
    algo_time_total_ns: int | None
    memory_max_kb: int
    source_bytes: int
    binary_bytes: int | None
    # キーの残りの成分。参考に落ちたとき、どの成分が動いたかを見分けるために持つ。
    harness_hash: str = ""
    problem_hash: str = ""
    # 閉包のファイルごとの正規化後ハッシュ (短縮)。提出側とハーネス側の両方。
    # 参考に落ちた理由をファイル名で言うために持つ。古い記録には無い。
    file_hashes: dict[str, str] = field(default_factory=dict)
    failed_case: FailedCase | None = None
    timestamp: str = field(
        default_factory=lambda: datetime.now(UTC)
        .replace(microsecond=0)
        .isoformat()
        .replace("+00:00", "Z")
    )

    def to_json(self) -> str:
        return json.dumps(asdict(self), ensure_ascii=False)


def _head_sha(directory) -> str | None:
    try:
        proc = subprocess.run(
            ["git", "rev-parse", "HEAD"],
            cwd=directory, capture_output=True, text=True, check=False, timeout=10,
        )
    except (subprocess.SubprocessError, OSError):
        return None
    return proc.stdout.strip() if proc.returncode == 0 else None


def judge_sha() -> str | None:
    """このリポジトリの HEAD。コミットが無ければ None。"""
    return _head_sha(ROOT)


def library_sha() -> str | None:
    """lib/ に取ってきたライブラリの HEAD。取得していなければ None。"""
    if not (LIB_DIR / ".git").exists():
        return None
    return _head_sha(LIB_DIR)
