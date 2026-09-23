"""記録の読み書き。

問題ごとに 1 ファイルの JSON Lines。問題をまたぐ追記が衝突しないのと、
サイトが必要とする単位と一致するのが理由。M3 でこのディレクトリが
results ブランチの作業ツリーになる。
"""

from __future__ import annotations

import json
import sys
from collections.abc import Iterable, Iterator
from pathlib import Path

from . import batch as batch_mod
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

    def identities(self) -> set[tuple[str, str]]:
        """記録の (キー, 束)。取り込みの重複排除に使う。

        束を入れる前はキーが 1 回しか測られなかったのでキーだけで見ていた。束は
        同じキーを束ごとに 1 件ずつ作るので、束も見る。束の無い記録は空文字。
        """
        found: set[tuple[str, str]] = set()
        for problem_id in self.problem_ids():
            for record in self.read(problem_id):
                key = record.get("key")
                if key:
                    found.add((key, record.get("batch") or ""))
        return found

    def batches(self) -> dict[tuple[str, str, str], batch_mod.Batch]:
        """(問題, 環境, CPU モデル) ごとのいちばん新しい束。

        run と plan が base の問題の測り直しの条件 (最新の束が今の全提出のキーを
        揃えているか) を見るのに使う。
        """
        found: dict[tuple[str, str, str], batch_mod.Batch] = {}
        for problem_id in self.problem_ids():
            for (env, cpu_model), one in batch_mod.index(self.read(problem_id)).items():
                found[(problem_id, env, cpu_model)] = one
        return found

    def cases_hashes(self) -> dict[str, str]:
        """問題ごとの、いちばん新しい記録の cases_hash。

        テストデータを落とさずにキーを組むために借りる。走らせるものが無い
        回は、これで判断が付いてしまえば 1 バイトも取らずに済む。

        借りた値は測った当時のもので、判定サイトや上流のジェネレータが動いて
        いれば古い。だから実際に走らせるときは本物を取り直して突き合わせる。
        リポジトリの中が原因の変化は problem_hash が拾うので、ここには載らない。
        """
        latest: dict[str, tuple[str, str]] = {}
        for problem_id in self.problem_ids():
            for record in self.read(problem_id):
                value = record.get("cases_hash")
                if value is None:
                    continue
                stamp = record.get("timestamp") or ""
                if problem_id not in latest or stamp > latest[problem_id][0]:
                    latest[problem_id] = (stamp, value)
        return {pid: value for pid, (_, value) in latest.items()}

    def append(self, record: Record) -> None:
        """重複の確認はしない。同じ (キー, 束) を 2 回書くのは取り込み側が防ぐ。"""
        path = self.path_for(record.problem)
        path.parent.mkdir(parents=True, exist_ok=True)
        with path.open("a") as f:
            f.write(record.to_json() + "\n")

    def count(self) -> int:
        return sum(1 for pid in self.problem_ids() for _ in self.read(pid))

    def append_raw(self, problem_id: str, line: str) -> None:
        path = self.path_for(problem_id)
        path.parent.mkdir(parents=True, exist_ok=True)
        with path.open("a") as f:
            f.write(line + "\n")

    def absorb(self, directories: Iterable[Path]) -> tuple[int, int]:
        """他所の jsonl を取り込む。足した件数と飛ばした件数を返す。

        CI では run のジョブがアーティファクトへ記録を置いて、collect が
        ここへまとめる。ワークフローを回し直しても重ならないよう、
        既にある (キー, 束) は飛ばす。同じキーでも束が違えば別の測定なので入れる。
        """
        known = self.identities()
        added = skipped = 0
        for path in _jsonl_files(directories):
            for number, line in enumerate(path.read_text().splitlines(), start=1):
                line = line.strip()
                if not line:
                    continue
                try:
                    record = json.loads(line)
                except json.JSONDecodeError:
                    print(f"warning: {path}:{number} を読めません", file=sys.stderr)
                    continue
                key, problem = record.get("key"), record.get("problem")
                if not key or not problem:
                    print(f"warning: {path}:{number} に key か problem がありません",
                          file=sys.stderr)
                    continue
                identity = (key, record.get("batch") or "")
                if identity in known:
                    skipped += 1
                    continue
                known.add(identity)
                self.append_raw(problem, line)
                added += 1
        return added, skipped


def _jsonl_files(directories: Iterable[Path]) -> list[Path]:
    """渡されたディレクトリの下の jsonl を集める。

    アーティファクトの展開先は run のジョブごとに 1 段深くなるので、
    決め打ちせずに再帰で拾う。
    """
    found: list[Path] = []
    for directory in directories:
        if directory.is_file() and directory.suffix == ".jsonl":
            found.append(directory)
        elif directory.is_dir():
            found.extend(sorted(directory.rglob("*.jsonl")))
    return sorted(set(found))
