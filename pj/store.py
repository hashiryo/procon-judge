"""記録の読み書き。

問題ごとに 1 ファイルの JSON Lines。問題をまたぐ追記が衝突しないのと、
サイトが必要とする単位と一致するのが理由。M3 でこのディレクトリが
results ブランチの作業ツリーになる。
"""

from __future__ import annotations

import json
import sys
from collections.abc import Iterator
from pathlib import Path

from .record import Record


class Store:
    def __init__(self, root: Path) -> None:
        self.root = root

    @property
    def problems_dir(self) -> Path:
        return self.root / "problems"

    def path_for(self, problem_id: str) -> Path:
        return self.problems_dir / f"{problem_id}.jsonl"

    def problem_ids(self) -> list[str]:
        if not self.problems_dir.is_dir():
            return []
        return sorted(p.stem for p in self.problems_dir.glob("*.jsonl"))

    def read(self, problem_id: str) -> Iterator[dict]:
        path = self.path_for(problem_id)
        if not path.is_file():
            return
        with path.open() as f:
            for number, line in enumerate(f, start=1):
                line = line.strip()
                if not line:
                    continue
                try:
                    yield json.loads(line)
                except json.JSONDecodeError:
                    # 書き込みの途中で落ちた行が残ることがある。読み飛ばして知らせる。
                    print(f"warning: {path}:{number} を読めません", file=sys.stderr)

    def keys(self) -> set[str]:
        """記録済みのキー。スキップの判定に使う。"""
        found: set[str] = set()
        for problem_id in self.problem_ids():
            for record in self.read(problem_id):
                key = record.get("key")
                if key:
                    found.add(key)
        return found

    def append(self, record: Record) -> None:
        """記録は必ず新しいキーのときだけ作られるので、重複の確認はしない。"""
        path = self.path_for(record.problem)
        path.parent.mkdir(parents=True, exist_ok=True)
        with path.open("a") as f:
            f.write(record.to_json() + "\n")

    def count(self) -> int:
        return sum(1 for pid in self.problem_ids() for _ in self.read(pid))
