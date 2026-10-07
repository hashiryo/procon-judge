"""判定機の CPU が持たない命令に代わる経路を、エミュレーションで確かめる。

判定サイトへ出すときは、bundle が土台の命令 (vpclmulqdq を含む) の pragma を先頭に
入れるので、vpclmulqdq を使うコードも組める。Codeforces の判定機は vpclmulqdq を
持たないので、それを使うコードは __builtin_cpu_supports で実行時に経路を分け、持たない
CPU では代わりの経路を通る。CI の x64 のランナーはどれも vpclmulqdq を持つので、
測定では代わりの経路を一度も通らない。

ここでは、Library の実行時の分岐を通る提出を x64-gcc と同じフラグで組み、Codeforces と
同じ命令の CPU を QEMU で真似て全ケース走らせる。持たない命令を実行すれば SIGILL で
落ち、代わりの経路の答えが違えば WA になる。時間はエミュレーションのものなので残さず、
制限は tle_sec を TLE_SCALE 倍に緩める。メモリは QEMU 自身のものしか取れないので見ない。
"""

from __future__ import annotations

import platform
import re
import shlex
import shutil
import signal
import sys
from collections.abc import Sequence
from dataclasses import dataclass
from datetime import UTC, datetime
from pathlib import Path

from . import build as build_mod
from . import compare as compare_mod
from . import environment as env_mod
from . import execute, fetch
from . import include as include_mod
from . import libraries as lib_mod
from . import run as run_mod
from .paths import CACHE_DIR, LIB_DIR, ROOT
from .problem import Problem
from .record import judge_sha, library_sha

# 組み方 (コンパイラとフラグ) を揃える環境。bundle の pragma と同じく vpclmulqdq を含む。
BASE_ENV = "x64-gcc"

# Codeforces の判定機と同じ命令の CPU。AVX2、BMI2、FMA、pclmul はあり、vpclmulqdq、
# GFNI、AVX-512 は無い。QEMU 8.2 の TCG はそもそも vpclmulqdq を実装していない (-cpu max
# でも出ない) ので、モデルを間違えても見逃さない。Haswell は TCG に無い TSX の警告を出す
# ので -noTSX を使う。-s は真似る CPU のスタックの大きさで、既定の 8 MB では、手元では
# 上限なしで走る深い再帰が落ちる (pj.execute.raise_stack_limit)。
EMULATOR = ("qemu-x86_64", "-cpu", "Haswell-noTSX", "-s", "1024M")
CPU_LABEL = "QEMU Haswell-noTSX (AVX2、BMI2、FMA、pclmul。vpclmulqdq、GFNI、AVX-512 なし)"

# エミュレーションでの時間の制限は tle_sec のこの倍。時間そのものは当てにしない。
TLE_SCALE = 20.0

# 実行時の分岐か、Codeforces の判定機に無い命令を使っている印。コメントを除いたソース
# から探す。Library のヘッダにあれば対象にし、提出の側にもあれば除く (提出が自分で
# その命令を使うのは、測定のための試作で、代わりの経路を持たないことがある)。
MARKERS = re.compile(
    r"__builtin_cpu_supports|vpclmulqdq|clmulepi64_epi128|gfni|gf2p8|avx512|_mm512_|__m512"
)
_COMMENT = re.compile(r"/\*.*?\*/|//[^\n]*", re.DOTALL)

# SIMDe は印の名前をすべて含むが、x86 では使われない。
THIRD_PARTY = ROOT / "third_party"

# 結果の JSON の形の版。サイトが読む。形を変えるときに上げる。
REPORT_SCHEMA = 1

# 落ちたケースの説明の長さ。記録の failed_case と同じ。
DETAIL_CHARS = compare_mod.DIFF_HEAD_CHARS

# 落ちたケースの名前を残す数。記録と同じ。
FAILED_CASES_MAX = run_mod.FAILED_CASES_MAX

# 確かめられなかったもの (テストデータやチェッカを用意できなかった)。失敗にはしない。
SKIP = "SKIP"


@dataclass(frozen=True)
class Target:
    problem: Problem
    submission: Path
    # 印を含む Library のヘッダのラベル。
    headers: tuple[str, ...]


@dataclass(frozen=True)
class Excluded:
    problem: Problem
    submission: Path
    headers: tuple[str, ...]
    reason: str


@dataclass(frozen=True)
class Selection:
    targets: tuple[Target, ...]
    excluded: tuple[Excluded, ...]


@dataclass(frozen=True)
class Outcome:
    problem: str
    submission: str
    headers: tuple[str, ...]
    status: str
    case_count: int = 0
    failed_cases: tuple[str, ...] = ()
    # 最初に落ちたケース。{"name", "status", "detail"}
    failed_case: dict | None = None


def _log(message: str) -> None:
    print(message, file=sys.stderr, flush=True)


def code(text: str) -> str:
    """コメントを除いたソース。コメントに命令の名前を書いただけで対象を変えないため。"""
    return _COMMENT.sub(" ", text)


def _marked(path: Path, cache: dict[Path, bool]) -> bool:
    if path not in cache:
        try:
            cache[path] = MARKERS.search(code(path.read_text(errors="replace"))) is not None
        except OSError:
            cache[path] = False
    return cache[path]


def _rel(path: Path) -> str:
    try:
        return path.relative_to(ROOT).as_posix()
    except ValueError:
        return path.as_posix()


def scan(
    problem: Problem,
    submission: Path,
    libraries: Sequence[lib_mod.Library],
    *,
    search: Sequence[Path] | None = None,
    cache: dict[Path, bool] | None = None,
) -> tuple[tuple[str, ...], tuple[str, ...]]:
    """提出の閉包 (base ならハーネスの閉包も) から、印を含むファイルを集める。

    返すのは (Library のヘッダのラベル, それ以外のファイル)。提出とハーネス自身は
    後者に入る。third_party はどちらにも入れない。cache は問題をまたいで同じファイルを
    読み直さないためのもの。
    """
    search = list(search) if search is not None else build_mod.include_dirs(problem)
    cache = {} if cache is None else cache
    entries = [problem.dir / submission]
    if problem.harness_kind == "base":
        entries.append(problem.base_cpp)
    third_party = THIRD_PARTY.resolve()
    headers: set[str] = set()
    own: set[str] = set()
    for entry in entries:
        found = include_mod.closure(entry, search)
        files: list[tuple[str | None, Path]] = [(None, entry.resolve())]
        files += list(zip(found.labels, found.files, strict=True))
        for label, path in files:
            if not _marked(path, cache):
                continue
            if label is not None and lib_mod.find(label, list(libraries)) is not None:
                headers.add(label)
            elif not path.is_relative_to(third_party):
                own.add(label or _rel(path))
    return tuple(sorted(headers)), tuple(sorted(own))


def runnable(problem: Problem) -> bool:
    """走らせて確かめられる問題か。compile_only はケースが無い。"""
    return fetch.needs_testdata(problem) or problem.compare.kind == "exit_code"


def select(
    problems: Sequence[Problem], libraries: Sequence[lib_mod.Library]
) -> Selection:
    """Library のヘッダに印がある提出を選ぶ。提出の側にも印があるものと、走らせる
    ケースが無いものは除く。"""
    cache: dict[Path, bool] = {}
    targets: list[Target] = []
    excluded: list[Excluded] = []
    for problem in problems:
        for submission in problem.submissions():
            headers, own = scan(problem, submission, libraries, cache=cache)
            if not headers:
                continue
            if not runnable(problem):
                excluded.append(Excluded(problem, submission, headers, "走らせるケースが無い"))
            elif own:
                reason = "提出の側にも印がある (" + ", ".join(own) + ")"
                excluded.append(Excluded(problem, submission, headers, reason))
            else:
                targets.append(Target(problem, submission, headers))
    return Selection(targets=tuple(targets), excluded=tuple(excluded))


def preflight(env: env_mod.Environment, emulator: Sequence[str]) -> str | None:
    """走らせられない理由。走らせられるなら None。"""
    if platform.system() != "Linux" or env_mod.cpu_arch() != "x86_64":
        return (
            f"x86_64 の Linux で走らせてください (ここは {platform.system()} "
            f"{env_mod.cpu_arch()})。手元の Mac なら docker の amd64 のコンテナで走らせます"
        )
    if shutil.which(env.cxx) is None:
        return f"{env.cxx} がありません"
    if not emulator or shutil.which(emulator[0]) is None:
        name = emulator[0] if emulator else "エミュレータ"
        return f"{name} がありません (Ubuntu なら apt-get install qemu-user)"
    if not LIB_DIR.is_dir():
        return "lib/ がありません。Library を lib/ に置いてから走らせてください"
    return None


def judge(
    result: execute.RunResult,
    problem: Problem,
    *,
    limit_sec: float,
    case: fetch.Case | None = None,
    actual: Path | None = None,
    checker: Path | None = None,
) -> tuple[str, str]:
    """エミュレーションで走らせた 1 回の状態と説明。case が無ければ exit_code の問題。

    MLE は見ない。測れるのは QEMU 自身のメモリなので。
    """
    if result.timed_out:
        return "TLE", (
            f"エミュレーションで {limit_sec:g} 秒を超えました "
            f"(制限 {problem.limits.tle_sec:g} 秒の {limit_sec / problem.limits.tle_sec:g} 倍)"
        )
    if result.term_signal == signal.SIGILL:
        return "RE", "SIGILL: CPU に無い命令を実行しました"
    if result.crashed:
        if result.term_signal is not None:
            return "RE", f"signal {result.term_signal}"
        return "RE", f"exit {result.exit_code}"
    if case is None:
        return "AC", "exit 0"
    assert actual is not None
    verdict = compare_mod.compare(
        problem.compare.kind,
        input_path=case.in_path,
        actual_path=actual,
        expected_path=case.out_path,
        checker=checker,
        abs_tol=problem.compare.abs_tol,
        rel_tol=problem.compare.rel_tol,
    )
    return ("AC" if verdict.ok else "WA"), verdict.detail


def _work_dir(problem: Problem, submission: Path) -> Path:
    path = CACHE_DIR / "fallback" / problem.id / submission.stem
    path.mkdir(parents=True, exist_ok=True)
    return path


def check(
    target: Target,
    env: env_mod.Environment,
    *,
    emulator: Sequence[str] = EMULATOR,
    tle_scale: float = TLE_SCALE,
) -> Outcome:
    """1 本を組んで、全ケースをエミュレーションで走らせる。"""
    problem, submission = target.problem, target.submission
    base = {
        "problem": problem.id,
        "submission": submission.as_posix(),
        "headers": target.headers,
    }
    work = _work_dir(problem, submission)
    built = build_mod.build(problem, submission, env, out_dir=work)
    if not built.ok:
        _log(f"  CE ({build_mod.summary(built)})")
        return Outcome(
            **base, status="CE",
            failed_case={"name": "", "status": "CE", "detail": built.log[:DETAIL_CHARS]},
        )
    assert built.binary is not None
    limit = problem.limits.tle_sec * tle_scale
    stdout, stderr = work / "stdout", work / "stderr"

    if not fetch.needs_testdata(problem):
        # exit_code の問題。空の入力で 1 回走らせる。
        result = execute.run(
            built.binary, stdin_path=None, stdout_path=stdout, stderr_path=stderr,
            tle_sec=limit, wrapper=emulator,
        )
        status, detail = judge(result, problem, limit_sec=limit)
        _log(f"    {status:3} {run_mod.SELF_CHECK_CASE}" + (f"  {detail}" if status != "AC" else ""))
        failed = (
            {"name": run_mod.SELF_CHECK_CASE, "status": status, "detail": detail[:DETAIL_CHARS]}
            if status != "AC" else None
        )
        return Outcome(
            **base, status=status, case_count=1,
            failed_cases=(run_mod.SELF_CHECK_CASE,) if failed else (), failed_case=failed,
        )

    try:
        testcases = fetch.ensure(problem, env=env)
        checker = run_mod.prepare_checker(problem, testcases, env, env_mod.cpu_arch())
    except fetch.FetchError as e:
        _log(f"  確かめられません: {e}")
        return Outcome(
            **base, status=SKIP,
            failed_case={"name": "", "status": SKIP, "detail": str(e)[:DETAIL_CHARS]},
        )

    failed_names: list[str] = []
    first: dict | None = None
    for case in testcases.cases:
        result = execute.run(
            built.binary, stdin_path=case.in_path, stdout_path=stdout, stderr_path=stderr,
            tle_sec=limit, wrapper=emulator,
        )
        status, detail = judge(
            result, problem, limit_sec=limit, case=case, actual=stdout, checker=checker
        )
        _log(f"    {status:3} {case.name}" + (f"  {detail}" if status != "AC" else ""))
        if status == "AC":
            continue
        failed_names.append(case.name)
        if first is None:
            first = {"name": case.name, "status": status, "detail": detail[:DETAIL_CHARS]}
        # 時間切れは 1 ケースが長い。run と同じく最初の 1 つで打ち切る。
        if status == "TLE":
            break
    return Outcome(
        **base,
        status=first["status"] if first else "AC",
        case_count=testcases.count,
        failed_cases=tuple(failed_names[:FAILED_CASES_MAX]),
        failed_case=first,
    )


def failing(status: str) -> bool:
    """失敗にする状態か。確かめられなかったもの (SKIP) は数えない。"""
    return status not in ("AC", SKIP)


def failures(outcomes: Sequence[Outcome]) -> list[Outcome]:
    return [o for o in outcomes if failing(o.status)]


def report(
    outcomes: Sequence[Outcome],
    excluded: Sequence[Excluded],
    env: env_mod.Environment,
    emulator: Sequence[str],
) -> dict:
    """結果の JSON。サイトが読む。"""
    return {
        "schema": REPORT_SCHEMA,
        "generated_at": datetime.now(UTC).replace(microsecond=0).isoformat().replace("+00:00", "Z"),
        "judge_sha": judge_sha(),
        "library_sha": library_sha(),
        "env": env.name,
        "cpu": CPU_LABEL,
        "emulator": shlex.join(emulator),
        "results": [
            {
                "problem": o.problem,
                "submission": o.submission,
                "headers": list(o.headers),
                "status": o.status,
                "case_count": o.case_count,
                "failed_cases": list(o.failed_cases),
                "failed_case": o.failed_case,
            }
            for o in outcomes
        ],
        "excluded": [
            {
                "problem": e.problem.id,
                "submission": e.submission.as_posix(),
                "headers": list(e.headers),
                "reason": e.reason,
            }
            for e in excluded
        ],
    }


def _cell(text: str) -> str:
    """Markdown の表の 1 マス。縦棒と改行で表が崩れないようにする。"""
    return text.replace("|", "\\|").replace("\n", " ")


def summary_markdown(outcomes: Sequence[Outcome], excluded: Sequence[Excluded]) -> str:
    """GitHub のジョブの要約。"""
    bad = failures(outcomes)
    skipped = [o for o in outcomes if o.status == SKIP]
    lines = [
        "## 代わりの経路",
        "",
        f"{CPU_LABEL} で、Library の実行時の分岐を通る提出を全ケース走らせました。",
        "",
        (
            f"対象 {len(outcomes)} 本: AC {len(outcomes) - len(bad) - len(skipped)} 本、"
            f"失敗 {len(bad)} 本、確かめられなかった {len(skipped)} 本"
        ),
        "",
    ]
    if outcomes:
        lines += ["| 問題 | 提出 | 状態 | ケース | 落ちたケース |", "|---|---|---|---|---|"]
        for o in outcomes:
            failed = ""
            if o.failed_case:
                name = o.failed_case.get("name") or ""
                detail = (o.failed_case.get("detail") or "").splitlines()
                failed = name + (f": {detail[0]}" if detail else "")
            lines.append(
                f"| {_cell(o.problem)} | {_cell(o.submission)} | {o.status} | "
                f"{o.case_count} | {_cell(failed)} |"
            )
        lines.append("")
    if excluded:
        lines.append(f"除いた提出 {len(excluded)} 本:")
        lines.append("")
        lines += [f"- {e.problem.id} {e.submission.as_posix()}: {e.reason}" for e in excluded]
        lines.append("")
    return "\n".join(lines)
