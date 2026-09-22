"""テストデータの取得。

`.cache/testcases/<source>/<name>/` に in/out のペアと manifest.json を置く。
置き場は取得元と問題の名前だけで決まる。取得する前に引ける必要があるため。
中身は関係しない。取り直して別の内容になっても、同じ場所に上書きされる。

内容から作るのは cases_hash の方で、こちらはキーの材料になる。混ぜないこと。
置き場が変わってもキーは動かないし、置き場が同じでもキーは動きうる。
`local` だけはジェネレータと参照実装の中身で置き場を分ける。同じ問題 id から
別の内容が出るため。
"""

from __future__ import annotations

import hashlib
import json
import shutil
import sys
from collections.abc import Mapping
from dataclasses import dataclass
from datetime import UTC, datetime
from pathlib import Path

from ..paths import TESTCASE_CACHE_DIR
from ..problem import Problem
from . import mirror
from .mirror import is_hidden

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
    """取得前に引けるキャッシュ先。source と name だけから決める。

    ログに出る場所なので、読める形にしてある。ハッシュにすると cases_hash と
    見分けがつかず、内容から決まっていると読み違える。
    """
    td = problem.testdata
    if td.source == "local":
        # local は判定サイトの名前を持たない。ジェネレータと参照実装が
        # 変われば別の内容が出るので、そこだけ中身で分ける。kind = "base" の
        # 参照実装は base.cpp と一緒に組むので、base.cpp も混ぜる (法の定数を
        # 直したのに古い期待出力が使われた)。
        parts = [
            str(td.count),
            _sha256_file(problem.dir / td.generator),
            _sha256_file(problem.dir / td.reference),
        ]
        base_cpp = problem.dir / "base.cpp"
        if problem.harness_kind == "base" and base_cpp.is_file():
            parts.append(_sha256_file(base_cpp))
        digest = hashlib.sha256("\n".join(parts).encode()).hexdigest()[:16]
        return TESTCASE_CACHE_DIR / "local" / _safe_parts(problem.id) / digest
    # source = "none" は判定サイトの名前を持たない。中身も置かないが、
    # ensure が場所を引くので落とさない。
    return TESTCASE_CACHE_DIR / _safe_parts(td.source) / _safe_parts(
        td.name or problem.id
    )


def _safe_parts(name: str) -> Path:
    """名前をそのままパスにする。判定サイト側の文字列なので素通しにしない。

    library_checker の name は `data_structure/point_add_range_sum` のように
    区切りを含むので、階層はそのまま残す。
    """
    parts = []
    for part in name.split("/"):
        cleaned = "".join(c if c.isalnum() or c in "._-" else "_" for c in part)
        cleaned = cleaned.lstrip(".")
        if cleaned:
            parts.append(cleaned)
    if not parts:
        raise FetchError(f"テストデータの名前からパスを作れません: {name!r}")
    return Path(*parts)


def _sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def _matches_manifest(cases: tuple[Case, ...], recorded: list[dict]) -> bool:
    """手元のケースが manifest の書いたものと食い違っていないか。

    食い違ったまま再利用すると、manifest の cases_hash をキーに使いながら
    別の中身で走らせることになる。記録の意味が静かにずれる。

    名前とサイズまでで見る。sha256 まで取ると毎回 100 MB 超を読み直すことに
    なって、すべてスキップされる実行が重くなる。
    """
    if len(cases) != len(recorded):
        return False
    sizes = {entry["name"]: entry for entry in recorded}
    for case in cases:
        entry = sizes.get(case.name)
        if entry is None:
            return False
        # 古い manifest はサイズを持たないことがある。そこは数だけで通す。
        for path, key in ((case.in_path, "in_bytes"), (case.out_path, "out_bytes")):
            want = entry.get(key)
            if want is not None and path.stat().st_size != want:
                return False
    return True


def collect_cases(directory: Path) -> tuple[Case, ...]:
    cases = []
    for in_path in sorted(directory.glob("*.in")):
        if is_hidden(in_path.name):
            continue
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


def write_manifest(
    directory: Path,
    cases: tuple[Case, ...],
    source: str,
    name: str,
    extra: Mapping[str, object] | None = None,
) -> str:
    """manifest を書いて cases_hash を返す。

    extra は取得元が足す項目。library_checker は生成に使った上流のコミットを
    入れて、pin が動いたときに古いものだと分かるようにする。
    """
    cases_hash = compute_cases_hash(cases)
    manifest = {
        "source": source,
        "name": name,
        "fetched_at": datetime.now(UTC).isoformat(timespec="seconds"),
        "cases_hash": cases_hash,
        **dict(extra or {}),
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
    env=None,
) -> Testcases:
    """テストデータを用意して返す。

    探索の順は、手元のキャッシュ、保管庫、原本。原本まで行ったら、成功した
    あとに保管庫へ上げる。CI のランナーは手元のキャッシュを持たずに始まるので、
    走らせる問題のぶんだけ保管庫から落とす。

    library_checker は pin で作ったものだけを使う。手元や保管庫にあるものが
    古い pin で作られていれば作り直し、保管庫のものは置き換える。

    env は local の参照実装を組むコンパイラ。渡さなければ環境 local のもの。
    """
    source = problem.testdata.source
    if source == "none":
        return Testcases(dir=cache_dir_for(problem), cases=(), cases_hash="")

    dest = cache_dir_for(problem)
    manifest_path = dest / MANIFEST_NAME
    if not refresh and manifest_path.is_file():
        manifest = _read_manifest(dest)
        cases = collect_cases(dest)
        if cases and _matches_manifest(cases, manifest.get("cases", [])):
            if not _is_current(problem, manifest):
                print(f"  手元の {problem.id} は古い pin で作ったものです。作り直します",
                      file=sys.stderr)
            elif _lacks_checker(problem, dest):
                print(f"  手元の {problem.id} に checker.cpp が無いので作り直します", file=sys.stderr)
            else:
                return Testcases(dir=dest, cases=cases, cases_hash=manifest["cases_hash"])

    # 保管庫から取れるなら原本を叩かない。レート制限と障害を経路から外す。
    replace = False
    if allow_mirror and mirror.should_mirror(problem) and mirror.available():
        try:
            if mirror.pull(problem, dest):
                pulled = _read_manifest(dest)
                if not _is_current(problem, pulled):
                    # pin を動かしたのは意図的な操作なので、CI からでも置き換える。
                    print(
                        f"  保管庫の {problem.id} は古い pin で作ったものです。"
                        "作り直して置き換えます",
                        file=sys.stderr,
                    )
                    replace = True
                elif _lacks_checker(problem, dest):
                    # チェッカを入れずに上げたアセットが残っている。作り直して置き換える。
                    print(
                        f"  保管庫の {problem.id} に checker.cpp が入っていません。"
                        "作り直して置き換えます",
                        file=sys.stderr,
                    )
                    replace = True
                else:
                    return _finish(problem, dest, source, extra=_carried(pulled))
        except mirror.MirrorError as e:
            print(f"  保管庫から取れませんでした: {e}", file=sys.stderr)

    extra = _fetch_from_origin(problem, dest, env=env)
    result = _finish(problem, dest, source, extra=extra)

    # 一度保管すれば、次からは原本を叩かない。
    if push_mirror and mirror.should_mirror(problem) and mirror.available():
        try:
            mirror.push(problem, dest, force=replace)
        except mirror.MirrorError as e:
            print(f"  保管庫へ上げられませんでした: {e}", file=sys.stderr)

    return result


def evict(problem: Problem) -> None:
    """手元のキャッシュからその問題のテストデータを消す。

    CI のランナーは disk が 14 GB ほどしか無い。1 ジョブで数十問を測るので、
    測り終えた問題のぶんは捨てていく。保管庫にあるので、次に要るときは落とせる。
    """
    directory = cache_dir_for(problem)
    if directory.exists():
        shutil.rmtree(directory)


def _lacks_checker(problem: Problem, directory: Path) -> bool:
    """compare.kind = "checker" なのにチェッカのソースが無いか。

    Library Checker が同梱するものを使うので、テストデータと一緒に置いてある
    必要がある。無いまま走らせると判定器が組めず、その問題は測れない。
    """
    return problem.compare.kind == "checker" and not (directory / "checker.cpp").is_file()


def _read_manifest(directory: Path) -> dict:
    try:
        data = json.loads((directory / MANIFEST_NAME).read_text())
    except (OSError, json.JSONDecodeError):
        return {}
    return data if isinstance(data, dict) else {}


def _is_current(problem: Problem, manifest: dict) -> bool:
    """手元や保管庫にあるものが、いま取るべきものと同じか。

    library_checker だけは pin があるので、manifest に書いた上流のコミットと
    突き合わせる。ほかの取得元は判定サイトの今の中身が正で、こちらから新旧を
    言えない。
    """
    if problem.testdata.source != "library_checker":
        return True
    from . import library_checker

    return library_checker.is_current(manifest)


def _carried(manifest: dict) -> dict:
    """保管庫から取ったものの manifest を書き直すとき、持ち越す項目。"""
    from . import library_checker

    key = library_checker.UPSTREAM_KEY
    return {key: manifest[key]} if key in manifest else {}


def _fetch_from_origin(problem: Problem, dest: Path, env=None) -> dict | None:
    """原本から取る。manifest に足す項目があれば返す。"""
    source = problem.testdata.source
    if source == "local":
        from . import local

        local.fetch(problem, dest, env)
        return None
    if source == "library_checker":
        from . import library_checker

        return library_checker.fetch(problem, dest)
    if source == "aoj":
        from . import aoj

        aoj.fetch(problem, dest)
    elif source == "yukicoder":
        from . import yukicoder

        yukicoder.fetch(problem, dest)
    elif source == "loj":
        from . import loj

        loj.fetch(problem, dest)
    elif source == "manual":
        from . import manual

        manual.fetch(problem, dest)
    else:
        raise FetchError(f"testdata.source {source!r} の取得はまだ実装していません")
    return None


def _finish(
    problem: Problem, dest: Path, source: str, extra: Mapping[str, object] | None = None
) -> Testcases:
    """取れたものを数えて manifest を書き直す。

    cases_hash はケースの内容から計算する。保管庫と原本のどちらから取っても
    同じ値になる必要がある。
    """
    cases = collect_cases(dest)
    if not cases:
        raise FetchError(f"{dest} にケースがありません")
    cases_hash = write_manifest(dest, cases, source, problem.testdata.name, extra)
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
