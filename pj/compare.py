"""出力比較。"""

from __future__ import annotations

import subprocess
from dataclasses import dataclass
from pathlib import Path

CHECKER_TIMEOUT_SEC = 60

# failed_case に入れる差分の長さ。
DIFF_HEAD_CHARS = 400


@dataclass(frozen=True)
class CompareResult:
    ok: bool
    detail: str = ""


def _tokens(path: Path) -> list[str]:
    return path.read_text(errors="replace").split()


def _abbrev(token: str, limit: int = 40) -> str:
    return token if len(token) <= limit else token[:limit] + "..."


def compare_tokens(actual_path: Path, expected_path: Path) -> CompareResult:
    """空白と改行を無視してトークン列を比較する。"""
    actual, expected = _tokens(actual_path), _tokens(expected_path)
    for i, (a, e) in enumerate(zip(actual, expected)):
        if a != e:
            return CompareResult(
                False,
                f"token {i}: expected {_abbrev(e)!r}, found {_abbrev(a)!r}",
            )
    if len(actual) != len(expected):
        return CompareResult(
            False, f"token count: expected {len(expected)}, found {len(actual)}"
        )
    return CompareResult(True, f"{len(actual)} tokens")


def compare_checker(
    checker: Path, input_path: Path, actual_path: Path, expected_path: Path
) -> CompareResult:
    """チェッカのバイナリに委ねる。引数は入力、提出の出力、期待出力の順。"""
    try:
        proc = subprocess.run(
            [str(checker), str(input_path), str(actual_path), str(expected_path)],
            capture_output=True, text=True, check=False, timeout=CHECKER_TIMEOUT_SEC,
        )
    except subprocess.TimeoutExpired:
        return CompareResult(False, f"checker が {CHECKER_TIMEOUT_SEC} 秒を超えました")
    except OSError as e:
        return CompareResult(False, f"checker を起動できません: {e}")
    message = (proc.stderr.strip() or proc.stdout.strip())[:DIFF_HEAD_CHARS]
    return CompareResult(proc.returncode == 0, message)


def compare(
    kind: str,
    *,
    input_path: Path,
    actual_path: Path,
    expected_path: Path,
    checker: Path | None = None,
) -> CompareResult:
    if kind == "tokens":
        return compare_tokens(actual_path, expected_path)
    if kind == "checker":
        if checker is None:
            return CompareResult(False, "checker が用意できていません")
        return compare_checker(checker, input_path, actual_path, expected_path)
    raise ValueError(f"compare.kind {kind!r} は実行時の比較に使えません")
