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


# 自己検証の提出。空の入力で走らせて終了コードだけを見る。
EXIT_TOML = RAW_TOML.replace('kind = "compile_only"', 'kind = "exit_code"').replace(
    'title = "compile only"', 'title = "exit code"'
)


def make_problem(tmp_path, source, toml=RAW_TOML):
    directory = tmp_path / "tmp-raw"
    directory.mkdir()
    (directory / "problem.toml").write_text(toml)
    (directory / "submissions").mkdir()
    (directory / "submissions" / "sol.cpp").write_text(source)
    return problem_mod.load(directory)


def worklist_for(problem, env, machine, known=frozenset()):
    return run_mod.build_worklist(
        [(problem, s) for s in problem.submissions()], env, machine, set(known)
    )


def test_compiling_submission_is_ac(tmp_path, local_env, machine):
    problem = make_problem(tmp_path, "int main() { return 0; }\n")
    worklist = worklist_for(problem, local_env, machine)
    assert len(worklist.jobs) == 1

    record = run_mod.execute_job(worklist.jobs[0])
    assert record.status == "AC"
    assert record.case_count == 0
    assert record.binary_bytes and record.binary_bytes > 0
    assert record.failed_case is None
    assert record.cpu_model
    assert record.key == worklist.jobs[0].key
    assert "-I" in record.cxxflags


def test_exit_code_runs_once_and_is_ac_on_zero(tmp_path, local_env, machine):
    problem = make_problem(tmp_path, "int main() { return 0; }\n", toml=EXIT_TOML)
    record = run_mod.execute_job(worklist_for(problem, local_env, machine).jobs[0])
    assert record.status == "AC"
    assert record.case_count == 1
    assert record.algo_time_max_ns is None
    assert record.failed_cases == []
    assert record.failed_case is None
    assert record.binary_bytes and record.binary_bytes > 0


def test_exit_code_nonzero_is_re_with_stderr(tmp_path, local_env, machine):
    source = '#include <cstdio>\nint main() { std::fputs("boom\\n", stderr); return 3; }\n'
    problem = make_problem(tmp_path, source, toml=EXIT_TOML)
    record = run_mod.execute_job(worklist_for(problem, local_env, machine).jobs[0])
    assert record.status == "RE"
    assert record.case_count == 1
    assert record.failed_cases == [run_mod.SELF_CHECK_CASE]
    assert record.failed_case is not None
    assert "exit 3" in record.failed_case.detail
    assert "boom" in record.failed_case.detail


def test_broken_submission_is_ce(tmp_path, local_env, machine):
    problem = make_problem(tmp_path, "int main() { return nope; }\n")
    record = run_mod.execute_job(worklist_for(problem, local_env, machine).jobs[0])

    assert record.status == "CE"
    assert record.binary_bytes is None
    assert record.failed_case is not None
    assert record.failed_case.status == "CE"
    assert "nope" in record.failed_case.detail


def test_record_is_one_json_line(tmp_path, local_env, machine):
    problem = make_problem(tmp_path, "int main() { return 0; }\n")
    record = run_mod.execute_job(worklist_for(problem, local_env, machine).jobs[0])

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
    worklist = worklist_for(problem, local_env, machine)
    again = worklist_for(problem, local_env, machine, known={worklist.jobs[0].key})

    assert again.jobs == ()
    assert len(again.skipped) == 1
    assert again.skipped[0].key == worklist.jobs[0].key


def test_the_key_is_stable_across_plans(tmp_path, local_env, machine):
    problem = make_problem(tmp_path, "int main() { return 0; }\n")
    first = worklist_for(problem, local_env, machine).jobs[0].key
    second = worklist_for(problem, local_env, machine).jobs[0].key
    assert first == second


def test_editing_the_submission_moves_the_key(tmp_path, local_env, machine):
    problem = make_problem(tmp_path, "int main() { return 0; }\n")
    before = worklist_for(problem, local_env, machine).jobs[0].key
    (problem.dir / "submissions" / "sol.cpp").write_text("int main() { return 1; }\n")
    assert worklist_for(problem, local_env, machine).jobs[0].key != before


def test_reformatting_the_submission_keeps_the_key(tmp_path, local_env, machine):
    problem = make_problem(tmp_path, "int main() { return 0; }\n")
    before = worklist_for(problem, local_env, machine).jobs[0].key
    (problem.dir / "submissions" / "sol.cpp").write_text(
        "int main() { return 0; }   \n\n\n"
    )
    assert worklist_for(problem, local_env, machine).jobs[0].key == before


def test_plan_can_target_one_submission(tmp_path, local_env, machine):
    problem = make_problem(tmp_path, "int main() { return 0; }\n")
    (problem.dir / "submissions" / "other.cpp").write_text("int main() {}\n")
    worklist = run_mod.build_worklist(
        [(problem, Path("submissions/other.cpp"))], local_env, machine, set()
    )
    assert [j.submission.as_posix() for j in worklist.jobs] == ["submissions/other.cpp"]


def test_identical_submissions_get_different_keys(tmp_path, local_env, machine):
    # 中身が同じでも別のファイルなら別の提出。提出ページはパスごとに描くので、
    # 片方が記録されないと困る。
    problem = make_problem(tmp_path, "int main() { return 0; }\n")
    (problem.dir / "submissions" / "copy.cpp").write_text("int main() { return 0; }\n")
    worklist = worklist_for(problem, local_env, machine)

    assert len(worklist.jobs) == 2
    assert len({job.key for job in worklist.jobs}) == 2
    # 中身のハッシュの方は同じ。こちらは中身だけを表す値なので変えない。
    assert len({job.submission_hash for job in worklist.jobs}) == 1


def test_renaming_a_submission_moves_the_key(tmp_path, local_env, machine):
    problem = make_problem(tmp_path, "int main() { return 0; }\n")
    before = worklist_for(problem, local_env, machine).jobs[0].key
    (problem.dir / "submissions" / "sol.cpp").rename(
        problem.dir / "submissions" / "renamed.cpp"
    )
    assert worklist_for(problem, local_env, machine).jobs[0].key != before


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

    worklist = run_mod.build_worklist(
        [(problem, s) for s in problem.submissions()],
        local_env, machine, set(), allow_fetch=False,
    )
    assert worklist.jobs == ()
    assert worklist.skipped == ()
    assert len(worklist.pending) == 1


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
    worklist = run_mod.build_worklist(targets, local_env, machine, set())

    assert [job.problem.id for job in worklist.jobs] == [ok.id]
    assert [pid for pid, _ in worklist.failed] == [bad.id]
    assert [problem.id for problem, _ in worklist.pending] == [bad.id]


MISSING_INCLUDE = '#include "nowhere/missing.hpp"\nint main() { return 0; }\n'


def test_unresolvable_include_blocks_the_submission(tmp_path, local_env, machine):
    """閉包が欠けたままキーを作ると意味が変わる。走らせずに次の回へ回す。"""
    problem = make_problem(tmp_path, MISSING_INCLUDE)
    worklist = worklist_for(problem, local_env, machine)
    assert worklist.jobs == ()
    assert [s.as_posix() for _, s in worklist.blocked] == ["submissions/sol.cpp"]
    assert worklist.unresolved == (("submissions/sol.cpp", "nowhere/missing.hpp"),)


def test_a_blocked_submission_does_not_stop_the_others(tmp_path, local_env, machine):
    problem = make_problem(tmp_path, "int main() { return 0; }\n")
    (problem.dir / "submissions" / "broken.cpp").write_text(MISSING_INCLUDE)
    problem = problem_mod.load(problem.dir)
    worklist = worklist_for(problem, local_env, machine)
    assert [j.submission.as_posix() for j in worklist.jobs] == ["submissions/sol.cpp"]
    assert [s.as_posix() for _, s in worklist.blocked] == ["submissions/broken.cpp"]


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
    worklist = run_mod.build_worklist(targets, local_env, machine, set(), budget=2)
    assert [j.submission.as_posix() for j in worklist.jobs] == [
        "submissions/a.cpp",
        "submissions/b.cpp",
    ]
    assert [s.as_posix() for _, s in worklist.held] == [
        "submissions/c.cpp",
        "submissions/d.cpp",
    ]


def test_budget_stops_inside_a_bundle(tmp_path, local_env, machine):
    """大きい束に当たったときこそ効いてほしい。束の切れ目は待たない。"""
    first = make_problem_with(tmp_path, "tmp-raw", ("a", "b", "c"))
    second = make_problem_with(tmp_path, "tmp-two", ("a",))
    targets = [(first, s) for s in first.submissions()]
    targets += [(second, s) for s in second.submissions()]
    worklist = run_mod.build_worklist(targets, local_env, machine, set(), budget=2)
    assert len(worklist.jobs) == 2
    assert {p.id for p, _ in worklist.held} == {"tmp-raw", "tmp-two"}


def test_skipped_submissions_do_not_eat_the_budget(tmp_path, local_env, machine):
    """既に記録のあるものは走らせないので、budget を減らさない。"""
    problem = make_problem_with(tmp_path, "tmp-raw", ("a", "b", "c"))
    targets = [(problem, s) for s in problem.submissions()]
    known = {run_mod.build_worklist(targets, local_env, machine, set()).jobs[0].key}
    worklist = run_mod.build_worklist(targets, local_env, machine, known, budget=2)
    assert [j.submission.as_posix() for j in worklist.jobs] == [
        "submissions/b.cpp",
        "submissions/c.cpp",
    ]
    assert worklist.held == ()


def test_without_a_budget_everything_is_planned(tmp_path, local_env, machine):
    problem = make_problem_with(tmp_path, "tmp-raw", ("a", "b", "c"))
    targets = [(problem, s) for s in problem.submissions()]
    worklist = run_mod.build_worklist(targets, local_env, machine, set())
    assert len(worklist.jobs) == 3
    assert worklist.held == ()


def test_the_order_of_the_targets_is_kept(tmp_path, local_env, machine):
    """束の順は worklist が重い順に決める。ここで並べ直すと budget の意味が変わる。"""
    first = make_problem_with(tmp_path, "tmp-zzz", ("a",))
    second = make_problem_with(tmp_path, "tmp-aaa", ("a",))
    targets = [(first, s) for s in first.submissions()]
    targets += [(second, s) for s in second.submissions()]
    worklist = run_mod.build_worklist(targets, local_env, machine, set())
    assert [j.problem.id for j in worklist.jobs] == ["tmp-zzz", "tmp-aaa"]


# --- 記録から借りて、測るものだけ取りに行く ---------------------------------


def uncached_problem(tmp_path, name="tmp-uncached", submissions=("sol",)):
    """取得していないテストデータを持つ問題。原本も存在しない。"""
    directory = tmp_path / name
    directory.mkdir()
    (directory / "problem.toml").write_text(
        UNCACHED_TOML.replace("tmp-uncached", name)
    )
    (directory / "submissions").mkdir()
    for number, stem in enumerate(submissions):
        (directory / "submissions" / f"{stem}.cpp").write_text(
            f"int main() {{ return {number}; }}\n"
        )
    return problem_mod.load(directory)


def keys_for(problem, env, machine, cases_hash, submissions=None):
    """その cases_hash で測ったことにしたときのキー。"""
    decided = run_mod._decide(
        problem,
        submissions if submissions is not None else problem.submissions(),
        cases_hash, env, machine, set(),
    )
    return {job.key for job in decided.jobs}


def test_nothing_to_run_means_nothing_to_fetch(tmp_path, local_env, machine):
    """走らせるものが無い問題には、テストデータを 1 バイトも触らない。"""
    problem = uncached_problem(tmp_path)
    known = keys_for(problem, local_env, machine, "borrowed")
    worklist = run_mod.build_worklist(
        [(problem, s) for s in problem.submissions()],
        local_env, machine, known,
        borrowed={problem.id: "borrowed"},
        allow_fetch=False,
    )
    assert worklist.jobs == ()
    assert len(worklist.skipped) == 1
    # 取りに行っていれば原本が無いので pending か failed に落ちる。
    assert worklist.pending == ()
    assert worklist.failed == ()


def test_work_makes_it_fetch_after_all(tmp_path, local_env, machine):
    """1 件でも走らせるなら、そこで初めて本物を取りに行く。"""
    problem = uncached_problem(tmp_path)
    worklist = run_mod.build_worklist(
        [(problem, s) for s in problem.submissions()],
        local_env, machine, set(),
        borrowed={problem.id: "borrowed"},
        allow_fetch=False,
    )
    assert worklist.jobs == ()
    assert len(worklist.pending) == 1


def test_without_borrowing_it_fetches_to_decide(tmp_path, local_env, machine):
    """borrowed を渡さなければ今までどおり。判定のために取りに行く。"""
    problem = uncached_problem(tmp_path)
    known = keys_for(problem, local_env, machine, "borrowed")
    worklist = run_mod.build_worklist(
        [(problem, s) for s in problem.submissions()],
        local_env, machine, known,
        allow_fetch=False,
    )
    assert worklist.skipped == ()
    assert len(worklist.pending) == 1


def test_refresh_does_not_take_the_shortcut(tmp_path, local_env, machine):
    """--refresh は疑って取り直す口。借りた値で早じまいさせない。"""
    problem = uncached_problem(tmp_path)
    known = keys_for(problem, local_env, machine, "borrowed")
    worklist = run_mod.build_worklist(
        [(problem, s) for s in problem.submissions()],
        local_env, machine, known,
        borrowed={problem.id: "borrowed"},
        allow_fetch=False, refresh=True,
    )
    assert worklist.skipped == ()
    assert len(worklist.pending) == 1


def test_a_changed_workload_redoes_the_whole_problem(
    tmp_path, local_env, machine, monkeypatch
):
    """取得した本物が借りた値と違ったら、その問題の判定をやり直す。

    借りた値のままキーを作って走らせると、測った相手と記録が食い違う。
    """
    problem = uncached_problem(tmp_path, submissions=("a", "b"))
    both = [s for s in problem.submissions()]
    # a だけ「借りた値で測定済み」にしておく。b があるので取得まで進む。
    known = keys_for(problem, local_env, machine, "borrowed", submissions=both[:1])
    monkeypatch.setattr(
        run_mod.fetch,
        "ensure",
        lambda p, **kw: run_mod.fetch.Testcases(
            dir=tmp_path, cases=(), cases_hash="real"
        ),
    )
    worklist = run_mod.build_worklist(
        [(problem, s) for s in both],
        local_env, machine, known,
        borrowed={problem.id: "borrowed"},
    )
    # 本物のキーはどれも記録に無いので、a も測り直しに戻る。
    assert len(worklist.jobs) == 2
    assert worklist.skipped == ()
    assert all(job.cases_hash == "real" for job in worklist.jobs)
    assert worklist.moved == ((problem.id, "borrowed", "real"),)


def test_a_matching_workload_keeps_the_decision(
    tmp_path, local_env, machine, monkeypatch
):
    problem = uncached_problem(tmp_path, submissions=("a", "b"))
    both = [s for s in problem.submissions()]
    known = keys_for(problem, local_env, machine, "borrowed", submissions=both[:1])
    monkeypatch.setattr(
        run_mod.fetch,
        "ensure",
        lambda p, **kw: run_mod.fetch.Testcases(
            dir=tmp_path, cases=(), cases_hash="borrowed"
        ),
    )
    worklist = run_mod.build_worklist(
        [(problem, s) for s in both],
        local_env, machine, known,
        borrowed={problem.id: "borrowed"},
    )
    assert len(worklist.jobs) == 1
    assert len(worklist.skipped) == 1
    assert worklist.moved == ()


def test_a_problem_without_testdata_needs_no_borrowing(tmp_path, local_env, machine):
    """compile_only は cases_hash が空文字で確定する。借りる必要が無い。"""
    problem = make_problem(tmp_path, "int main() {}\n")
    worklist = run_mod.build_worklist(
        [(problem, s) for s in problem.submissions()],
        local_env, machine, set(), borrowed={},
    )
    assert [job.cases_hash for job in worklist.jobs] == [""]


def test_a_swapped_workload_stops_the_job(tmp_path, local_env, machine, monkeypatch):
    """キーを決めたときと違う相手を測ると、記録が嘘をつく。"""
    problem = uncached_problem(tmp_path)
    decided = run_mod._decide(
        problem, problem.submissions(), "decided", local_env, machine, set()
    )
    monkeypatch.setattr(
        run_mod.fetch,
        "ensure",
        lambda p, **kw: run_mod.fetch.Testcases(
            dir=tmp_path, cases=(), cases_hash="swapped"
        ),
    )
    with pytest.raises(run_mod.fetch.FetchError):
        run_mod.execute_job(decided.jobs[0])


# --- WA と RE は最後まで走らせる -------------------------------------------

MANUAL_TOML = """
id = "tmp-manual"
title = "t"

[harness]
kind = "raw"

[testdata]
source = "manual"
name = "tmp"

[compare]
kind = "tokens"
"""

# 1 を読んだときだけ正しく、それ以外は 0 を出す。
PICKY = "#include <cstdio>\nint main() { int n; scanf(\"%d\", &n); printf(\"%d\\n\", n == 1 ? 1 : 0); }\n"


def make_case_problem(tmp_path, source, cases):
    """手で置いたテストデータを持つ問題。fetch.ensure を差し替えて使う。"""
    directory = tmp_path / "tmp-manual"
    directory.mkdir()
    (directory / "problem.toml").write_text(MANUAL_TOML)
    (directory / "submissions").mkdir()
    (directory / "submissions" / "sol.cpp").write_text(source)
    data = tmp_path / "data"
    data.mkdir()
    built = []
    for name, (stdin, expected) in cases.items():
        (data / f"{name}.in").write_text(stdin)
        (data / f"{name}.out").write_text(expected)
        built.append(run_mod.fetch.Case(name=name, in_path=data / f"{name}.in", out_path=data / f"{name}.out"))
    testcases = run_mod.fetch.Testcases(dir=data, cases=tuple(built), cases_hash="h")
    return problem_mod.load(directory), testcases


def test_wa_keeps_running_and_records_every_failed_case(tmp_path, local_env, machine, monkeypatch):
    problem, testcases = make_case_problem(
        tmp_path, PICKY, {"a": ("1\n", "1\n"), "b": ("2\n", "2\n"), "c": ("3\n", "3\n")}
    )
    monkeypatch.setattr(run_mod.fetch, "ensure", lambda p, **kw: testcases)
    worklist = worklist_for(problem, local_env, machine)
    record = run_mod.execute_job(worklist.jobs[0])

    assert record.status == "WA"
    assert record.failed_case is not None and record.failed_case.name == "b"
    # b で止まらず c も走っている。
    assert record.failed_cases == ["b", "c"]
    assert "expected '2'" in record.failed_case.detail
