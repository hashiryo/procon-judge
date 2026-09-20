"""compile_only の縦 1 本と、スキップの判定。"""

import json
from pathlib import Path

import pytest

from pj import environment as env_mod
from pj import problem as problem_mod
from pj import run as run_mod

RAW_TOML = """
id = "tmp-raw"
title = "compile only"

[harness]
kind = "raw"

[testdata]
source = "none"

[compare]
kind = "compile_only"
"""


@pytest.fixture
def local_env():
    return env_mod.load("local")


@pytest.fixture
def machine(local_env):
    return run_mod.Machine.detect(local_env)


def make_problem(tmp_path, source):
    directory = tmp_path / "tmp-raw"
    directory.mkdir()
    (directory / "problem.toml").write_text(RAW_TOML)
    (directory / "submissions").mkdir()
    (directory / "submissions" / "sol.cpp").write_text(source)
    return problem_mod.load(directory)


def plan_for(problem, env, machine, known=frozenset()):
    return run_mod.build_plan(
        [(problem, s) for s in problem.submissions()], env, machine, set(known)
    )


def test_compiling_submission_is_ac(tmp_path, local_env, machine):
    problem = make_problem(tmp_path, "int main() { return 0; }\n")
    plan = plan_for(problem, local_env, machine)
    assert len(plan.jobs) == 1

    record = run_mod.execute_job(plan.jobs[0])
    assert record.status == "AC"
    assert record.case_count == 0
    assert record.binary_bytes and record.binary_bytes > 0
    assert record.failed_case is None
    assert record.cpu_model
    assert record.key == plan.jobs[0].key
    assert "-I" in record.cxxflags


def test_broken_submission_is_ce(tmp_path, local_env, machine):
    problem = make_problem(tmp_path, "int main() { return nope; }\n")
    record = run_mod.execute_job(plan_for(problem, local_env, machine).jobs[0])

    assert record.status == "CE"
    assert record.binary_bytes is None
    assert record.failed_case is not None
    assert record.failed_case.status == "CE"
    assert "nope" in record.failed_case.detail


def test_record_is_one_json_line(tmp_path, local_env, machine):
    problem = make_problem(tmp_path, "int main() { return 0; }\n")
    record = run_mod.execute_job(plan_for(problem, local_env, machine).jobs[0])

    line = record.to_json()
    assert "\n" not in line
    parsed = json.loads(line)
    assert parsed["problem"] == "tmp-raw"
    assert parsed["submission"] == "submissions/sol.cpp"
    assert parsed["env"] == "local"
    assert parsed["status"] == "AC"
    assert parsed["key"] == record.key


def test_a_known_key_is_skipped(tmp_path, local_env, machine):
    problem = make_problem(tmp_path, "int main() { return 0; }\n")
    plan = plan_for(problem, local_env, machine)
    again = plan_for(problem, local_env, machine, known={plan.jobs[0].key})

    assert again.jobs == ()
    assert len(again.skipped) == 1
    assert again.skipped[0].key == plan.jobs[0].key


def test_the_key_is_stable_across_plans(tmp_path, local_env, machine):
    problem = make_problem(tmp_path, "int main() { return 0; }\n")
    first = plan_for(problem, local_env, machine).jobs[0].key
    second = plan_for(problem, local_env, machine).jobs[0].key
    assert first == second


def test_editing_the_submission_moves_the_key(tmp_path, local_env, machine):
    problem = make_problem(tmp_path, "int main() { return 0; }\n")
    before = plan_for(problem, local_env, machine).jobs[0].key
    (problem.dir / "submissions" / "sol.cpp").write_text("int main() { return 1; }\n")
    assert plan_for(problem, local_env, machine).jobs[0].key != before


def test_reformatting_the_submission_keeps_the_key(tmp_path, local_env, machine):
    problem = make_problem(tmp_path, "int main() { return 0; }\n")
    before = plan_for(problem, local_env, machine).jobs[0].key
    (problem.dir / "submissions" / "sol.cpp").write_text(
        "int main() { return 0; }   \n\n\n"
    )
    assert plan_for(problem, local_env, machine).jobs[0].key == before


def test_plan_can_target_one_submission(tmp_path, local_env, machine):
    problem = make_problem(tmp_path, "int main() { return 0; }\n")
    (problem.dir / "submissions" / "other.cpp").write_text("int main() {}\n")
    plan = run_mod.build_plan(
        [(problem, Path("submissions/other.cpp"))], local_env, machine, set()
    )
    assert [j.submission.as_posix() for j in plan.jobs] == ["submissions/other.cpp"]


def test_identical_submissions_get_different_keys(tmp_path, local_env, machine):
    # 中身が同じでも別のファイルなら別の提出。提出ページはパスごとに描くので、
    # 片方が記録されないと困る。
    problem = make_problem(tmp_path, "int main() { return 0; }\n")
    (problem.dir / "submissions" / "copy.cpp").write_text("int main() { return 0; }\n")
    plan = plan_for(problem, local_env, machine)

    assert len(plan.jobs) == 2
    assert len({job.key for job in plan.jobs}) == 2
    # 中身のハッシュの方は同じ。こちらは中身だけを表す値なので変えない。
    assert len({job.submission_hash for job in plan.jobs}) == 1


def test_renaming_a_submission_moves_the_key(tmp_path, local_env, machine):
    problem = make_problem(tmp_path, "int main() { return 0; }\n")
    before = plan_for(problem, local_env, machine).jobs[0].key
    (problem.dir / "submissions" / "sol.cpp").rename(
        problem.dir / "submissions" / "renamed.cpp"
    )
    assert plan_for(problem, local_env, machine).jobs[0].key != before


UNCACHED_TOML = """
id = "tmp-uncached"
title = "not fetched yet"

[harness]
kind = "raw"

[testdata]
source = "library_checker"
name = "nowhere/nothing"

[compare]
kind = "tokens"
"""


def test_uncached_testdata_defers_instead_of_fetching(tmp_path, local_env, machine):
    # cases_hash が無いとキーを計算できない。--dry-run では取りに行かず、
    # 未取得として返す。
    directory = tmp_path / "tmp-uncached"
    directory.mkdir()
    (directory / "problem.toml").write_text(UNCACHED_TOML)
    (directory / "submissions").mkdir()
    (directory / "submissions" / "sol.cpp").write_text("int main() {}\n")
    problem = problem_mod.load(directory)

    plan = run_mod.build_plan(
        [(problem, s) for s in problem.submissions()],
        local_env, machine, set(), allow_fetch=False,
    )
    assert plan.jobs == ()
    assert plan.skipped == ()
    assert len(plan.pending) == 1


YUKI_TOML = """
id = "tmp-yuki"
title = "取得に失敗する問題"

[harness]
kind = "raw"

[testdata]
source = "yukicoder"
name = "1"

[compare]
kind = "tokens"
"""


def test_a_problem_that_cannot_be_fetched_does_not_stop_the_others(
    tmp_path, local_env, machine, monkeypatch
):
    """1 問取れないだけで、走れる問題の記録まで落とさない。"""
    monkeypatch.delenv("YUKICODER_TOKEN", raising=False)
    monkeypatch.delenv("TESTDATA_TOKEN", raising=False)

    ok = make_problem(tmp_path, "int main() { return 0; }\n")
    directory = tmp_path / "tmp-yuki"
    directory.mkdir()
    (directory / "problem.toml").write_text(YUKI_TOML)
    (directory / "submissions").mkdir()
    (directory / "submissions" / "sol.cpp").write_text("int main() {}\n")
    bad = problem_mod.load(directory)

    targets = [(ok, s) for s in ok.submissions()]
    targets += [(bad, s) for s in bad.submissions()]
    plan = run_mod.build_plan(targets, local_env, machine, set())

    assert [job.problem.id for job in plan.jobs] == [ok.id]
    assert [pid for pid, _ in plan.failed] == [bad.id]
    assert [problem.id for problem, _ in plan.pending] == [bad.id]


MISSING_INCLUDE = '#include "nowhere/missing.hpp"\nint main() { return 0; }\n'


def test_unresolvable_include_blocks_the_submission(tmp_path, local_env, machine):
    """閉包が欠けたままキーを作ると意味が変わる。走らせずに次の回へ回す。"""
    problem = make_problem(tmp_path, MISSING_INCLUDE)
    plan = plan_for(problem, local_env, machine)
    assert plan.jobs == ()
    assert [s.as_posix() for _, s in plan.blocked] == ["submissions/sol.cpp"]
    assert plan.unresolved == (("submissions/sol.cpp", "nowhere/missing.hpp"),)


def test_a_blocked_submission_does_not_stop_the_others(tmp_path, local_env, machine):
    problem = make_problem(tmp_path, "int main() { return 0; }\n")
    (problem.dir / "submissions" / "broken.cpp").write_text(MISSING_INCLUDE)
    problem = problem_mod.load(problem.dir)
    plan = plan_for(problem, local_env, machine)
    assert [j.submission.as_posix() for j in plan.jobs] == ["submissions/sol.cpp"]
    assert [s.as_posix() for _, s in plan.blocked] == ["submissions/broken.cpp"]


# --- budget ----------------------------------------------------------------


def make_problem_with(tmp_path, name, submissions):
    directory = tmp_path / name
    directory.mkdir()
    (directory / "problem.toml").write_text(RAW_TOML.replace("tmp-raw", name))
    (directory / "submissions").mkdir()
    for number, stem in enumerate(submissions):
        (directory / "submissions" / f"{stem}.cpp").write_text(
            f"int main() {{ return {number}; }}\n"
        )
    return problem_mod.load(directory)


def test_budget_stops_partway_and_leaves_the_rest(tmp_path, local_env, machine):
    """6 時間で打ち切られると、その回に測ったぶんを丸ごと落とす。"""
    problem = make_problem_with(tmp_path, "tmp-raw", ("a", "b", "c", "d"))
    targets = [(problem, s) for s in problem.submissions()]
    plan = run_mod.build_plan(targets, local_env, machine, set(), budget=2)
    assert [j.submission.as_posix() for j in plan.jobs] == [
        "submissions/a.cpp",
        "submissions/b.cpp",
    ]
    assert [s.as_posix() for _, s in plan.held] == [
        "submissions/c.cpp",
        "submissions/d.cpp",
    ]


def test_budget_stops_inside_a_bundle(tmp_path, local_env, machine):
    """大きい束に当たったときこそ効いてほしい。束の切れ目は待たない。"""
    first = make_problem_with(tmp_path, "tmp-raw", ("a", "b", "c"))
    second = make_problem_with(tmp_path, "tmp-two", ("a",))
    targets = [(first, s) for s in first.submissions()]
    targets += [(second, s) for s in second.submissions()]
    plan = run_mod.build_plan(targets, local_env, machine, set(), budget=2)
    assert len(plan.jobs) == 2
    assert {p.id for p, _ in plan.held} == {"tmp-raw", "tmp-two"}


def test_skipped_submissions_do_not_eat_the_budget(tmp_path, local_env, machine):
    """既に記録のあるものは走らせないので、budget を減らさない。"""
    problem = make_problem_with(tmp_path, "tmp-raw", ("a", "b", "c"))
    targets = [(problem, s) for s in problem.submissions()]
    known = {run_mod.build_plan(targets, local_env, machine, set()).jobs[0].key}
    plan = run_mod.build_plan(targets, local_env, machine, known, budget=2)
    assert [j.submission.as_posix() for j in plan.jobs] == [
        "submissions/b.cpp",
        "submissions/c.cpp",
    ]
    assert plan.held == ()


def test_without_a_budget_everything_is_planned(tmp_path, local_env, machine):
    problem = make_problem_with(tmp_path, "tmp-raw", ("a", "b", "c"))
    targets = [(problem, s) for s in problem.submissions()]
    plan = run_mod.build_plan(targets, local_env, machine, set())
    assert len(plan.jobs) == 3
    assert plan.held == ()


def test_the_order_of_the_targets_is_kept(tmp_path, local_env, machine):
    """束の順は plan が重い順に決める。ここで並べ直すと budget の意味が変わる。"""
    first = make_problem_with(tmp_path, "tmp-zzz", ("a",))
    second = make_problem_with(tmp_path, "tmp-aaa", ("a",))
    targets = [(first, s) for s in first.submissions()]
    targets += [(second, s) for s in second.submissions()]
    plan = run_mod.build_plan(targets, local_env, machine, set())
    assert [j.problem.id for j in plan.jobs] == ["tmp-zzz", "tmp-aaa"]
