"""束。1 つの (問題, 環境, CPU モデル) を 1 本のジョブで一度に測った記録の集合。

同じ CPU モデルでも VM ごとに速さが 2 割ほど違う (2026-09-22 に gf2-64-pow で確認)。
variant 同士の差 (5 から 10 パーセント) より大きいので、別のジョブで測った記録を
1 つの順位表に並べても比べられない。順位を出す問題 (harness.kind = "base") は
提出を束にして同じジョブで測り直し、順位表はいちばん新しい束だけを並べる。

束の id は CI では "<run id>/<組>/<ジョブ番号>"、手元では "local/<12 桁>"。同じジョブ
は組の全環境を測るので id は環境をまたいで同じ文字列になるが、束の同一性は
(問題, 環境, モデル, id) で見る。

測り直しの条件は 1 つ。その (問題, 環境, モデル) のいちばん新しい束が、今の全提出の
現行のキーの記録を全部含んでいなければ、全提出を同じジョブで測り直す。提出を
足したとき、ヘッダを直して閉包が動いたとき、テストデータが入れ替わったとき、
書き換えを元に戻して古いキーが復活したとき、のどれもこれで覆う。束の無い古い
記録は「束が無い」なので、最初の 1 回は全部が測り直しになる。

記録の正しさ (現行の AC) は記録ごとの判定のままで、束は順位表だけの話。raw の
問題 (提出が 1 本で順位が無い) は束を作らず、今までどおりキーの有無で測る。
設計は my-docs の「procon-judge の順位表を束で測る設計」。
"""

from __future__ import annotations

import uuid
from collections.abc import Iterable
from dataclasses import dataclass

LOCAL_PREFIX = "local/"


def ci_id(run_id: str, scope: str, job: int) -> str:
    """CI のジョブの束の id。同じ run の同じジョブなら同じ VM で測っている。"""
    return f"{run_id}/{scope}/{job}"


def local_id() -> str:
    """手元の 1 回の pj run の束の id。"""
    return LOCAL_PREFIX + uuid.uuid4().hex[:12]


@dataclass(frozen=True)
class Batch:
    id: str
    # 束の時刻。中の記録のいちばん新しい timestamp。
    timestamp: str
    records: tuple[dict, ...]

    @property
    def keys(self) -> frozenset[str]:
        return frozenset(r.get("key", "") for r in self.records)

    def covers(self, keys: Iterable[str]) -> bool:
        """今の全提出のキーがこの束に揃っているか。揃っていれば測り直さない。"""
        have = self.keys
        return all(key in have for key in keys)


def newest(records: Iterable[dict], *, env: str, cpu_model: str) -> Batch | None:
    """(環境, モデル) のいちばん新しい束。束の無い記録は数えない。

    同じ時刻が並んだときは id で決めて、どこで読んでも同じ束を選ぶ。
    """
    return index(r for r in records if r.get("env") == env and r.get("cpu_model") == cpu_model).get(
        (env, cpu_model)
    )


def index(records: Iterable[dict]) -> dict[tuple[str, str], Batch]:
    """(環境, モデル) ごとのいちばん新しい束。"""
    groups: dict[tuple[str, str, str], list[dict]] = {}
    for record in records:
        batch = record.get("batch")
        if not batch:
            continue
        combo = (record.get("env", ""), record.get("cpu_model", ""), batch)
        groups.setdefault(combo, []).append(record)
    best: dict[tuple[str, str], Batch] = {}
    for (env, cpu_model, batch), rows in groups.items():
        candidate = Batch(
            id=batch,
            timestamp=max(r.get("timestamp") or "" for r in rows),
            records=tuple(rows),
        )
        current = best.get((env, cpu_model))
        if current is None or (candidate.timestamp, candidate.id) > (current.timestamp, current.id):
            best[(env, cpu_model)] = candidate
    return best
