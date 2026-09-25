"""problem.toml の読み込みと検証。"""

from __future__ import annotations

import tomllib
from dataclasses import dataclass, field
from pathlib import Path

from .paths import PROBLEMS_DIR

HARNESS_KINDS = frozenset({"base", "raw"})

# 実装している取得元と比較。残りは DESIGN.md の「最小の第一版」を参照。
TESTDATA_SOURCES = frozenset(
    {"library_checker", "aoj", "yukicoder", "loj", "manual", "local", "none"}
)
PLANNED_TESTDATA_SOURCES: frozenset[str] = frozenset()
COMPARE_KINDS = frozenset({"tokens", "float", "checker", "compile_only", "exit_code"})
PLANNED_COMPARE_KINDS: frozenset[str] = frozenset()

# id の接頭辞は問題そのものの出どころを表す (DESIGN.md「problem.toml」)。判定サイトから
# 取るテストデータはその出どころの問題にしか付かないので、食い違っていたらどちらかが
# 間違っている。逆向きは成り立たない (aoj- の問題を local や manual で持ってよい)。
SOURCE_PREFIXES = {
    "library_checker": "yosupo-",
    "aoj": "aoj-",
    "yukicoder": "yuki-",
    "loj": "loj-",
}

# 判定サイトの問題の id の接頭辞 (DESIGN.md「problem.toml」の表)。importer が新しい問題を
# 書き出すとき、この接頭辞を持つ id は problems/<接頭辞>/<残り>/ に置く。
JUDGE_PREFIXES = (
    "yosupo", "aoj", "yuki", "atcoder", "loj", "hackerrank", "cses", "joisc", "cf",
    "ojuz", "codechef", "kattis", "luogu",
)

# 自作の問題の id の接頭辞。自作の問題は problems/self/ の下に族ごとに置く。
SELF_PREFIX = "self"


class ProblemError(Exception):
    """problem.toml が壊れているときに投げる。"""


@dataclass(frozen=True)
class Limits:
    tle_sec: float = 5.0
    mle_mb: int = 256


@dataclass(frozen=True)
class Testdata:
    source: str
    name: str = ""
    generator: str = "gen.py"
    count: int = 0
    reference: str = ""


@dataclass(frozen=True)
class Compare:
    kind: str = "tokens"
    # kind = "float" の許容誤差。絶対誤差か相対誤差のどちらかに収まれば同じとみなす。
    abs_tol: float = 0.0
    rel_tol: float = 0.0


@dataclass(frozen=True)
class Problem:
    id: str
    title: str
    dir: Path
    harness_kind: str
    limits: Limits
    testdata: Testdata
    compare: Compare
    # 元の問題のページ。判定サイトから取る問題は source と name から組めるので書かない。
    # none (AtCoder や自己検証) と manual の問題だけが持つ。表示にしか使わない。
    url: str = ""
    raw: dict = field(repr=False, default_factory=dict)

    @property
    def base_cpp(self) -> Path:
        return self.dir / "base.cpp"

    @property
    def submissions_dir(self) -> Path:
        return self.dir / "submissions"

    def submissions(self) -> list[Path]:
        """提出のパスを問題ディレクトリからの相対で返す。先頭が _ のものは除く。"""
        if not self.submissions_dir.is_dir():
            return []
        suffix = ".hpp" if self.harness_kind == "base" else ".cpp"
        found = [
            p
            for p in sorted(self.submissions_dir.iterdir())
            if p.is_file() and p.suffix == suffix and not p.name.startswith("_")
        ]
        return [p.relative_to(self.dir) for p in found]


def _require(cond: bool, message: str) -> None:
    if not cond:
        raise ProblemError(message)


def load(problem_dir: Path) -> Problem:
    """problem.toml を読んで検証する。"""
    problem_dir = problem_dir.resolve()
    toml_path = problem_dir / "problem.toml"
    _require(toml_path.is_file(), f"{toml_path} がありません")

    with toml_path.open("rb") as f:
        raw = tomllib.load(f)

    pid = raw.get("id", "")
    _require(bool(pid), f"{toml_path}: id が必要です")
    expected = problem_id_of(problem_dir)
    _require(
        pid == expected,
        f"{toml_path}: id {pid!r} が置き場所から決まる名前 {expected!r} と一致しません",
    )

    limits_raw = raw.get("limits", {})
    limits = Limits(
        tle_sec=float(limits_raw.get("tle_sec", Limits.tle_sec)),
        mle_mb=int(limits_raw.get("mle_mb", Limits.mle_mb)),
    )
    _require(limits.tle_sec > 0, f"{toml_path}: limits.tle_sec は正の数にしてください")
    _require(limits.mle_mb > 0, f"{toml_path}: limits.mle_mb は正の数にしてください")

    harness_kind = raw.get("harness", {}).get("kind", "base")
    _require(
        harness_kind in HARNESS_KINDS,
        f"{toml_path}: harness.kind {harness_kind!r} は {sorted(HARNESS_KINDS)} のいずれかです",
    )

    td_raw = raw.get("testdata", {})
    source = td_raw.get("source", "")
    if source in PLANNED_TESTDATA_SOURCES:
        raise ProblemError(
            f"{toml_path}: testdata.source {source!r} は未実装です (M4 で足します)"
        )
    _require(
        source in TESTDATA_SOURCES,
        f"{toml_path}: testdata.source {source!r} は {sorted(TESTDATA_SOURCES)} のいずれかです",
    )
    testdata = Testdata(
        source=source,
        name=td_raw.get("name", ""),
        generator=td_raw.get("generator", Testdata.generator),
        count=int(td_raw.get("count", 0)),
        reference=td_raw.get("reference", ""),
    )
    if source in ("library_checker", "aoj", "yukicoder", "loj", "manual"):
        _require(bool(testdata.name), f"{toml_path}: {source} には name が必要です")
    if source == "local":
        _require(
            testdata.count > 0, f"{toml_path}: local には正の count が必要です"
        )
        _require(bool(testdata.reference), f"{toml_path}: local には reference が必要です")
        _require(
            (problem_dir / testdata.generator).is_file(),
            f"{toml_path}: ジェネレータ {testdata.generator} がありません",
        )
        _require(
            (problem_dir / testdata.reference).is_file(),
            f"{toml_path}: 参照実装 {testdata.reference} がありません",
        )

    cmp_raw = raw.get("compare", {})
    kind = cmp_raw.get("kind", Compare.kind)
    if kind in PLANNED_COMPARE_KINDS:
        raise ProblemError(f"{toml_path}: compare.kind {kind!r} は未実装です")
    _require(
        kind in COMPARE_KINDS,
        f"{toml_path}: compare.kind {kind!r} は {sorted(COMPARE_KINDS)} のいずれかです",
    )
    compare = Compare(
        kind=kind,
        abs_tol=float(cmp_raw.get("abs_tol", 0.0)),
        rel_tol=float(cmp_raw.get("rel_tol", 0.0)),
    )
    if kind == "float":
        _require(
            compare.abs_tol > 0 or compare.rel_tol > 0,
            f"{toml_path}: compare.kind = 'float' には正の abs_tol か rel_tol が必要です",
        )
    else:
        _require(
            "abs_tol" not in cmp_raw and "rel_tol" not in cmp_raw,
            f"{toml_path}: abs_tol と rel_tol は compare.kind = 'float' でだけ使います",
        )

    if source == "none":
        _require(
            kind in ("compile_only", "exit_code"),
            f"{toml_path}: testdata.source = 'none' で使える compare.kind は 'compile_only' か 'exit_code' です",
        )
    if kind == "exit_code":
        _require(
            source == "none",
            f"{toml_path}: compare.kind = 'exit_code' は testdata.source = 'none' の問題で使います",
        )

    if harness_kind == "base":
        _require(
            (problem_dir / "base.cpp").is_file(),
            f"{toml_path}: harness.kind = 'base' には base.cpp が必要です",
        )

    return Problem(
        id=pid,
        title=raw.get("title", pid),
        dir=problem_dir,
        harness_kind=harness_kind,
        limits=limits,
        testdata=testdata,
        compare=compare,
        url=str(raw.get("url", "")),
        raw=raw,
    )


def warnings(problem: Problem) -> list[str]:
    """エラーにはしないが、直したほうがよいものを返す。"""
    source = problem.testdata.source
    prefix = SOURCE_PREFIXES.get(source)
    if prefix is None or problem.id.startswith(prefix):
        return []
    return [f"id が {prefix!r} で始まっていません (testdata.source = {source!r})"]


def problem_id_of(problem_dir: Path) -> str:
    """ディレクトリの置き場所から決まる id。

    problems/ の下は何段でも掘れて、problems/ からの各段を `-` で繋いだものが id になる。
    problems/atcoder/abc172-d/ なら atcoder-abc172-d、problems/self/gf2-64/pow/ なら
    self-gf2-64-pow。平らに置いた 1 段のものは今までどおりディレクトリ名がそのまま id。
    problems という名前の祖先が無い (テストの一時ディレクトリなど) ときもディレクトリ名。
    """
    resolved = problem_dir.resolve()
    parts: list[str] = []
    for ancestor in [resolved, *resolved.parents]:
        if ancestor.name == PROBLEMS_DIR.name and ancestor != resolved:
            return "-".join(reversed(parts))
        parts.append(ancestor.name)
    return problem_dir.name


def dir_for_new(problem_id: str, root: Path = PROBLEMS_DIR) -> Path:
    """新しい問題を書き出す場所。判定サイトの接頭辞を持つ id は接頭辞のディレクトリの下。

    自作 (self-) の問題も problems/self/ の下に置く。族のディレクトリは人が決めるので、
    self/ の中では平らに置く。
    """
    prefix, sep, rest = problem_id.partition("-")
    if sep and rest and (prefix in JUDGE_PREFIXES or prefix == SELF_PREFIX):
        return root / prefix / rest
    return root / problem_id


def load_by_id(problem_id: str) -> Problem:
    flat = PROBLEMS_DIR / problem_id
    if (flat / "problem.toml").is_file():
        return load(flat)
    for problem_dir in all_problem_dirs():
        if problem_id_of(problem_dir) == problem_id:
            return load(problem_dir)
    raise ProblemError(f"問題 {problem_id!r} がありません ({PROBLEMS_DIR} の下に見つかりません)")


def all_problem_dirs() -> list[Path]:
    """problem.toml を持つディレクトリを problems/ の下から何段でも探す。

    problem.toml を持つディレクトリが問題で、その下は探さない。持たないディレクトリは
    ただの入れ物。`_` か `.` で始まる名前 (_shared、__pycache__) は飛ばす。同じ id が
    2 か所から出たら、どちらを測ればよいか決まらないので止める。
    """
    if not PROBLEMS_DIR.is_dir():
        return []
    found: list[Path] = []

    def walk(directory: Path) -> None:
        for child in sorted(directory.iterdir()):
            if not child.is_dir() or child.name.startswith(("_", ".")):
                continue
            if (child / "problem.toml").is_file():
                found.append(child)
            else:
                walk(child)

    walk(PROBLEMS_DIR)
    seen: dict[str, Path] = {}
    for problem_dir in found:
        pid = problem_id_of(problem_dir)
        if pid in seen:
            raise ProblemError(f"問題 {pid!r} が 2 か所にあります: {seen[pid]} と {problem_dir}")
        seen[pid] = problem_dir
    return sorted(found, key=problem_id_of)


def resolve_submission(problem: Problem, given: str) -> Path:
    """--submission に渡された文字列を問題ディレクトリからの相対パスにする。"""
    candidates = [Path(given), Path("submissions") / given]
    for cand in candidates:
        if (problem.dir / cand).is_file():
            return cand
    # リポジトリルートからのパスで渡された場合も受ける。
    abs_given = Path(given).resolve()
    if abs_given.is_file():
        try:
            return abs_given.relative_to(problem.dir)
        except ValueError:
            pass
    raise ProblemError(f"提出 {given!r} が問題 {problem.id} の中に見つかりません")
