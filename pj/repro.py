"""1 提出を手元で走らせて、落ちたケースの差分とファイルの場所を出す。

サイトと記録は「どのケースで落ちたか」と差分の先頭までしか持たない。そこから
先を手作業にしないための口で、記録は書かない。手元のコンパイラは CI と違うので
結果が同じとは限らないが、WA と RE の大半はここで再現する。
"""

from __future__ import annotations

import difflib
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import TextIO

from . import build as build_mod
from . import environment as env_mod
from . import execute, fetch
from . import run as run_mod
from .paths import CACHE_DIR
from .problem import Problem

# 差分を出す行数の上限。全部は端末に収まらない。ファイルの場所は別に出す。
DIFF_LINES = 200
# ケースが見つからないときに例として並べる数。
SUGGEST_CASES = 10


class ReproError(Exception):
    """走らせられないときに投げる。"""


@dataclass(frozen=True)
class CaseReport:
    name: str
    status: str
    time_ms: int
    memory_kb: int
    detail: str
    in_path: Path
    out_path: Path
    actual_path: Path
    stderr_path: Path


def work_dir(problem: Problem, submission: Path, env_name: str) -> Path:
    path = CACHE_DIR / "repro" / problem.id / env_name / submission.stem
    path.mkdir(parents=True, exist_ok=True)
    return path


def repro(
    problem: Problem,
    submission: Path,
    env: env_mod.Environment,
    *,
    case: str | None = None,
    out: TextIO = sys.stdout,
) -> int:
    """走らせて結果を out に書く。全部 AC なら 0、そうでなければ 1。"""
    print(f"{env.cxx} でコンパイルします", file=out)
    built = build_mod.build(problem, submission, env)
    if not built.ok:
        print(f"CE ({built.seconds:.1f}s)", file=out)
        print(built.log, file=out)
        return 1
    assert built.binary is not None
    print(f"コンパイル完了 ({built.seconds:.1f}s, {built.binary.stat().st_size} bytes)", file=out)

    if not fetch.needs_testdata(problem):
        if problem.compare.kind == "exit_code":
            return _self_check(problem, submission, env, built.binary, out)
        print("テストデータを使わない問題です。組めたので終わります。", file=out)
        return 0

    testcases = fetch.ensure(problem)
    cases = list(testcases.cases)
    if case is not None:
        cases = [c for c in cases if c.name == case]
        if not cases:
            names = ", ".join(c.name for c in testcases.cases[:SUGGEST_CASES])
            raise ReproError(f"ケース {case!r} がありません (例: {names})")

    checker = run_mod.prepare_checker(problem, testcases, env, env_mod.cpu_arch())
    work = work_dir(problem, submission, env.name)
    execute.warmup(built.binary, tle_sec=problem.limits.tle_sec)

    failures: list[CaseReport] = []
    for c in cases:
        actual = work / f"{c.name}.out"
        stderr = work / f"{c.name}.err"
        result = execute.run(
            built.binary,
            stdin_path=c.in_path,
            stdout_path=actual,
            stderr_path=stderr,
            tle_sec=problem.limits.tle_sec,
        )
        status, detail = run_mod.judge_case(result, c, problem, actual, checker)
        print(f"  {status:3} {c.name}  {result.wall_ms} ms  {result.max_rss_kb} KB", file=out)
        if status != "AC":
            failures.append(
                CaseReport(
                    name=c.name, status=status, time_ms=result.wall_ms,
                    memory_kb=result.max_rss_kb, detail=detail,
                    in_path=c.in_path, out_path=c.out_path,
                    actual_path=actual, stderr_path=stderr,
                )
            )

    for report in failures:
        _report(report, out)
    print(
        f"\n{len(cases)} ケース中 {len(cases) - len(failures)} 件 AC / {len(failures)} 件 失敗",
        file=out,
    )
    return 1 if failures else 0


def _self_check(
    problem: Problem, submission: Path, env: env_mod.Environment, binary: Path, out: TextIO
) -> int:
    """exit_code の問題。空の入力で 1 回走らせて、終了コードを見る。"""
    name = run_mod.SELF_CHECK_CASE
    work = work_dir(problem, submission, env.name)
    actual, stderr = work / f"{name}.out", work / f"{name}.err"
    execute.warmup(binary, tle_sec=problem.limits.tle_sec)
    result = execute.run(
        binary, stdin_path=None, stdout_path=actual, stderr_path=stderr,
        tle_sec=problem.limits.tle_sec,
    )
    status, detail = run_mod.judge_exit_code(result, problem, stderr)
    print(f"  {status:3} {name}  {result.wall_ms} ms  {result.max_rss_kb} KB", file=out)
    if status == "AC":
        print("\n終了コード 0 で走り切りました", file=out)
        return 0
    print(f"\n--- {name}: {status} ({result.wall_ms} ms, {result.max_rss_kb} KB)", file=out)
    if detail:
        print(detail, file=out)
    print(f"標準出力   {actual}", file=out)
    print(f"stderr     {stderr}", file=out)
    return 1


def _report(report: CaseReport, out: TextIO) -> None:
    print(f"\n--- {report.name}: {report.status} ({report.time_ms} ms, {report.memory_kb} KB)", file=out)
    if report.detail:
        print(report.detail, file=out)
    print(f"入力       {report.in_path}", file=out)
    print(f"期待出力   {report.out_path}", file=out)
    print(f"実際の出力 {report.actual_path}", file=out)
    print(f"stderr     {report.stderr_path}", file=out)
    if report.status == "WA":
        for line in unified_diff(report.out_path, report.actual_path):
            print(line, file=out)


def unified_diff(expected_path: Path, actual_path: Path) -> list[str]:
    """期待出力と実際の出力の差分。長ければ途中で切る。"""
    expected = expected_path.read_text(errors="replace").splitlines()
    actual = actual_path.read_text(errors="replace").splitlines()
    lines = list(
        difflib.unified_diff(expected, actual, fromfile="期待", tofile="実際", n=1, lineterm="")
    )
    if len(lines) > DIFF_LINES:
        rest = len(lines) - DIFF_LINES
        lines = lines[:DIFF_LINES] + [f"... (あと {rest} 行。ファイルを直接見てください)"]
    return lines
