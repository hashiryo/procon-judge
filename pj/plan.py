"""ジョブが始まる前に決められることを決める。

CPU モデルはマシンが割り当たった瞬間に決まるので、ジョブが始まる前には
分からない。だからモデルに依存しない判断だけをここで済ませて、依存する判断は
run の中に残す。ここが決めるのは、環境ごとに何本のジョブを立てるかと、
問題をどの順で片付けるかの 2 つ。

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

from .environment import Environment, toolchain
from .freshness import Freshness
from .problem import Problem
from .store import Store

# 同時実行は Free で 20 本。分割を細かくしても並列度は上がらないので、
# 4 環境で割り切れるところに置く。
MAX_JOBS_PER_ENV = 5

# 1 ジョブで測る提出の上限。6 時間で打ち切られると、その回に測ったぶんを
# 丸ごと落とす。必ず終わって記録を上げるところまで行かせるための値。
# raw 移行の CI で 40 件が 60 分前後だった (保管庫からの取得と mylib のコンパイルが
# 大半)。100 件なら 2.5 時間ほどで、上限の半分に収まる。
DEFAULT_BUDGET = 100

# 記録がまだ 1 件も無い問題のケース数の仮置き。費用は束の重い順を決める
# ためだけに使うので、外れても順番が少し入れ替わるだけで済む。
ASSUMED_CASE_COUNT = 20


@dataclass(frozen=True)
class Bundle:
    """1 問題ぶんの仕事。同じ問題の提出は必ず同じ束に入る。

    同じマシンに載れば順位表の 1 行がその回で埋まるし、テストデータの取得も
    その束につき 1 回で済む。
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


@dataclass(frozen=True)
class EnvPlan:
    env: Environment
    models: tuple[str, ...]
    # 重い順。run はこの並びから自分の担当を取る。
    bundles: tuple[Bundle, ...]
    jobs: int
    # run に渡す上限。plan が本数を決めるときに置いた前提なので、run も同じ値を使う。
    budget: int = DEFAULT_BUDGET
    # include を解決できなかった提出。ライブラリを取れていないと全部並ぶ。
    unresolved: tuple[str, ...] = ()
    # 今のソースで CE になると分かっている提出。他のモデルでも必ず CE になる。
    compile_errors: tuple[str, ...] = ()

    @property
    def order(self) -> tuple[str, ...]:
        return tuple(b.problem for b in self.bundles)

    @property
    def expected(self) -> float:
        return sum(b.expected for b in self.bundles)


def build(
    problems: Sequence[Problem],
    envs: Sequence[Environment],
    store: Store,
    *,
    budget: int = DEFAULT_BUDGET,
) -> list[EnvPlan]:
    """CI で走らせる環境それぞれの計画。"""
    keys = store.keys()
    records = {p.id: list(store.read(p.id)) for p in problems}
    # Freshness は提出のハッシュを問題につき一度だけ作る。環境をまたいで
    # 使い回さないと、閉包を環境の数だけ辿り直すことになる。
    fresh = {p.id: Freshness(p, envs) for p in problems}
    cases = store.cases_hashes()
    return [
        _for_env(env, problems, records, fresh, keys, budget, cases)
        for env in envs
        if env.runs_on != "self"
    ]


def for_env(
    env: Environment,
    problems: Sequence[Problem],
    envs: Sequence[Environment],
    store: Store,
    *,
    budget: int = DEFAULT_BUDGET,
) -> EnvPlan:
    """1 環境ぶん。run が自分の担当を組み直すのに使う。

    run は plan と同じ並びを作れないといけない。並びは記録と問題定義と lib/
    だけから決まるので、同じものを読めば同じ順になる。
    """
    keys = store.keys()
    records = {p.id: list(store.read(p.id)) for p in problems}
    fresh = {p.id: Freshness(p, envs) for p in problems}
    return _for_env(env, problems, records, fresh, keys, budget, store.cases_hashes())


def assignment(order: Sequence[str], job: int, jobs: int) -> list[str]:
    """j 番目のジョブが束を片付ける順番。

    j 番目に重い束から始めて、そこから本数ぶん飛ばして進む。先頭が重い順の
    上位に揃い、溢れた先が他のジョブの先頭を避ける。

    自分の担当のうしろに残り全部を付ける。plan はモデルを知らないので、
    自分の束がそのモデルでは全部計測済みで暇になることがある。そのときは
    この順で次の束へ踏み込む。別のジョブと同じキーを測ることがあるが、
    同じキーの 2 本目の記録は標本の蓄積になるので無駄ではない。
    """
    if jobs < 1:
        raise ValueError(f"ジョブの本数が {jobs} です")
    if not 0 <= job < jobs:
        raise ValueError(f"ジョブ番号 {job} が本数 {jobs} に収まりません")
    mine = list(order[job::jobs])
    taken = set(mine)
    return mine + [p for p in order if p not in taken]


def matrix(plans: Iterable[EnvPlan]) -> dict:
    """judge.yml が fromJSON で受ける形。

    束は入れない。ジョブ名に全部並ぶと読めなくなるし、問題が増えると
    マトリクスが膨らむ。run は order を自分で組み直せる。
    """
    include = [
        {
            "env": plan.env.name,
            "runs_on": plan.env.runs_on,
            "toolchain": toolchain(plan.env),
            "job": job,
            "jobs": plan.jobs,
            "budget": plan.budget,
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
    budget: int,
    cases: Mapping[str, str],
) -> EnvPlan:
    models = _models(records, env.name)
    bundles: list[Bundle] = []
    unresolved: set[str] = set()
    compile_errors: set[str] = set()

    for problem in problems:
        rows = records[problem.id]
        known_ce = _compile_errors(rows, fresh[problem.id], env.name)
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
                if key not in keys:
                    missing.append(submission)
            todo[model] = tuple(missing)

        if any(todo.values()):
            bundles.append(
                Bundle(
                    problem=problem.id,
                    todo=todo,
                    weight=_weight(problem, rows, todo),
                )
            )

    # 重い順。同じ重さのときは id で並べて、plan と run が同じ順を作れるようにする。
    bundles.sort(key=lambda b: (-b.weight, b.problem))
    return EnvPlan(
        env=env,
        models=models,
        bundles=tuple(bundles),
        jobs=_jobs(bundles, budget),
        budget=budget,
        unresolved=tuple(sorted(unresolved)),
        compile_errors=tuple(sorted(compile_errors)),
    )


def _jobs(bundles: Sequence[Bundle], budget: int) -> int:
    """立てるジョブの本数。

    既知のモデルすべてで 0 件ならジョブを立てない。未知のモデルに当たれば
    仕事はあるが、それは計画された仕事ではない。新しい CPU モデルを探しには
    行かない。
    """
    expected = sum(b.expected for b in bundles)
    if expected <= 0:
        return 0
    return min(MAX_JOBS_PER_ENV, max(1, math.ceil(expected / budget)))


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

    束の重い順を決めるためだけに使う。実測とずれても順番が入れ替わるだけで、
    測るものは変わらない。
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
