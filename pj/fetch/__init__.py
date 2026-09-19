"""テストデータの取得。

`.cache/testcases/<ハッシュ>/` に in/out のペアと manifest.json を置く。
ハッシュは source と name から作る (取得する前に引ける必要があるため)。
内容から作る cases_hash とは別物。
"""

from __future__ import annotations

import hashlib
import json
import shutil
import sys
from dataclasses import dataclass
from datetime import UTC, datetime
from pathlib import Path

from ..paths import TESTCASE_CACHE_DIR
from ..problem import Problem
from . import mirror

MANIFEST_NAME = "manifest.json"


class FetchError(Exception):
    """テストデータを用意できなかったときに投げる。"""


@dataclass(frozen=True)
class Case:
    name: str
    in_path: Path
    out_path: Path


@dataclass(frozen=True)
class Testcases:
    dir: Path
    cases: tuple[Case, ...]
    cases_hash: str

    @property
    def count(self) -> int:
        return len(self.cases)

    def checker_source(self) -> Path | None:
        path = self.dir / "checker.cpp"
        return path if path.is_file() else None


def needs_testdata(problem: Problem) -> bool:
    """このレコードを出すのにテストデータが要るか。"""
    return problem.compare.kind != "compile_only" and problem.testdata.source != "none"


def cached_cases_hash(problem: Problem) -> str | None:
    """取得済みなら cases_hash を返す。原本を叩きに行かない。

    キーの計算に cases_hash が要るが、スキップの判定のために毎回落としたくない。
    手元のキャッシュ (CI では actions/cache) に manifest があれば、それで足りる。
    """
    if not needs_testdata(problem):
        return ""
    manifest = cache_dir_for(problem) / MANIFEST_NAME
    if not manifest.is_file():
        return None
    try:
        return json.loads(manifest.read_text())["cases_hash"]
    except (OSError, json.JSONDecodeError, KeyError):
        return None


def cache_dir_for(problem: Problem) -> Path:
    """取得前に引けるキャッシュ先。source と name だけから決める。"""
    td = problem.testdata
    if td.source == "local":
        # local は name を持たない。ジェネレータと参照実装が変わったら別のキャッシュにする。
        parts = [
            "local",
            problem.id,
            str(td.count),
            _sha256_file(problem.dir / td.generator),
            _sha256_file(problem.dir / td.reference),
        ]
    else:
        parts = [td.source, td.name]
    digest = hashlib.sha256("\n".join(parts).encode()).hexdigest()[:16]
    return TESTCASE_CACHE_DIR / digest


def _sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def collect_cases(directory: Path) -> tuple[Case, ...]:
    cases = []
    for in_path in sorted(directory.glob("*.in")):
        out_path = in_path.with_suffix(".out")
        if out_path.is_file():
            cases.append(Case(name=in_path.stem, in_path=in_path, out_path=out_path))
    return tuple(cases)


def compute_cases_hash(cases: tuple[Case, ...]) -> str:
    """ケースの内容から計算する。取得経路に依存させてはいけない。"""
    h = hashlib.sha256()
    for case in sorted(cases, key=lambda c: c.name):
        h.update(case.name.encode())
        h.update(b"\0")
        h.update(_sha256_file(case.in_path).encode())
        h.update(b"\0")
        h.update(_sha256_file(case.out_path).encode())
        h.update(b"\0")
    return h.hexdigest()[:16]


def write_manifest(directory: Path, cases: tuple[Case, ...], source: str, name: str) -> str:
    cases_hash = compute_cases_hash(cases)
    manifest = {
        "source": source,
        "name": name,
        "fetched_at": datetime.now(UTC).isoformat(timespec="seconds"),
        "cases_hash": cases_hash,
        "cases": [
            {
                "name": c.name,
                "in_bytes": c.in_path.stat().st_size,
                "out_bytes": c.out_path.stat().st_size,
                "in_sha256": _sha256_file(c.in_path),
                "out_sha256": _sha256_file(c.out_path),
            }
            for c in cases
        ],
    }
    (directory / MANIFEST_NAME).write_text(json.dumps(manifest, indent=1) + "\n")
    return cases_hash


def replace_dir(tmp_dir: Path, dest: Path) -> None:
    if dest.exists():
        shutil.rmtree(dest)
    dest.parent.mkdir(parents=True, exist_ok=True)
    tmp_dir.rename(dest)


def ensure(
    problem: Problem,
    *,
    refresh: bool = False,
    allow_mirror: bool = True,
    push_mirror: bool = True,
) -> Testcases:
    """テストデータを用意して返す。

    探索の順は、手元のキャッシュ (CI では actions/cache が復元する)、保管庫、
    原本。原本まで行ったら、成功したあとに保管庫へ上げる。
    """
    source = problem.testdata.source
    if source == "none":
        return Testcases(dir=cache_dir_for(problem), cases=(), cases_hash="")

    dest = cache_dir_for(problem)
    manifest_path = dest / MANIFEST_NAME
    if not refresh and manifest_path.is_file():
        manifest = json.loads(manifest_path.read_text())
        cases = collect_cases(dest)
        if cases and len(cases) == len(manifest["cases"]):
            return Testcases(dir=dest, cases=cases, cases_hash=manifest["cases_hash"])

    # 保管庫から取れるなら原本を叩かない。レート制限と障害を経路から外す。
    if allow_mirror and mirror.should_mirror(problem) and mirror.available():
        try:
            if mirror.pull(problem, dest):
                return _finish(problem, dest, source)
        except mirror.MirrorError as e:
            print(f"  保管庫から取れませんでした: {e}", file=sys.stderr)

    _fetch_from_origin(problem, dest)
    result = _finish(problem, dest, source)

    # 一度保管すれば、次からは原本を叩かない。
    if push_mirror and mirror.should_mirror(problem) and mirror.available():
        try:
            mirror.push(problem, dest)
        except mirror.MirrorError as e:
            print(f"  保管庫へ上げられませんでした: {e}", file=sys.stderr)

    return result


def _fetch_from_origin(problem: Problem, dest: Path) -> None:
    source = problem.testdata.source
    if source == "library_checker":
        from . import library_checker

        library_checker.fetch(problem, dest)
    elif source == "aoj":
        from . import aoj

        aoj.fetch(problem, dest)
    elif source == "yukicoder":
        from . import yukicoder

        yukicoder.fetch(problem, dest)
    elif source == "manual":
        from . import manual

        manual.fetch(problem, dest)
    else:
        raise FetchError(f"testdata.source {source!r} の取得はまだ実装していません")


def _finish(problem: Problem, dest: Path, source: str) -> Testcases:
    """取れたものを数えて manifest を書き直す。

    cases_hash はケースの内容から計算する。保管庫と原本のどちらから取っても
    同じ値になる必要がある。
    """
    cases = collect_cases(dest)
    if not cases:
        raise FetchError(f"{dest} にケースがありません")
    cases_hash = write_manifest(dest, cases, source, problem.testdata.name)
    return Testcases(dir=dest, cases=cases, cases_hash=cases_hash)


def import_dir(problem: Problem, source_dir: Path) -> Testcases:
    """手元で落としたテストデータを取り込む。"""
    pairs = collect_cases(source_dir)
    if not pairs:
        raise FetchError(f"{source_dir} に .in と .out の組がありません")

    dest = cache_dir_for(problem)
    tmp_dir = Path(str(dest) + ".tmp")
    if tmp_dir.exists():
        shutil.rmtree(tmp_dir)
    tmp_dir.mkdir(parents=True)
    for case in pairs:
        shutil.copy2(case.in_path, tmp_dir / case.in_path.name)
        shutil.copy2(case.out_path, tmp_dir / case.out_path.name)
    replace_dir(tmp_dir, dest)
    return _finish(problem, dest, problem.testdata.source)
