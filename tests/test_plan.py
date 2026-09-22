"""ジョブが始まる前の計画。本数の決め方と、束を配る順番。

plan が数え間違えると、仕事があるのにジョブが立たない (記録が増えない) か、
仕事が無いのに立つ (空回りする) かのどちらかになる。どちらも静かに起きる。
"""

from __future__ import annotations

import json

import pytest

from pj import build as build_mod
from pj import environment as env_mod
from pj import key as key_mod
from pj import plan as plan_mod
from pj import problem as problem_mod
from pj.store import Store

PROBLEM_TOML = """
id = "{id}"
title = "t"

[harness]
kind = "raw"

[testdata]
source = "none"

[compare]
kind = "compile_only"
"""

SOURCE = "int main() {{ return {n}; }}\n"

CASES_HASH = "h"
COMPILER = "g++-15"


@pytest.fixture
def envs():
    return env_mod.load_all()


@pytest.fixture
def ci_envs(envs):
    return [e for e in envs if e.runs_on != "self"]


def make_problem(tmp_path, problem_id, submissions=("a", "b")):
    directory = tmp_path / problem_id
    directory.mkdir()
    (directory / "problem.toml").write_text(PROBLEM_TOML.format(id=problem_id))
    (directory / "submissions").mkdir()
    for number, name in enumerate(submissions):
        (directory / "submissions" / f"{name}.cpp").write_text(SOURCE.format(n=number))
    return problem_mod.load(directory)


def record_for(problem, env, submission, *, cpu_model="EPYC", **over):
    """今のソースをその条件で測ったことにした記録。"""
    search = build_mod.include_dirs(problem)
    cxxflags = build_mod.effective_cxxflags(env, problem)
    sub = key_mod.submission_hash(problem.dir / submission, search)
    compiler = over.pop("compiler_version", COMPILER)
    cases_hash = over.pop("cases_hash", CASES_HASH)
    key = key_mod.compute(
        submission=submission,
        submission_hash=sub.submission_hash,
        harness_hash=key_mod.harness_hash(problem, search),
        problem_hash=key_mod.problem_hash(problem),
        cases_hash=cases_hash,
        env=env.name,
        compiler_version=compiler,
        cxxflags=cxxflags,
        cpu_model=cpu_model,
    )
    record = {
        "key": key,
        "problem": problem.id,
        "submission": submission,
        "status": "AC",
        "env": env.name,
        "cpu_arch": "x86_64",
        "cpu_model": cpu_model,
        "compiler_version": compiler,
        "cxxflags": cxxflags,
        "cases_hash": cases_hash,
        "case_count": 3,
        "submission_hash": sub.submission_hash,
        "includes": list(sub.includes),
        "library_sha": None,
        "judge_sha": None,
        "time_max_ms": 100,
        "time_total_ms": 200,
        "algo_time_max_ns": None,
        "algo_time_total_ns": None,
        "memory_max_kb": 1024,
        "source_bytes": 10,
        "binary_bytes": 20,
        "failed_case": None,
        "timestamp": "2026-01-01T00:00:00Z",
    }
    record.update(over)
    return record


def store_with(tmp_path, records):
    store = Store(tmp_path / "results")
    for record in records:
        store.append_raw(record["problem"], json.dumps(record, ensure_ascii=False))
    return store


def plan_for(env, problems, envs, store, **kw):
    return plan_mod.for_env(env, problems, envs, store, **kw)


# --- 何を未計測と数えるか ---------------------------------------------------


def test_a_model_that_has_every_submission_gives_no_work(tmp_path, envs, ci_envs):
    env = ci_envs[0]
    problem = make_problem(tmp_path, "p1")
    records = [
        record_for(problem, env, s.as_posix()) for s in problem.submissions()
    ]
    one = plan_for(env, [problem], envs, store_with(tmp_path, records))
    assert one.models == ("EPYC",)
    assert one.bundles == ()
    assert one.jobs == 0


def test_a_submission_missing_on_one_model_is_work(tmp_path, envs, ci_envs):
    """モデルが 2 つあって片方に穴が空いていれば、その穴を埋めに行く。"""
    env = ci_envs[0]
    problem = make_problem(tmp_path, "p1")
    submissions = [s.as_posix() for s in problem.submissions()]
    records = [record_for(problem, env, s, cpu_model="EPYC") for s in submissions]
    records.append(record_for(problem, env, submissions[0], cpu_model="Xeon"))
    one = plan_for(env, [problem], envs, store_with(tmp_path, records))
    assert one.models == ("EPYC", "Xeon")
    assert [b.problem for b in one.bundles] == ["p1"]
    assert one.bundles[0].todo == {"EPYC": (), "Xeon": (submissions[1],)}
    assert one.jobs == 1


def test_a_problem_without_any_record_is_all_work(tmp_path, envs, ci_envs):
    """cases_hash を借りる先が無い。借りられないものは未計測。"""
    env = ci_envs[0]
    measured = make_problem(tmp_path, "p1")
    fresh = make_problem(tmp_path, "p2")
    records = [
        record_for(measured, env, s.as_posix()) for s in measured.submissions()
    ]
    one = plan_for(env, [measured, fresh], envs, store_with(tmp_path, records))
    assert [b.problem for b in one.bundles] == ["p2"]
    assert one.bundles[0].todo["EPYC"] == ("submissions/a.cpp", "submissions/b.cpp")


def test_an_environment_without_records_plans_everything(tmp_path, envs, ci_envs):
    """モデルを 1 つも知らない環境。まっさらから始めてもジョブが立つ。"""
    known, unknown = ci_envs[0], ci_envs[1]
    problem = make_problem(tmp_path, "p1")
    records = [
        record_for(problem, known, s.as_posix()) for s in problem.submissions()
    ]
    one = plan_for(unknown, [problem], envs, store_with(tmp_path, records))
    assert one.models == ()
    assert one.bundles[0].todo == {"": ("submissions/a.cpp", "submissions/b.cpp")}
    assert one.jobs == 1


def test_editing_a_submission_makes_it_work_again(tmp_path, envs, ci_envs):
    env = ci_envs[0]
    problem = make_problem(tmp_path, "p1")
    records = [
        record_for(problem, env, s.as_posix()) for s in problem.submissions()
    ]
    store = store_with(tmp_path, records)
    assert plan_for(env, [problem], envs, store).jobs == 0
    (problem.dir / "submissions" / "a.cpp").write_text("int main() { return 7; }\n")
    one = plan_for(env, [problem], envs, store)
    assert one.bundles[0].todo["EPYC"] == ("submissions/a.cpp",)


def test_the_submissions_of_one_problem_stay_in_one_bundle(tmp_path, envs, ci_envs):
    """束は問題の単位。同じマシンに載れば順位表の 1 行がその回で埋まる。"""
    env = ci_envs[0]
    problem = make_problem(tmp_path, "p1", submissions=("a", "b", "c"))
    one = plan_for(env, [problem], envs, store_with(tmp_path, []))
    assert len(one.bundles) == 1
    assert len(one.bundles[0].todo[""]) == 3


# --- 落とせるもの -----------------------------------------------------------


def test_a_known_compile_error_is_not_planned_for_other_models(
    tmp_path, envs, ci_envs
):
    """コンパイルに CPU モデルは影響しない。他のモデルでも必ず CE になる。"""
    env = ci_envs[0]
    problem = make_problem(tmp_path, "p1")
    submissions = [s.as_posix() for s in problem.submissions()]
    records = [
        record_for(problem, env, submissions[0], cpu_model="EPYC", status="CE"),
        record_for(problem, env, submissions[1], cpu_model="EPYC"),
        record_for(problem, env, submissions[1], cpu_model="Xeon"),
    ]
    one = plan_for(env, [problem], envs, store_with(tmp_path, records))
    assert one.compile_errors == (f"p1/{submissions[0]}",)
    assert one.bundles == ()
    assert one.jobs == 0


def test_a_compile_error_in_one_environment_does_not_suppress_another(
    tmp_path, envs, ci_envs
):
    """CE になるかは環境で変わる。片方で落ちても、もう片方は立てる。"""
    a, b = ci_envs[0], ci_envs[1]
    problem = make_problem(tmp_path, "p1", submissions=("a",))
    records = [record_for(problem, a, "submissions/a.cpp", status="CE")]
    store = store_with(tmp_path, records)
    assert plan_for(a, [problem], envs, store).compile_errors == (
        "p1/submissions/a.cpp",
    )
    assert plan_for(b, [problem], envs, store).compile_errors == ()


def test_a_stale_compile_error_does_not_suppress_anything(tmp_path, envs, ci_envs):
    """ソースを書き換えたあとの CE は、今のソースの話ではない。"""
    env = ci_envs[0]
    problem = make_problem(tmp_path, "p1", submissions=("a",))
    records = [record_for(problem, env, "submissions/a.cpp", status="CE")]
    store = store_with(tmp_path, records)
    assert plan_for(env, [problem], envs, store).compile_errors == (
        "p1/submissions/a.cpp",
    )
    (problem.dir / "submissions" / "a.cpp").write_text("int main() { return 7; }\n")
    one = plan_for(env, [problem], envs, store)
    assert one.compile_errors == ()
    assert one.bundles[0].todo["EPYC"] == ("submissions/a.cpp",)


def test_an_unresolved_include_is_reported_and_not_planned(tmp_path, envs, ci_envs):
    """閉包が欠けたままではキーが別の意味になる。解決できる回まで待つ。"""
    env = ci_envs[0]
    problem = make_problem(tmp_path, "p1", submissions=("a",))
    records = [record_for(problem, env, "submissions/a.cpp")]
    store = store_with(tmp_path, records)
    (problem.dir / "submissions" / "a.cpp").write_text(
        '#include "mylib/nowhere.hpp"\nint main() { return 0; }\n'
    )
    one = plan_for(env, [problem], envs, store)
    assert one.unresolved == ("p1/submissions/a.cpp",)
    assert one.bundles == ()
    assert one.jobs == 0


# --- ジョブの本数 -----------------------------------------------------------


def test_the_job_count_follows_the_budget(tmp_path, envs, ci_envs):
    env = ci_envs[0]
    problem = make_problem(tmp_path, "p1", submissions=tuple("abcdefg"))
    store = store_with(tmp_path, [])
    assert plan_for(env, [problem], envs, store, budget=7).jobs == 1
    assert plan_for(env, [problem], envs, store, budget=4).jobs == 2
    assert plan_for(env, [problem], envs, store, budget=3).jobs == 3


def test_the_job_count_stops_at_the_concurrency_limit(tmp_path, envs, ci_envs):
    """分割を細かくしても並列度は上がらない。"""
    env = ci_envs[0]
    problem = make_problem(tmp_path, "p1", submissions=tuple("abcdefghijkl"))
    one = plan_for(env, [problem], envs, store_with(tmp_path, []), budget=1)
    assert one.jobs == plan_mod.MAX_JOBS_PER_ENV


def test_the_job_count_counts_every_model(tmp_path, envs, ci_envs):
    """仕事はモデルごとにある。1 モデルあたりの平均で数えると、モデルが多い環境で
    本数が足りなくなる。"""
    env = ci_envs[0]
    problem = make_problem(tmp_path, "p1", submissions=tuple("abcdef"))
    records = [
        record_for(problem, env, "submissions/a.cpp", cpu_model="EPYC"),
        record_for(problem, env, "submissions/a.cpp", cpu_model="Xeon"),
    ]
    one = plan_for(env, [problem], envs, store_with(tmp_path, records), budget=5)
    assert one.models == ("EPYC", "Xeon") or set(one.models) == {"EPYC", "Xeon"}
    assert one.bundles[0].expected == 5
    assert one.bundles[0].total == 10
    assert one.jobs == 2


# --- 束を配る順番 -----------------------------------------------------------


def test_each_job_starts_at_its_own_rank_and_strides():
    order = ("p0", "p1", "p2", "p3", "p4")
    assert plan_mod.assignment(order, 0, 2)[:3] == ["p0", "p2", "p4"]
    assert plan_mod.assignment(order, 1, 2)[:2] == ["p1", "p3"]


def test_every_job_is_handed_all_the_bundles():
    """自分の束が全部計測済みで暇になったら、この順で次へ踏み込む。"""
    order = ("p0", "p1", "p2", "p3", "p4")
    for job in range(2):
        handed = plan_mod.assignment(order, job, 2)
        assert sorted(handed) == sorted(order)
        assert len(handed) == len(set(handed))


def test_a_single_job_gets_the_heavy_order_as_is():
    order = ("p0", "p1", "p2")
    assert plan_mod.assignment(order, 0, 1) == list(order)


def test_the_job_number_has_to_fit_in_the_count():
    with pytest.raises(ValueError):
        plan_mod.assignment(("p0",), 2, 2)
    with pytest.raises(ValueError):
        plan_mod.assignment(("p0",), 0, 0)


def test_heavier_bundles_come_first(tmp_path, envs, ci_envs):
    """費用の見積もりは、記録のある提出は time_total_ms、無い提出は最悪で置く。"""
    env = ci_envs[0]
    light = make_problem(tmp_path, "p-light", submissions=("a",))
    heavy = make_problem(tmp_path, "p-heavy", submissions=("a",))
    records = [
        record_for(light, env, "submissions/a.cpp", time_total_ms=10),
        record_for(heavy, env, "submissions/a.cpp", time_total_ms=9000),
    ]
    store = store_with(tmp_path, records)
    for problem in (light, heavy):
        (problem.dir / "submissions" / "a.cpp").write_text(
            "int main() { return 7; }\n"
        )
    assert plan_for(env, [light, heavy], envs, store).order == ("p-heavy", "p-light")


# --- マトリクス -------------------------------------------------------------


def test_the_matrix_carries_what_the_runner_needs(tmp_path, envs, ci_envs):
    problem = make_problem(tmp_path, "p1", submissions=("a",))
    plans = plan_mod.build([problem], envs, store_with(tmp_path, []), budget=1)
    entries = plan_mod.matrix(plans)["include"]
    assert {e["env"] for e in entries} == {e.name for e in ci_envs}
    runs_on = {e.name: e.runs_on for e in ci_envs}
    for entry in entries:
        assert entry["runs_on"] == runs_on[entry["env"]]
        assert entry["toolchain"] in ("gcc", "clang")
        assert 0 <= entry["job"] < entry["jobs"]


def test_the_matrix_carries_the_time_limit(tmp_path, envs, ci_envs):
    """run は件数の上限と時間の上限の両方で止める。時間の値も plan が決めて matrix で渡す。"""
    problem = make_problem(tmp_path, "p1", submissions=("a",))
    plans = plan_mod.build([problem], envs, store_with(tmp_path, []), budget=1, minutes=30)
    for entry in plan_mod.matrix(plans)["include"]:
        assert entry["minutes"] == 30
    default = plan_mod.build([problem], envs, store_with(tmp_path, []), budget=1)
    assert {e["minutes"] for e in plan_mod.matrix(default)["include"]} == {plan_mod.DEFAULT_MINUTES}


def test_the_matrix_is_empty_when_there_is_nothing_to_do(tmp_path, envs, ci_envs):
    """やることが 0 件の push で run が 1 本も立たない。"""
    problem = make_problem(tmp_path, "p1", submissions=("a",))
    records = [
        record_for(problem, env, "submissions/a.cpp") for env in ci_envs
    ]
    plans = plan_mod.build([problem], envs, store_with(tmp_path, records))
    assert plan_mod.matrix(plans) == {"include": []}


def test_the_local_environment_never_enters_the_matrix(tmp_path, envs):
    problem = make_problem(tmp_path, "p1", submissions=("a",))
    plans = plan_mod.build([problem], envs, store_with(tmp_path, []))
    assert "local" not in {p.env.name for p in plans}
