"""走らせる対象を決めて、実行して、採点レコードを作る。

CPU モデルはジョブが始まるまで分からないので、そのモデルで実際に何が未計測か
の判定はここでやる。モデルに依存しない判断 (環境ごとのジョブの本数と、束を
片付ける順番) は pj.plan が先に決める。
"""

from __future__ import annotations

import sys
from collections.abc import Iterable, Mapping, Sequence
from dataclasses import dataclass, field
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
class Worklist:
    """この回に走らせる対象と、走らせなかったものの内訳。

    ジョブが始まる前に立てる pj.plan の計画とは別物。あちらは環境ごとの
    ジョブの本数と束の順番を決め、こちらは引いたモデルで実際に何を走らせるかを
    決める。
    """

    jobs: tuple[Job, ...]
    skipped: tuple[Job, ...]
    pending: tuple[Target, ...]
    unresolved: tuple[tuple[str, str], ...]
    # 取得に失敗した問題。(問題 id, 理由)
    failed: tuple[tuple[str, str], ...] = ()
    # include を解決できないので走らせなかった提出。
    blocked: tuple[Target, ...] = ()
    # budget に達したので見もしなかった提出。次の実行が拾う。
    held: tuple[Target, ...] = ()
    # テストデータが borrowed と違っていた問題。(問題 id, 借りた値, 本物)
    moved: tuple[tuple[str, str, str], ...] = ()


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
    # 渡された順を保つ。束の順は plan が重い順に決めていて、budget で打ち切る
    # ときにどれが残るかがその順で決まる。ここで並べ直すと意味が変わる。
    return list(grouped.values())


def build_worklist(
    targets: Sequence[Target],
    env: env_mod.Environment,
    machine: Machine,
    known_keys: set[str],
    *,
    borrowed: Mapping[str, str] | None = None,
    allow_fetch: bool = True,
    refresh: bool = False,
    budget: int | None = None,
) -> Worklist:
    """各提出のキーを計算して、記録にあるものを除く。

    キーには cases_hash が要るが、そのために毎回テストデータを落とすのは高い。
    手元に manifest があればそれが本物なのでそれを使い、無ければ記録から借りた
    値で仮に判定する。借りた値で「走らせるものが 0 件」と出た問題には、
    テストデータを 1 バイトも触らない。

    1 件でも走らせるなら、そこで初めて取得する。取得した本物が借りた値と違って
    いたら、その問題の判定を本物で組み直す。どの記録にも無いキーになるので、
    その問題の提出が全部未計測に戻る。

    borrowed を渡さないと借用をしない。その場合は今までどおり、判定のために
    テストデータを取りに行く。

    budget を渡すと、走らせる対象がその件数に達したところで見るのをやめる。
    束の途中でも止める。budget は 6 時間で打ち切られないための上限なので、
    大きい束に当たったときこそ効いてほしい。残りは次の実行が同じ順で拾う。
    """
    state = _State()
    borrowed = borrowed or {}

    for problem, submissions in _group_by_problem(targets):
        known = _known_cases_hash(problem, refresh=refresh)
        if known is not None:
            guess = known
        elif refresh:
            # --refresh は疑って取り直す口。借りた値で早じまいさせない。
            guess = None
        else:
            guess = borrowed.get(problem.id)

        if guess is not None:
            decided = _decide(problem, submissions, guess, env, machine, known_keys)
            if not decided.jobs or known is not None:
                # 走らせるものが無いか、手元の manifest が本物か。どちらでも
                # 取得は要らない。前者はテストデータに触らずに次の問題へ行く。
                if state.absorb(decided, targets, budget):
                    return state.finish()
                continue

        if not allow_fetch:
            state.pending.extend((problem, s) for s in submissions)
            continue
        try:
            real = fetch.ensure(problem, refresh=refresh).cases_hash
        except fetch.FetchError as e:
            # 1 問取れなかっただけで、走れる問題の記録まで落とさない。
            # 取りこぼしは次の実行が拾う。
            state.failed.append((problem.id, str(e)))
            state.pending.extend((problem, s) for s in submissions)
            continue

        if guess is None or real != guess:
            if guess is not None:
                state.moved.append((problem.id, guess, real))
            decided = _decide(problem, submissions, real, env, machine, known_keys)
        if state.absorb(decided, targets, budget):
            return state.finish()

    return state.finish()


@dataclass
class _Decision:
    jobs: list[Job]
    skipped: list[Job]
    blocked: list[Target]
    unresolved: list[tuple[str, str]]


@dataclass
class _State:
    jobs: list[Job] = field(default_factory=list)
    skipped: list[Job] = field(default_factory=list)
    pending: list[Target] = field(default_factory=list)
    unresolved: list[tuple[str, str]] = field(default_factory=list)
    failed: list[tuple[str, str]] = field(default_factory=list)
    blocked: list[Target] = field(default_factory=list)
    held: list[Target] = field(default_factory=list)
    moved: list[tuple[str, str, str]] = field(default_factory=list)

    def absorb(
        self,
        decided: _Decision,
        targets: Sequence[Target],
        budget: int | None,
    ) -> bool:
        """1 問ぶんの判定を取り込む。budget に達したら True を返す。"""
        self.skipped.extend(decided.skipped)
        self.blocked.extend(decided.blocked)
        self.unresolved.extend(decided.unresolved)
        for job in decided.jobs:
            self.jobs.append(job)
            if budget is not None and len(self.jobs) >= budget:
                self.held.extend(_rest(targets, job.problem, job.submission))
                return True
        return False

    def finish(self) -> Worklist:
        return Worklist(
            jobs=tuple(self.jobs),
            skipped=tuple(self.skipped),
            pending=tuple(self.pending),
            unresolved=tuple(self.unresolved),
            blocked=tuple(self.blocked),
            failed=tuple(self.failed),
            held=tuple(self.held),
            moved=tuple(self.moved),
        )


def _known_cases_hash(problem: Problem, *, refresh: bool) -> str | None:
    """取得せずに分かる本物の cases_hash。

    テストデータを持たない問題は空文字で、これも本物。--refresh のときは
    手元のものを信じないので None にして取り直させる。
    """
    if not fetch.needs_testdata(problem):
        return ""
    if refresh:
        return None
    return fetch.cached_cases_hash(problem)


def _decide(
    problem: Problem,
    submissions: Sequence[Path],
    cases_hash: str,
    env: env_mod.Environment,
    machine: Machine,
    known_keys: set[str],
) -> _Decision:
    """この cases_hash を前提に、走らせるものと飛ばすものを分ける。"""
    cxxflags = build_mod.effective_cxxflags(env, problem)
    search_paths = build_mod.include_dirs(problem)
    harness = key_mod.harness_hash(problem, search_paths)
    problem_h = key_mod.problem_hash(problem)
    decided = _Decision(jobs=[], skipped=[], blocked=[], unresolved=[])

    for submission in submissions:
        sub = key_mod.submission_hash(problem.dir / submission, search_paths)
        if sub.unresolved:
            # 閉包が欠けたままではキーが別の意味になる。ライブラリを取れな
            # かった回に CE の記録を残すより、解決できる回まで待つ方がよい。
            for target in sub.unresolved:
                decided.unresolved.append((submission.as_posix(), target))
            decided.blocked.append((problem, submission))
            continue
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
        (decided.skipped if job.key in known_keys else decided.jobs).append(job)
    return decided


def _rest(
    targets: Sequence[Target], problem: Problem, submission: Path
) -> list[Target]:
    """この提出より後ろに並んでいる対象。budget で止めたときの残り。"""
    for index, target in enumerate(targets):
        if target[0].id == problem.id and target[1] == submission:
            return list(targets[index + 1 :])
    return []


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
        if testcases.cases_hash != job.cases_hash:
            # キーを決めたときと違う相手を測ろうとしている。そのまま走らせると
            # 記録の cases_hash が実際に測ったものと食い違う。記録が嘘をつく
            # より、ここで止めた方がよい。既に出した記録は書き出し済み。
            raise fetch.FetchError(
                f"{problem.id}: テストデータが入れ替わりました "
                f"({job.cases_hash} -> {testcases.cases_hash})"
            )
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
