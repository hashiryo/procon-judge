"""ジョブが始まる前に決められることを決める。

CPU モデルはマシンが割り当たった瞬間に決まるので、ジョブが始まる前には
分からない。だからモデルに依存しない判断だけをここで済ませて、依存する判断は
run の中に残す。ここが決めるのは、組 (x64 / arm、pj.environment.groups) ごとに
何本のジョブを立てるかと、問題を見る順番 (重い順) の 2 つ。どの問題をどのジョブが
測るかは決めない。それは run のジョブが起動してから宣言 (pj.claims) で取る。
1 本のジョブは組の全環境 (gcc と clang) を測るので、稀な CPU モデルに当たった
1 本がその場で両方を埋める。

テストデータは 1 バイトも触らない。キーに要る cases_hash は既存の記録から
借りる。記録が無ければそれは未計測なので、借りる必要もない。

「今のソースならこのキーになるはず」を出すのは Freshness.key_for で、記録が
古いかどうかを見るのと同じ道具を使う。同じ recipe を二度書くと、片方を直し
忘れたときに記録の意味が静かにずれる。
"""

from __future__ import annotations

import math
from collections.abc import Iterable, Mapping, Sequence
from dataclasses import dataclass

from . import batch as batch_mod
from .environment import Environment, group_name, toolchain
from .freshness import Freshness
from .problem import Problem
from .store import Store

# 同時実行は Free で 20 本で、超えたぶんは GitHub が待たせるだけで落ちはしない。
# 上限を同時実行より多く取るのは、CPU モデルの当たりを引き直すため。未計測は
# よく当たるモデルでは埋まり、稀なモデルに残る (2026-09-22 の実測: EPYC 7763 と
# 9V74 は 4 件、Xeon 8370C は 900 件超)。当たったモデルに仕事が無いジョブは
# 1 分ほどで終わって次のジョブに枠を渡すので、本数を増やすほど稀なモデルに
# 当たる回数が増える。1 組 (x64) が gcc と clang の両方を担うので、環境ごとに
# 20 本だったときと引き直しの回数を揃えるために 40 本。arm はモデルが 1 つで
# 仕事が少なく、本数は x64 に寄る。
MAX_JOBS_PER_GROUP = 40

# 1 本のジョブに見込む未計測の件数。本数を決めるためだけの値で、ジョブはこの
# 件数で止まらない (宣言が尽きるか時間の上限まで測る)。1 ジョブは 1 分に 5 件
# ほど測るので、50 件は 10 分ほど。少なめに見て早く本数を増やす。当たった
# モデルに仕事が無いジョブは 1 分ほどで終わるので、余っても安い。
ITEMS_PER_JOB = 50

# 1 ジョブの実行時間の上限 (分)。これを過ぎたら次の問題を宣言しない。
# 6 時間の上限に対して、最後の 1 問 (最悪で 提出数 × ケース数 × tle_sec) と
# upload のぶんを残して 4 時間に置く。
DEFAULT_MINUTES = 240

# 記録がまだ 1 件も無い問題のケース数の仮置き。費用は問題を重い順に並べる
# ためだけに使うので、外れても順番が少し入れ替わるだけで済む。
ASSUMED_CASE_COUNT = 20


@dataclass(frozen=True)
class Work:
    """1 問題ぶんの仕事の見積もり。順番と本数を決めるためにある。

    run のジョブが宣言して取る単位もこの 1 問題 (× 引いた CPU モデル) で、
    その問題のそのモデルで未計測の提出を全部測る。テストデータの取得もその
    1 回で済む。
    """

    problem: str
    # 既知の CPU モデルごとの、未計測の提出。モデルが 1 つも無い環境では
    # 空文字のキーに全提出が入る。
    todo: dict[str, tuple[str, ...]]
    # 費用の見積もり (ms)。1 モデルあたりの期待値。
    weight: float

    @property
    def expected(self) -> float:
        """1 モデルあたりの未計測の件数の期待値。"""
        return sum(len(v) for v in self.todo.values()) / len(self.todo)

    @property
    def total(self) -> int:
        """既知のモデル全部を合わせた未計測の件数。ジョブの本数を決めるのに使う。"""
        return sum(len(v) for v in self.todo.values())


@dataclass(frozen=True)
class EnvPlan:
    env: Environment
    models: tuple[str, ...]
    # 重い順。組の並びはこれを環境ごとに足して作る。
    works: tuple[Work, ...]
    # include を解決できなかった提出。ライブラリを取れていないと全部並ぶ。
    unresolved: tuple[str, ...] = ()
    # 今のソースで CE になると分かっている提出。他のモデルでも必ず CE になる。
    compile_errors: tuple[str, ...] = ()

    @property
    def order(self) -> tuple[str, ...]:
        return tuple(w.problem for w in self.works)

    @property
    def expected(self) -> float:
        return sum(w.expected for w in self.works)

    @property
    def total(self) -> int:
        return sum(w.total for w in self.works)

    @property
    def jobs(self) -> int:
        """この環境だけで組を作ったときの本数。組の本数は group が全環境の合計から決める。"""
        return job_count(self.total)


@dataclass(frozen=True)
class GroupPlan:
    """CI のジョブの単位 (x64 / arm) ぶんの計画。matrix の 1 環境ぶんだった位置。"""

    name: str
    runs_on: str
    envs: tuple[Environment, ...]
    # 組の全環境を合わせた重い順。run のジョブはこの並びを (番号でずらして) 見ていく。
    order: tuple[str, ...]
    # 全環境と全モデルを合わせた未計測の件数。本数を決めるのに使う。
    total: int
    jobs: int
    # run に渡す時間の上限 (分)。
    minutes: int = DEFAULT_MINUTES

    @property
    def env_names(self) -> tuple[str, ...]:
        return tuple(e.name for e in self.envs)


def build(
    problems: Sequence[Problem],
    envs: Sequence[Environment],
    store: Store,
    *,
    minutes: int = DEFAULT_MINUTES,
) -> list[EnvPlan]:
    """CI で走らせる環境それぞれの計画。"""
    return for_envs([e for e in envs if e.runs_on != "self"], problems, envs, store)


def for_envs(
    targets: Sequence[Environment],
    problems: Sequence[Problem],
    envs: Sequence[Environment],
    store: Store,
) -> list[EnvPlan]:
    """指定した環境ぶんの計画。run のジョブが自分の組の並びを組み直すのにも使う。

    並びは記録と問題定義と lib/ だけから決まるので、同じものを読めば全ジョブが
    同じ順を作る。
    """
    keys = store.keys()
    records = {p.id: list(store.read(p.id)) for p in problems}
    # Freshness は提出のハッシュを問題につき一度だけ作る。環境をまたいで
    # 使い回さないと、閉包を環境の数だけ辿り直すことになる。
    fresh = {p.id: Freshness(p, envs) for p in problems}
    cases = store.cases_hashes()
    return [_for_env(env, problems, records, fresh, keys, cases) for env in targets]


def for_env(
    env: Environment,
    problems: Sequence[Problem],
    envs: Sequence[Environment],
    store: Store,
) -> EnvPlan:
    """1 環境ぶん。"""
    return for_envs([env], problems, envs, store)[0]


def combined_order(plans: Sequence[EnvPlan]) -> tuple[str, ...]:
    """組の全環境を合わせた重い順。同じ問題は環境ごとの費用を足す。

    同じ重さのときは id で並べて、全ジョブが同じ順を作れるようにする。
    """
    weight: dict[str, float] = {}
    for plan in plans:
        for work in plan.works:
            weight[work.problem] = weight.get(work.problem, 0.0) + work.weight
    return tuple(sorted(weight, key=lambda pid: (-weight[pid], pid)))


def group(plans: Sequence[EnvPlan], *, minutes: int = DEFAULT_MINUTES) -> list[GroupPlan]:
    """環境ごとの計画を組 (ジョブの単位) にまとめる。"""
    by_group: dict[str, list[EnvPlan]] = {}
    for plan in plans:
        by_group.setdefault(group_name(plan.env), []).append(plan)
    out = []
    for name, members in by_group.items():
        total = sum(p.total for p in members)
        out.append(
            GroupPlan(
                name=name,
                runs_on=members[0].env.runs_on,
                envs=tuple(p.env for p in members),
                order=combined_order(members),
                total=total,
                jobs=job_count(total),
                minutes=minutes,
            )
        )
    return out


def job_count(total: int) -> int:
    """立てるジョブの本数。

    既知のモデルすべてで 0 件ならジョブを立てない。未知のモデルに当たれば
    仕事はあるが、それは計画された仕事ではない。新しい CPU モデルを探しには
    行かない。

    仕事はモデルごとにあり、ジョブは当たったモデルのぶんしか測れない。それでも
    本数は全モデルを合わせた件数で数える。1 モデルあたりの平均で数えると、モデルが
    6 つある x64 で本当の仕事の 1/6 しか見ないことになり、本数が足りなくなる。
    """
    if total <= 0:
        return 0
    return min(MAX_JOBS_PER_GROUP, max(1, math.ceil(total / ITEMS_PER_JOB)))


def rotation(order: Sequence[str], job: int, jobs: int) -> list[str]:
    """j 番目のジョブが問題を見ていく順番。

    全ジョブが同じ並びを持つので、開始点を j/N の位置にずらして末尾から先頭へ
    回る。開始点が散っていれば、同じ問題を同じ瞬間に宣言しようとすることが
    ほぼ起きない。宣言済みのものは run が飛ばすので、取りこぼしも重複も無い。
    時間切れで持ち越される問題が並びの末尾に偏らない副作用もある。
    """
    if jobs < 1:
        raise ValueError(f"ジョブの本数が {jobs} です")
    if not 0 <= job < jobs:
        raise ValueError(f"ジョブ番号 {job} が本数 {jobs} に収まりません")
    start = len(order) * job // jobs
    return list(order[start:]) + list(order[:start])


def matrix(plans: Iterable[GroupPlan]) -> dict:
    """judge.yml が fromJSON で受ける形。1 行が 1 ジョブで、組の全環境を担う。

    問題の並びは入れない。ジョブ名に全部並ぶと読めなくなるし、問題が増えると
    マトリクスが膨らむ。run は並びを自分で組み直せる。
    """
    include = [
        {
            "group": plan.name,
            "runs_on": plan.runs_on,
            # pj run --env に渡す。コンマ区切り。
            "envs": ",".join(plan.env_names),
            # judge.yml が入れるコンパイラ。envs と同じ順。
            "toolchains": ",".join(toolchain(e) for e in plan.envs),
            "job": job,
            "jobs": plan.jobs,
            "minutes": plan.minutes,
        }
        for plan in plans
        for job in range(plan.jobs)
    ]
    return {"include": include}


# --- 中身 ------------------------------------------------------------------


def _for_env(
    env: Environment,
    problems: Sequence[Problem],
    records: dict[str, list[dict]],
    fresh: dict[str, Freshness],
    keys: set[str],
    cases: Mapping[str, str],
) -> EnvPlan:
    models = _models(records, env.name)
    works: list[Work] = []
    unresolved: set[str] = set()
    compile_errors: set[str] = set()

    for problem in problems:
        rows = records[problem.id]
        # base の問題は束 (pj.batch) で測る。最新の束が今の全提出のキーを揃えて
        # いれば 0、欠けていれば全提出。CE と分かっている提出も束に入れる (束は
        # 全提出で作り、CE のコンパイルは安い)。除外は raw だけ。
        batched = problem.harness_kind == "base"
        known_ce = set() if batched else _compile_errors(rows, fresh[problem.id], env.name)
        compile_errors |= {f"{problem.id}/{s}" for s in known_ce}
        # 「いちばん新しい記録から」の規則は Store.cases_hashes が持つ。
        # run も同じものを使う。二度書くと片方を直し忘れる。
        cases_hash = cases.get(problem.id)
        candidates = [
            s.as_posix()
            for s in problem.submissions()
            if s.as_posix() not in known_ce
        ]

        todo: dict[str, tuple[str, ...]] = {}
        for model in models or ("",):
            compiler = _borrow(rows, "compiler_version", cpu_model=model, env=env.name)
            if not models or cases_hash is None or compiler is None:
                # 借りる先が無い = その条件の記録がまだ無い = 全部未計測。
                todo[model] = tuple(candidates)
                continue
            missing = []
            resolvable = []
            wanted = []
            for submission in candidates:
                key = fresh[problem.id].key_for(
                    submission,
                    cases_hash=cases_hash,
                    env=env.name,
                    compiler_version=compiler,
                    cpu_model=model,
                )
                if key is None:
                    # 閉包が欠けたままではキーが別の意味になる。解決できる回まで待つ。
                    unresolved.add(f"{problem.id}/{submission}")
                    continue
                resolvable.append(submission)
                wanted.append(key)
                if key not in keys:
                    missing.append(submission)
            if batched:
                newest = batch_mod.newest(rows, env=env.name, cpu_model=model)
                covered = newest is not None and newest.covers(wanted)
                todo[model] = () if covered else tuple(resolvable)
            else:
                todo[model] = tuple(missing)

        if any(todo.values()):
            works.append(
                Work(
                    problem=problem.id,
                    todo=todo,
                    weight=_weight(problem, rows, todo),
                )
            )

    # 重い順。同じ重さのときは id で並べて、全ジョブが同じ順を作れるようにする。
    works.sort(key=lambda w: (-w.weight, w.problem))
    return EnvPlan(
        env=env,
        models=models,
        works=tuple(works),
        unresolved=tuple(sorted(unresolved)),
        compile_errors=tuple(sorted(compile_errors)),
    )


def _models(records: dict[str, list[dict]], env_name: str) -> tuple[str, ...]:
    found = {
        row.get("cpu_model", "")
        for rows in records.values()
        for row in rows
        if row.get("env") == env_name and row.get("cpu_model")
    }
    return tuple(sorted(found))


def _borrow(
    rows: Sequence[dict], field_name: str, **match: str
) -> str | None:
    """条件に合ういちばん新しい記録からその項目を借りる。

    テストデータも コンパイラの版も、測った側にしか無い。だが知りたいのは
    「今のソースならこのキーになるはず」だけなので、機械側の値は記録から
    持ってくれば足りる。
    """
    best: tuple[str, str] | None = None
    for row in rows:
        if any(row.get(k) != v for k, v in match.items()):
            continue
        value = row.get(field_name)
        if not value:
            continue
        stamp = row.get("timestamp") or ""
        if best is None or stamp > best[0]:
            best = (stamp, value)
    return best[1] if best else None


def _compile_errors(
    rows: Sequence[dict], fresh: Freshness, env_name: str
) -> set[str]:
    """今のソースでこの環境が CE になると分かっている提出。

    コンパイルに CPU モデルは影響しないので、同じ環境の他のモデルでも必ず
    CE になる。立てても得るものが無い。
    """
    return {
        row.get("submission", "")
        for row in rows
        if row.get("env") == env_name
        and row.get("status") == "CE"
        and fresh.current(row) is True
    }


def _weight(
    problem: Problem, rows: Sequence[dict], todo: dict[str, tuple[str, ...]]
) -> float:
    """費用の見積もり (ms)。1 モデルあたりの期待値。

    問題を重い順に並べるためだけに使う。実測とずれても順番が入れ替わるだけで、
    測るものは変わらない。重いものが先に宣言されると、最後に残る 1 問が軽く
    なって全体の終わりが揃う。
    """
    measured: dict[str, int] = {}
    for row in rows:
        elapsed = row.get("time_total_ms")
        if elapsed is None:
            continue
        submission = row.get("submission", "")
        measured[submission] = max(measured.get(submission, 0), elapsed)
    unknown = _unmeasured_ms(problem, rows)
    total = sum(
        sum(measured.get(s, unknown) for s in submissions)
        for submissions in todo.values()
    )
    return total / len(todo)


def _unmeasured_ms(problem: Problem, rows: Sequence[dict]) -> float:
    """まだ一度も測っていない提出の費用。最悪 (全ケース TLE) で置く。"""
    counts = [row["case_count"] for row in rows if row.get("case_count")]
    cases = max(counts) if counts else ASSUMED_CASE_COUNT
    return cases * problem.limits.tle_sec * 1000
