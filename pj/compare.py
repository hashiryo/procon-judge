"""出力比較。"""

from __future__ import annotations

import math
import subprocess
from dataclasses import dataclass
from pathlib import Path

CHECKER_TIMEOUT_SEC = 60

# failed_case に入れる差分の長さ。失敗した記録だけが持つので、提出ページで読める
# 長さまで残す。
DIFF_HEAD_CHARS = 2000


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


def _as_float(token: str) -> float | None:
    try:
        value = float(token)
    except ValueError:
        return None
    return value if math.isfinite(value) else None


def compare_float(
    actual_path: Path, expected_path: Path, *, abs_tol: float, rel_tol: float
) -> CompareResult:
    """トークン比較で、数値は誤差を許す。

    絶対誤差か相対誤差のどちらかが収まれば同じとみなす。AtCoder や
    competitive-verifier の ERROR と同じ規則。数値でないトークンは文字列で比べる。
    """
    actual, expected = _tokens(actual_path), _tokens(expected_path)
    for i, (a, e) in enumerate(zip(actual, expected)):
        if a == e:
            continue
        x, y = _as_float(a), _as_float(e)
        if x is None or y is None:
            return CompareResult(
                False, f"token {i}: expected {_abbrev(e)!r}, found {_abbrev(a)!r}"
            )
        diff = abs(x - y)
        if diff <= abs_tol or diff <= rel_tol * abs(y):
            continue
        return CompareResult(
            False,
            f"token {i}: expected {_abbrev(e)}, found {_abbrev(a)} (diff {diff:.3g}, "
            f"abs_tol {abs_tol:g}, rel_tol {rel_tol:g})",
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
    abs_tol: float = 0.0,
    rel_tol: float = 0.0,
) -> CompareResult:
    if kind == "tokens":
        return compare_tokens(actual_path, expected_path)
    if kind == "float":
        return compare_float(actual_path, expected_path, abs_tol=abs_tol, rel_tol=rel_tol)
    if kind == "checker":
        if checker is None:
            return CompareResult(False, "checker が用意できていません")
        return compare_checker(checker, input_path, actual_path, expected_path)
    raise ValueError(f"compare.kind {kind!r} は実行時の比較に使えません")
