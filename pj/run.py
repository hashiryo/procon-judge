"""走らせる対象を決めて、実行して、採点レコードを作る。

CPU モデルはジョブが始まるまで分からないので、何を走らせるかの判定もここでやる。
事前に計画を立てる別のジョブは要らない。
"""

from __future__ import annotations

import sys
from collections.abc import Iterable, Sequence
from dataclasses import dataclass
from pathlib import Path

from . import build as build_mod
from . import compare as compare_mod
from . import environment as env_mod
from . import execute, fetch
from . import key as key_mod
from .paths import CACHE_DIR
from .problem import Problem
from .record import FailedCase, Record, judge_sha, library_sha

Target = tuple[Problem, Path]


@dataclass(frozen=True)
class Machine:
    """走っているマシンとコンパイラの素性。キーの一部になる。"""

    cpu_arch: str
    cpu_model: str
    compiler_version: str

    @classmethod
    def detect(cls, env: env_mod.Environment) -> Machine:
        return cls(
            cpu_arch=env_mod.cpu_arch(),
            cpu_model=env_mod.cpu_model(),
            compiler_version=env_mod.compiler_version(env.cxx),
        )


@dataclass(frozen=True)
class Job:
    problem: Problem
    submission: Path
    env: env_mod.Environment
    machine: Machine
    key: str
    submission_hash: str
    includes: tuple[str, ...]
    cases_hash: str
    cxxflags: str

    @property
    def label(self) -> str:
        return f"{self.problem.id}\t{self.submission.as_posix()}"


@dataclass(frozen=True)
class Plan:
    jobs: tuple[Job, ...]
    skipped: tuple[Job, ...]
    pending: tuple[Target, ...]
    unresolved: tuple[tuple[str, str], ...]


@dataclass
class CaseOutcome:
    name: str
    status: str
    time_ms: int
    memory_kb: int
    algo_time_ns: int | None
    detail: str


def _log(message: str) -> None:
    print(message, file=sys.stderr, flush=True)


def _work_dir(problem: Problem, submission: Path, env_name: str) -> Path:
    path = CACHE_DIR / "run" / problem.id / env_name / submission.stem
    path.mkdir(parents=True, exist_ok=True)
    return path


def _group_by_problem(targets: Iterable[Target]) -> list[tuple[Problem, list[Path]]]:
    grouped: dict[str, tuple[Problem, list[Path]]] = {}
    for problem, submission in targets:
        grouped.setdefault(problem.id, (problem, []))[1].append(submission)
    return [grouped[pid] for pid in sorted(grouped)]


def build_plan(
    targets: Sequence[Target],
    env: env_mod.Environment,
    machine: Machine,
    known_keys: set[str],
    *,
    allow_fetch: bool = True,
    refresh: bool = False,
) -> Plan:
    """各提出のキーを計算して、記録にあるものを除く。

    キーには cases_hash が要る。手元のキャッシュに manifest があればそれで済むので、
    すべてスキップされる実行では 1 件も落とさない。キャッシュが無いときだけ取りに行く。
    """
    jobs: list[Job] = []
    skipped: list[Job] = []
    pending: list[Target] = []
    unresolved: list[tuple[str, str]] = []

    for problem, submissions in _group_by_problem(targets):
        cases_hash = fetch.cached_cases_hash(problem)
        if cases_hash is None or (refresh and fetch.needs_testdata(problem)):
            if not allow_fetch:
                pending.extend((problem, s) for s in submissions)
                continue
            cases_hash = fetch.ensure(problem, refresh=refresh).cases_hash

        cxxflags = build_mod.effective_cxxflags(env, problem)
        search_paths = build_mod.include_dirs(problem)
        harness = key_mod.harness_hash(problem)
        problem_h = key_mod.problem_hash(problem)

        for submission in submissions:
            sub = key_mod.submission_hash(problem.dir / submission, search_paths)
            for target in sub.unresolved:
                unresolved.append((submission.as_posix(), target))
            job = Job(
                problem=problem,
                submission=submission,
                env=env,
                machine=machine,
                key=key_mod.compute(
                    submission=submission.as_posix(),
                    submission_hash=sub.submission_hash,
                    harness_hash=harness,
                    problem_hash=problem_h,
                    cases_hash=cases_hash,
                    env=env.name,
                    compiler_version=machine.compiler_version,
                    cxxflags=cxxflags,
                    cpu_model=machine.cpu_model,
                ),
                submission_hash=sub.submission_hash,
                includes=sub.includes,
                cases_hash=cases_hash,
                cxxflags=cxxflags,
            )
            (skipped if job.key in known_keys else jobs).append(job)

    return Plan(
        jobs=tuple(jobs),
        skipped=tuple(skipped),
        pending=tuple(pending),
        unresolved=tuple(unresolved),
    )



def _judge_case(
    result: execute.RunResult,
    case: fetch.Case,
    problem: Problem,
    actual_path: Path,
    checker: Path | None,
) -> tuple[str, str]:
    """1 ケースの状態と、失敗したときの説明を返す。"""
    if result.timed_out:
        return "TLE", f"{problem.limits.tle_sec} 秒を超えました"
    if result.max_rss_kb > problem.limits.mle_mb * 1024:
        return "MLE", f"{result.max_rss_kb} KB (上限 {problem.limits.mle_mb * 1024} KB)"
    if result.crashed:
        if result.term_signal is not None:
            return "RE", f"signal {result.term_signal}"
        return "RE", f"exit {result.exit_code}"
    verdict = compare_mod.compare(
        problem.compare.kind,
        input_path=case.in_path,
        actual_path=actual_path,
        expected_path=case.out_path,
        checker=checker,
    )
    return ("AC" if verdict.ok else "WA"), verdict.detail


def execute_job(job: Job) -> Record:
    """1 件走らせて記録を返す。"""
    problem, submission, env = job.problem, job.submission, job.env

    testcases = None
    if fetch.needs_testdata(problem):
        testcases = fetch.ensure(problem)
        _log(f"  テストデータ {testcases.count} ケース ({testcases.dir})")

    _log(f"  {env.cxx} でコンパイルします")
    built = build_mod.build(problem, submission, env)

    base = {
        "key": job.key,
        "problem": problem.id,
        "submission": submission.as_posix(),
        "env": env.name,
        "cpu_arch": job.machine.cpu_arch,
        "cpu_model": job.machine.cpu_model,
        "compiler_version": job.machine.compiler_version,
        "cxxflags": job.cxxflags,
        "cases_hash": job.cases_hash,
        "case_count": testcases.count if testcases else 0,
        "submission_hash": job.submission_hash,
        "includes": list(job.includes),
        "library_sha": library_sha(),
        "judge_sha": judge_sha(),
        "source_bytes": (problem.dir / submission).stat().st_size,
    }

    if not built.ok:
        _log(f"  CE ({built.seconds:.1f}s)")
        if built.log:
            _log(_indent(built.log))
        return Record(
            **base,
            status="CE",
            time_max_ms=0,
            time_total_ms=0,
            algo_time_max_ns=None,
            algo_time_total_ns=None,
            memory_max_kb=0,
            binary_bytes=None,
            failed_case=FailedCase(
                name="", status="CE", time_ms=0, memory_kb=0,
                detail=built.log[: compare_mod.DIFF_HEAD_CHARS],
            ),
        )

    assert built.binary is not None
    _log(f"  コンパイル完了 ({built.seconds:.1f}s, {built.binary.stat().st_size} bytes)")
    if built.log:
        _log(_indent(built.log))

    binary_bytes = built.binary.stat().st_size
    if testcases is None:
        return Record(
            **base, status="AC", time_max_ms=0, time_total_ms=0,
            algo_time_max_ns=None, algo_time_total_ns=None, memory_max_kb=0,
            binary_bytes=binary_bytes,
        )

    checker = None
    if problem.compare.kind == "checker":
        source = testcases.checker_source()
        if source is None:
            raise fetch.FetchError(
                f"{problem.id}: compare.kind = 'checker' なのに checker.cpp がありません"
            )
        # テストデータのキャッシュは 4 環境で共有するので、名前にアーキテクチャを
        # 入れる。x86 で組んだチェッカが arm のジョブに復元されると動かない。
        checker = build_mod.build_checker(
            source, testcases.dir / f"checker-{job.machine.cpu_arch}.bin", env
        )
        if checker is None:
            raise fetch.FetchError(f"{source} のコンパイルに失敗しました")

    work = _work_dir(problem, submission, env.name)
    actual_path, stderr_path = work / "stdout", work / "stderr"

    execute.warmup(built.binary, tle_sec=problem.limits.tle_sec)

    outcomes: list[CaseOutcome] = []
    for case in testcases.cases:
        result = execute.run(
            built.binary,
            stdin_path=case.in_path,
            stdout_path=actual_path,
            stderr_path=stderr_path,
            tle_sec=problem.limits.tle_sec,
        )
        status, detail = _judge_case(result, case, problem, actual_path, checker)
        outcomes.append(
            CaseOutcome(
                name=case.name, status=status, time_ms=result.wall_ms,
                memory_kb=result.max_rss_kb, algo_time_ns=result.algo_time_ns,
                detail=detail,
            )
        )
        _log(
            f"    {status:3} {case.name}  {result.wall_ms} ms  "
            f"{result.max_rss_kb} KB" + (f"  {detail}" if status != "AC" else "")
        )
        # 最初の非 AC で打ち切る。残りは走らせない。
        if status != "AC":
            break

    return _summarize(base, outcomes, binary_bytes)


def _summarize(base: dict, outcomes: list[CaseOutcome], binary_bytes: int) -> Record:
    failed = next((o for o in outcomes if o.status != "AC"), None)
    algo = [o.algo_time_ns for o in outcomes if o.algo_time_ns is not None]
    return Record(
        **base,
        status=failed.status if failed else "AC",
        time_max_ms=max((o.time_ms for o in outcomes), default=0),
        time_total_ms=sum(o.time_ms for o in outcomes),
        algo_time_max_ns=max(algo) if algo else None,
        algo_time_total_ns=sum(algo) if algo else None,
        memory_max_kb=max((o.memory_kb for o in outcomes), default=0),
        binary_bytes=binary_bytes,
        failed_case=(
            FailedCase(
                name=failed.name, status=failed.status, time_ms=failed.time_ms,
                memory_kb=failed.memory_kb,
                detail=failed.detail[: compare_mod.DIFF_HEAD_CHARS],
            )
            if failed
            else None
        ),
    )


def _indent(text: str, prefix: str = "    | ") -> str:
    return "\n".join(prefix + line for line in text.splitlines())
