"""problem.toml の読み込みと検証。"""

from __future__ import annotations

import tomllib
from dataclasses import dataclass, field
from pathlib import Path

from .paths import PROBLEMS_DIR

HARNESS_KINDS = frozenset({"base", "raw"})

# 実装している取得元と比較。残りは DESIGN.md の「最小の第一版」を参照。
TESTDATA_SOURCES = frozenset(
    {"library_checker", "aoj", "yukicoder", "manual", "local", "none"}
)
PLANNED_TESTDATA_SOURCES: frozenset[str] = frozenset()
COMPARE_KINDS = frozenset({"tokens", "checker", "compile_only", "exit_code"})
PLANNED_COMPARE_KINDS = frozenset({"float"})

# id の接頭辞は問題そのものの出どころを表す (DESIGN.md「problem.toml」)。判定サイトから
# 取るテストデータはその出どころの問題にしか付かないので、食い違っていたらどちらかが
# 間違っている。逆向きは成り立たない (aoj- の問題を local や manual で持ってよい)。
SOURCE_PREFIXES = {
    "library_checker": "yosupo-",
    "aoj": "aoj-",
    "yukicoder": "yuki-",
}


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


@dataclass(frozen=True)
class Problem:
    id: str
    title: str
    dir: Path
    harness_kind: str
    limits: Limits
    testdata: Testdata
    compare: Compare
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
    _require(
        pid == problem_dir.name,
        f"{toml_path}: id {pid!r} がディレクトリ名 {problem_dir.name!r} と一致しません",
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
    if source in ("library_checker", "aoj", "yukicoder", "manual"):
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
    compare = Compare(kind=kind)

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
        raw=raw,
    )


def warnings(problem: Problem) -> list[str]:
    """エラーにはしないが、直したほうがよいものを返す。"""
    source = problem.testdata.source
    prefix = SOURCE_PREFIXES.get(source)
    if prefix is None or problem.id.startswith(prefix):
        return []
    return [f"id が {prefix!r} で始まっていません (testdata.source = {source!r})"]


def load_by_id(problem_id: str) -> Problem:
    problem_dir = PROBLEMS_DIR / problem_id
    if not problem_dir.is_dir():
        raise ProblemError(f"問題 {problem_id!r} がありません ({problem_dir})")
    return load(problem_dir)


def all_problem_dirs() -> list[Path]:
    if not PROBLEMS_DIR.is_dir():
        return []
    return sorted(p for p in PROBLEMS_DIR.iterdir() if (p / "problem.toml").is_file())


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
