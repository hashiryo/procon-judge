"""AOJ の judgedat からテストデータを取る。

Library/scripts/lib/download.py の download_aoj を移植した。
"""

from __future__ import annotations

import json
import shutil
import sys
import urllib.error
import urllib.request
from pathlib import Path

from ..problem import Problem
from . import FetchError, replace_dir

BASE = "https://judgedat.u-aizu.ac.jp/testcases"
TIMEOUT_SEC = 60


def _get(url: str) -> bytes:
    try:
        with urllib.request.urlopen(url, timeout=TIMEOUT_SEC) as resp:
            return resp.read()
    except urllib.error.HTTPError as e:
        raise FetchError(f"{url}: HTTP {e.code} {e.reason}") from e
    except Exception as e:
        raise FetchError(f"{url}: {e}") from e


def fetch(problem: Problem, dest: Path) -> None:
    problem_id = problem.testdata.name
    headers = json.loads(_get(f"{BASE}/{problem_id}/header")).get("headers", [])
    expected = [h for h in headers if h.get("serial") is not None]
    if not expected:
        raise FetchError(f"AOJ {problem_id}: ケースの一覧が空です")

    print(f"  AOJ {problem_id} を {len(expected)} ケース落とします...", file=sys.stderr)
    tmp_dir = Path(str(dest) + ".tmp")
    if tmp_dir.exists():
        shutil.rmtree(tmp_dir)
    tmp_dir.mkdir(parents=True)

    truncated: list[str] = []
    for header in expected:
        serial = header["serial"]
        name = Path(header.get("name", f"case{serial}")).stem
        in_data = _get(f"{BASE}/{problem_id}/{serial}/in")
        out_data = _get(f"{BASE}/{problem_id}/{serial}/out")
        # judgedat は大きいケースを切り詰めて返すことがある。切り詰められた
        # データで判定すると結果の意味が変わるので、header と突き合わせる。
        if _short(in_data, header.get("inputSize")) or _short(
            out_data, header.get("outputSize")
        ):
            truncated.append(name)
        (tmp_dir / f"{name}.in").write_bytes(in_data)
        (tmp_dir / f"{name}.out").write_bytes(out_data)

    if truncated:
        print(
            f"  warning: AOJ {problem_id}: 切り詰められたケースがあります "
            f"({', '.join(truncated)})",
            file=sys.stderr,
        )
    replace_dir(tmp_dir, dest)


def _short(data: bytes, expected_size: object) -> bool:
    return isinstance(expected_size, int) and len(data) != expected_size
