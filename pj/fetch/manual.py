"""手で取り込む取得元。

LOJ、IOI、CSES、Codeforces のようにリアルタイムで取れないものに使う。原本は
叩かない。手元で落としたものを pj testdata import で取り込んで、
pj mirror push で保管庫へ上げる。以降は保管庫から取る。
"""

from __future__ import annotations

from pathlib import Path

from ..problem import Problem
from . import FetchError


def fetch(problem: Problem, dest: Path) -> None:
    raise FetchError(
        f"{problem.id}: source = 'manual' は原本を叩きません。"
        f"手元で落として `pj testdata import --problem {problem.id} --dir PATH` で"
        f"取り込んでから、`pj mirror push --problem {problem.id}` で上げてください"
    )
