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
