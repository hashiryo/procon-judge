"""competitive-verifier のテストを kind = "raw" の問題として取り込む。

Library の test/**/*.test.cpp は 1 ファイル 1 main で、先頭のコメント行に問題の
URL と制限が書いてある。実装が 1 本しかない問題はハーネスを書いても比べる相手が
いないので、ファイルをそのまま提出にして問題を機械的に作る。ハーネスに書き直す
のは 2 本目の実装が来たときで、それまでは raw が最終形でよい。

ファイル名は <問題>.<実装>.test.cpp の形で、同じ問題の実装が複数あることがある。
問題の同一性は URL (から決まる id) で見る。
"""

from __future__ import annotations

import re
import sys
from dataclasses import dataclass
from pathlib import Path
from urllib.parse import parse_qs, urlparse

from . import titles as titles_mod
from .fetch import FetchError, library_checker
from . import problem as problem_mod
from .paths import PROBLEMS_DIR

ANNOTATION = re.compile(r"^\s*//\s*competitive-verifier:\s*(\S+)(?:\s+(.*?))?\s*$")

# 判定サイトの制限が取れないときの仮の値。手で移した問題と揃えてある。
DEFAULT_TLE_SEC = 5.0
DEFAULT_MLE_MB = 512
# Library Checker はどの問題もメモリ 1024 MB。
LIBRARY_CHECKER_MLE_MB = 1024

# 制限は判定サイトの値を採り、Library の注釈 (TLE 0.5 / MLE 64 のように締めた値) は
# 採らない。ここは劣化を pass/fail で見る場所ではなく、1 つのキーは 1 回しか測られない
# ので、締めた値で落とすとたまたま遅かった回の TLE がキーが変わるまで残る。
# ただし注釈の方が大きいときは注釈に合わせる。Library が通ると知っている値だから。


def _limits(official_tle: float, official_mle: int, notes: Annotations) -> tuple[float, int]:
    return (
        max(official_tle, notes.tle or 0.0),
        max(official_mle, notes.mle or 0),
    )


class MigrateError(Exception):
    """取り込めないときに投げる。"""


@dataclass(frozen=True)
class Annotations:
    problem: str | None = None
    tle: float | None = None
    mle: int | None = None
    error: float | None = None
    standalone: bool = False
    ignore: bool = False


def parse_annotations(text: str) -> Annotations:
    found: dict[str, object] = {}
    for line in text.splitlines():
        m = ANNOTATION.match(line)
        if not m:
            continue
        kind, value = m.group(1).upper(), (m.group(2) or "").strip()
        if kind == "PROBLEM":
            found["problem"] = value
        elif kind == "TLE":
            found["tle"] = float(value)
        elif kind == "MLE":
            found["mle"] = int(float(value))
        elif kind == "ERROR":
            found["error"] = float(value)
        elif kind == "STANDALONE":
            found["standalone"] = True
        elif kind == "IGNORE":
            found["ignore"] = True
    return Annotations(**found)  # type: ignore[arg-type]


def strip_annotations(text: str) -> str:
    """competitive-verifier の行だけを落とす。ほかは 1 文字も変えない。"""
    lines = [line for line in text.splitlines(keepends=True) if not ANNOTATION.match(line)]
    while lines and not lines[0].strip():
        lines.pop(0)
    return "".join(lines)


@dataclass(frozen=True)
class Origin:
    """URL から決まる、問題の出どころ。"""

    source: str
    name: str
    id: str
    url: str


def origin_of(url: str) -> Origin:
    """URL を testdata.source と name と問題 id に直す。

    id の付け方は DESIGN.md の表のとおり。yosupo と atcoder は URL の名前の
    `_` を `-` にする (yosupo-sharp-p-subset-sum、atcoder-typical90-bp)。
    AOJ と yukicoder は判定サイトの id をそのまま使う (aoj-DSL_2_B、yuki-1234)。
    """
    parsed = urlparse(url)
    host = parsed.netloc.lower()
    parts = [p for p in parsed.path.split("/") if p]
    if host == "judge.yosupo.jp" and len(parts) >= 2 and parts[0] == "problem":
        name = parts[1]
        return Origin("library_checker", name, "yosupo-" + name.replace("_", "-"), url)
    if host.endswith("u-aizu.ac.jp") and parts:
        query = parse_qs(parsed.query)
        pid = query["id"][0] if "id" in query else parts[-1]
        return Origin("aoj", pid, f"aoj-{pid}", url)
    if host == "yukicoder.me" and len(parts) >= 3 and parts[:2] == ["problems", "no"]:
        return Origin("yukicoder", parts[2], f"yuki-{parts[2]}", url)
    if host == "atcoder.jp" and "tasks" in parts and parts.index("tasks") + 1 < len(parts):
        task = parts[parts.index("tasks") + 1]
        # ABC の Ex 問題の task id は _h。_Ex と書いた URL は 404 になるので直す。
        if task.lower().endswith("_ex"):
            task = task[: -len("_ex")] + "_h"
            url = url[: url.rfind("/") + 1] + task
        return Origin("none", "", "atcoder-" + task.replace("_", "-"), url)
    if host == "loj.ac" and parts[:1] == ["p"] and len(parts) >= 2:
        return Origin("loj", parts[1], f"loj-{parts[1]}", url)
    # 以下はテストデータを機械的に取れない判定サイト。source = "manual" で、name は id。
    manual = _manual_id(host, parts, parsed.fragment)
    if manual is not None:
        return Origin("manual", manual, manual, url)
    raise MigrateError(f"{url}: 出どころが分かりません。手で取り込んでください")


def _manual_id(host: str, parts: list[str], fragment: str) -> str | None:
    """テストデータを手で取り込む判定サイトの id。接頭辞は既にある問題に合わせる。"""
    if host == "codeforces.com" and "problem" in parts:
        i = parts.index("problem")
        if parts[0] == "contest" and i == 2:
            return f"cf-{parts[1]}-{parts[3].lower()}"
        if parts[0] == "gym" and i == 2:
            return f"cf-gym{parts[1]}-{parts[3].lower()}"
        if parts[:2] == ["problemset", "problem"] and len(parts) >= 4:
            return f"cf-{parts[2]}-{parts[3].lower()}"
    if host == "oj.uz" and parts[:2] == ["problem", "view"] and len(parts) >= 3:
        return f"ojuz-{parts[2]}"
    if host.endswith("codechef.com") and parts[:1] == ["problems"] and len(parts) >= 2:
        return f"codechef-{parts[1]}"
    if host.endswith("kattis.com") and parts[:1] == ["problems"] and len(parts) >= 2:
        return f"kattis-{parts[1]}"
    if host.endswith("luogu.com.cn") and parts[:1] == ["problem"] and len(parts) >= 2:
        return f"luogu-{parts[1]}"
    if host.endswith("hackerrank.com") and "challenges" in parts:
        i = parts.index("challenges")
        if i + 1 < len(parts):
            return f"hackerrank-{parts[i + 1]}"
    if host == "cses.fi" and parts[:2] == ["problemset", "task"] and len(parts) >= 3:
        return f"cses-{parts[2]}"
    if host.endswith("ioi-jp.org") and parts[:1] == ["camp"] and len(parts) >= 3:
        # https://www2.ioi-jp.org/camp/2019/2019-sp-tasks/day1/examination.pdf -> joisc-2019-examination
        year = parts[1]
        stem = parts[-1].removesuffix(".pdf").removeprefix(f"{year}-")
        pieces = [year, stem] + ([fragment] if fragment else [])
        return "joisc-" + "-".join(pieces).replace("_", "-")
    return None


@dataclass(frozen=True)
class Item:
    """1 問ぶんの取り込みの計画。"""

    id: str
    title: str
    source: str
    name: str
    compare: str
    tle_sec: float
    mle_mb: int
    # (提出の名前, 元のファイル)。名前は submissions/ の下のファイル名。
    submissions: tuple[tuple[str, Path], ...]
    url: str = ""
    skip: str = ""
    # compare = "float" の許容誤差。ERROR の注釈から、絶対と相対の両方に同じ値を置く。
    tolerance: float = 0.0

    @property
    def skipped(self) -> bool:
        return bool(self.skip)


def collect_files(paths: list[Path]) -> list[Path]:
    found: list[Path] = []
    for path in paths:
        if path.is_dir():
            found.extend(sorted(path.rglob("*.test.cpp")))
        elif path.is_file():
            found.append(path)
        else:
            raise MigrateError(f"{path} がありません")
    return sorted(set(found))


def _impl_of(path: Path) -> str:
    """`assignment.mcf.test.cpp` の `mcf`。無ければ空。"""
    stem = path.name[: -len(".test.cpp")] if path.name.endswith(".test.cpp") else path.stem
    parts = stem.split(".")
    return ".".join(parts[1:]) if len(parts) > 1 else ""


def _key_of(path: Path) -> str:
    stem = path.name[: -len(".test.cpp")] if path.name.endswith(".test.cpp") else path.stem
    return stem.split(".")[0]


def _stem_of(path: Path) -> str:
    return path.name[: -len(".test.cpp")] if path.name.endswith(".test.cpp") else path.stem


URL_IN_TEXT = re.compile(r"https?://[^\s)>\"']+")


def _standalone_origin(text: str) -> Origin | None:
    """STANDALONE のファイルが普通のコメントに書いている元の問題の URL。

    自分で入出力を持つテストでも、何の問題を解いたかは書いてある。id はそこから
    取る (atcoder-typical90-bp、yuki-1421 のように)。無ければ None。
    """
    for m in URL_IN_TEXT.finditer(text):
        try:
            return origin_of(m.group(0).rstrip(".,;"))
        except MigrateError:
            continue
    return None


def _submission_name(path: Path, single: bool) -> str:
    """慣習の lib を名前にする。実装が複数ならファイル名の実装の部分を添える。"""
    impl = _impl_of(path)
    if single or not impl:
        return "lib.cpp"
    cleaned = re.sub(r"[^A-Za-z0-9]+", "-", impl).strip("-").lower()
    return f"lib-{cleaned}.cpp"


def plan(
    files: list[Path],
    *,
    existing: set[str],
    single_only: bool = False,
    titles: titles_mod.Titles | None = None,
    library_checker_dir: Path | None = None,
) -> list[Item]:
    """取り込みの計画。書き込みはしない。"""
    groups: dict[str, list[tuple[Path, Annotations, Origin | None]]] = {}
    skipped: list[Item] = []
    for path in files:
        text = path.read_text(errors="replace")
        notes = parse_annotations(text)
        if notes.standalone or not notes.problem:
            if notes.ignore:
                skipped.append(_skipped(path, "IGNORE が付いています"))
                continue
            # 自己検証。元の問題の URL がコメントにあれば id はそこから、無ければ
            # 自作の問題として self- にファイル名の全部を続ける (mat.unit_test ->
            # self-mat-unit-test)。
            origin = _standalone_origin(text)
            key = (
                origin.id
                if origin
                else f"{problem_mod.SELF_PREFIX}-" + re.sub(r"[._]", "-", _stem_of(path))
            )
            groups.setdefault(key, []).append((path, notes, _Standalone(origin)))
            continue
        try:
            origin = origin_of(notes.problem)
        except MigrateError as e:
            skipped.append(_skipped(path, str(e)))
            continue
        # AtCoder の IGNORE はケースが取れないという意味で、こちらでは compile_only に
        # なるので通す。ほかの取得元の IGNORE は verify を止めているので、そのまま飛ばす。
        if notes.ignore and origin.source != "none":
            skipped.append(_skipped(path, "IGNORE が付いています"))
            continue
        groups.setdefault(origin.id, []).append((path, notes, origin))

    items: list[Item] = list(skipped)
    for problem_id, members in sorted(groups.items()):
        if problem_id in existing:
            items.append(_skipped_many(problem_id, members, "既にあります"))
            continue
        if single_only and len(members) > 1:
            items.append(
                _skipped_many(problem_id, members, f"実装が {len(members)} 本あります")
            )
            continue
        try:
            items.append(
                _item(problem_id, members, titles=titles, library_checker_dir=library_checker_dir)
            )
        except (MigrateError, FetchError, titles_mod.TitleError) as e:
            items.append(_skipped_many(problem_id, members, str(e)))
    return items


def _skipped(path: Path, reason: str) -> Item:
    return Item(
        id=_key_of(path), title="", source="", name="", compare="", tle_sec=0, mle_mb=0,
        submissions=(("", path),), skip=reason,
    )


def _skipped_many(problem_id: str, members, reason: str) -> Item:
    return Item(
        id=problem_id, title="", source="", name="", compare="", tle_sec=0, mle_mb=0,
        submissions=tuple(("", m[0]) for m in members), skip=reason,
    )


def _item(problem_id, members, *, titles, library_checker_dir) -> Item:
    single = len(members) == 1
    submissions = tuple(
        (_submission_name(path, single), path) for path, _, _ in members
    )
    names = [name for name, _ in submissions]
    if len(set(names)) != len(names):
        raise MigrateError(f"提出の名前が重なります: {names}")

    origin = members[0][2]
    notes = Annotations(
        tle=max((m[1].tle or 0.0 for m in members), default=0.0) or None,
        mle=max((m[1].mle or 0 for m in members), default=0) or None,
    )
    # ERROR が付いていれば誤差つきの比較。複数あれば緩い方に合わせる。
    tolerance = max((m[1].error or 0.0 for m in members), default=0.0)
    compare = "float" if tolerance > 0 else "tokens"
    if isinstance(origin, _Standalone):
        # STANDALONE。入出力を持たず、自分で確かめて終了コードで答える。
        tle, mle = _limits(DEFAULT_TLE_SEC, DEFAULT_MLE_MB, notes)
        url = origin.origin.url if origin.origin else ""
        return Item(
            id=problem_id, title=_title_for(problem_id, url, titles), source="none", name="",
            compare="exit_code", tle_sec=tle, mle_mb=mle,
            submissions=submissions, url=url,
        )
    assert origin is not None
    if origin.source == "library_checker":
        repo = library_checker_dir or library_checker.ensure_repo()
        directory = library_checker.problem_dir(origin.name, repo)
        if directory is None:
            raise MigrateError(f"library-checker-problems に {origin.name} がありません")
        info = library_checker.read_info(directory)
        tle, mle = _limits(
            float(info.get("timelimit", DEFAULT_TLE_SEC)), LIBRARY_CHECKER_MLE_MB, notes
        )
        return Item(
            id=problem_id,
            title=titles_mod.plain(str(info.get("title", origin.name))),
            source="library_checker",
            name=directory.relative_to(repo).as_posix(),
            compare="checker" if (directory / "checker.cpp").is_file() else "tokens",
            tle_sec=tle,
            mle_mb=mle,
            submissions=submissions,
            url=origin.url,
        )
    if origin.source == "none":
        # AtCoder。ケースが公開されないので、コンパイルが通るかだけを見る。
        tle, mle = _limits(DEFAULT_TLE_SEC, DEFAULT_MLE_MB, notes)
        return Item(
            id=problem_id, title=_title_for(problem_id, origin.url, titles), source="none",
            name="", compare="compile_only", tle_sec=tle, mle_mb=mle,
            submissions=submissions, url=origin.url,
        )
    if origin.source == "manual":
        # テストデータは人が取り込む。保管庫に無い間は CI が警告を出して飛ばす。
        tle, mle = _limits(DEFAULT_TLE_SEC, DEFAULT_MLE_MB, notes)
        return Item(
            id=problem_id, title=problem_id, source="manual", name=origin.name,
            compare=compare, tle_sec=tle, mle_mb=mle,
            submissions=submissions, url=origin.url, tolerance=tolerance,
        )
    title = problem_id
    official_tle, official_mle = DEFAULT_TLE_SEC, DEFAULT_MLE_MB
    if titles is not None:
        probe = _Probe(origin.source, origin.name)
        official = titles.official(probe)  # type: ignore[arg-type]
        if official:
            title = official
        if origin.source == "aoj":
            # AOJ の一覧は制限も持っている。判定サイトの値より下には締めない。
            limits = titles.aoj_limits(origin.name)
            if limits is not None:
                official_tle = max(official_tle, limits[0])
                official_mle = max(official_mle, limits[1])
        if origin.source == "loj":
            # LOJ は問題の API が制限を持っている。AOJ と同じ扱い。
            limits = titles.loj_limits(origin.name)
            if limits is not None:
                official_tle = max(official_tle, limits[0])
                official_mle = max(official_mle, limits[1])
    tle, mle = _limits(official_tle, official_mle, notes)
    return Item(
        id=problem_id, title=title, source=origin.source, name=origin.name,
        compare=compare, tle_sec=tle, mle_mb=mle,
        submissions=submissions, url=origin.url, tolerance=tolerance,
    )


@dataclass(frozen=True)
class _Standalone:
    """STANDALONE の印。元の問題が分かればその出どころを持つ (id と url に使う)。"""

    origin: Origin | None


def _title_for(problem_id: str, url: str, titles: titles_mod.Titles | None) -> str:
    """source = none の問題の題名。AtCoder ならページから取れる。取れなければ id。"""
    if titles is None or not url:
        return problem_id
    try:
        official = titles.official(_Probe("none", "", url))  # type: ignore[arg-type]
    except titles_mod.TitleError as e:
        print(f"  warning: {problem_id} の題名を取れません: {e}", file=sys.stderr)
        return problem_id
    return official or problem_id


class _Probe:
    """Titles.official が見るのは testdata.source と name と url だけ。"""

    def __init__(self, source: str, name: str, url: str = "") -> None:
        self.testdata = _Testdata(source, name)
        self.url = url


class _Testdata:
    def __init__(self, source: str, name: str) -> None:
        self.source, self.name = source, name


def problem_toml(item: Item) -> str:
    lines = [
        f"id = {titles_mod.toml_string(item.id)}",
        f"title = {titles_mod.toml_string(item.title)}",
    ]
    if item.url and item.source in ("none", "manual"):
        # 判定サイトから取る問題は source と name から URL を組めるので書かない。
        lines.append(f"url = {titles_mod.toml_string(item.url)}")
    lines += [
        "",
        "[limits]",
        f"tle_sec = {item.tle_sec}",
        f"mle_mb = {item.mle_mb}",
        "",
        "[harness]",
        'kind = "raw"',
        "",
        "[testdata]",
        f"source = {titles_mod.toml_string(item.source)}",
    ]
    if item.name:
        lines.append(f"name = {titles_mod.toml_string(item.name)}")
    lines += ["", "[compare]", f"kind = {titles_mod.toml_string(item.compare)}"]
    if item.compare == "float":
        lines += [f"abs_tol = {item.tolerance!r}", f"rel_tol = {item.tolerance!r}"]
    lines.append("")
    return "\n".join(lines)


def write(item: Item, problems_dir: Path = PROBLEMS_DIR) -> Path:
    """問題を書き出す。既にあれば触らない。

    判定サイトの接頭辞を持つ id は problems/<接頭辞>/<残り>/ に置く (problem.dir_for_new)。
    """
    directory = problem_mod.dir_for_new(item.id, problems_dir)
    if directory.exists():
        raise MigrateError(f"{directory} は既にあります")
    (directory / "submissions").mkdir(parents=True)
    (directory / "problem.toml").write_text(problem_toml(item))
    for name, source in item.submissions:
        text = strip_annotations(source.read_text(errors="replace"))
        (directory / "submissions" / name).write_text(text)
    return directory


def report(items: list[Item], *, dry_run: bool, file=sys.stderr) -> None:
    for item in items:
        if item.skipped:
            files = ", ".join(str(p) for _, p in item.submissions)
            print(f"SKIP {item.id}: {item.skip}  ({files})", file=file)
        else:
            verb = "PLAN" if dry_run else "NEW "
            tol = f" (誤差 {item.tolerance:g})" if item.compare == "float" else ""
            print(
                f"{verb} {item.id}  {item.source}/{item.compare}{tol}  "
                f"tle {item.tle_sec} s / mle {item.mle_mb} MB  "
                + ", ".join(f"{name} <- {path}" for name, path in item.submissions),
                file=file,
            )
    made = sum(1 for i in items if not i.skipped)
    skipped = len(items) - made
    print(f"{'計画' if dry_run else '取り込み'} {made} 問 / 飛ばした {skipped} 件", file=file)
