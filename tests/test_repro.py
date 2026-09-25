"""pj repro。落ちたケースの差分とファイルの場所を出す。"""

import io
from pathlib import Path

import pytest

from pj import environment as env_mod
from pj import problem as problem_mod
from pj import repro as repro_mod
from pj import run as run_mod
from tests.test_run import PICKY, RAW_TOML, make_case_problem


@pytest.fixture
def local_env():
    return env_mod.load("local")


def run_repro(problem, env, case=None, cases=None):
    out = io.StringIO()
    code = repro_mod.repro(
        problem, Path("submissions/sol.cpp"), env, case=case, cases=cases, out=out
    )
    return code, out.getvalue()


def test_repro_lists_every_failing_case_with_a_diff(tmp_path, local_env, monkeypatch):
    problem, testcases = make_case_problem(
        tmp_path, PICKY, {"a": ("1\n", "1\n"), "b": ("2\n", "2\n"), "c": ("3\n", "3\n")}
    )
    monkeypatch.setattr(repro_mod.fetch, "ensure", lambda p, **kw: testcases)
    code, text = run_repro(problem, local_env)

    assert code == 1
    assert "AC  a" in text and "WA  b" in text and "WA  c" in text
    assert "--- b: WA" in text
    assert "-2" in text and "+0" in text  # 期待 2、実際 0
    assert "3 ケース中 1 件 AC / 2 件 失敗" in text
    for label in ("入力", "期待出力", "実際の出力", "stderr"):
        assert label in text


def test_repro_runs_only_the_named_case(tmp_path, local_env, monkeypatch):
    problem, testcases = make_case_problem(
        tmp_path, PICKY, {"a": ("1\n", "1\n"), "b": ("2\n", "2\n")}
    )
    monkeypatch.setattr(repro_mod.fetch, "ensure", lambda p, **kw: testcases)
    code, text = run_repro(problem, local_env, case="a")
    assert code == 0
    assert "AC  a" in text
    assert "WA  b" not in text
    assert "1 ケース中 1 件 AC / 0 件 失敗" in text


def test_repro_runs_only_the_first_cases_by_name(tmp_path, local_env, monkeypatch):
    # CI を待つ前の手早い確認。名前順の先頭から N ケースだけ走らせる。
    problem, testcases = make_case_problem(
        tmp_path, PICKY, {"a": ("1\n", "1\n"), "b": ("2\n", "2\n"), "c": ("3\n", "3\n")}
    )
    monkeypatch.setattr(repro_mod.fetch, "ensure", lambda p, **kw: testcases)
    code, text = run_repro(problem, local_env, cases=1)
    assert code == 0
    assert "AC  a" in text
    assert "WA  b" not in text and "WA  c" not in text
    assert "1 ケース中 1 件 AC / 0 件 失敗 (全 3 ケースのうち先頭 1 ケース)" in text


def test_repro_runs_every_case_when_n_is_larger(tmp_path, local_env, monkeypatch):
    problem, testcases = make_case_problem(
        tmp_path, PICKY, {"a": ("1\n", "1\n"), "b": ("2\n", "2\n")}
    )
    monkeypatch.setattr(repro_mod.fetch, "ensure", lambda p, **kw: testcases)
    code, text = run_repro(problem, local_env, cases=5)
    assert code == 1
    assert "AC  a" in text and "WA  b" in text
    assert "2 ケース中 1 件 AC / 1 件 失敗\n" in text


def test_repro_rejects_a_case_count_below_one(tmp_path, local_env):
    problem, _ = make_case_problem(tmp_path, PICKY, {"a": ("1\n", "1\n")})
    with pytest.raises(repro_mod.ReproError, match="1 以上"):
        run_repro(problem, local_env, cases=0)


def test_repro_rejects_an_unknown_case(tmp_path, local_env, monkeypatch):
    problem, testcases = make_case_problem(tmp_path, PICKY, {"a": ("1\n", "1\n")})
    monkeypatch.setattr(repro_mod.fetch, "ensure", lambda p, **kw: testcases)
    with pytest.raises(repro_mod.ReproError, match="a"):
        run_repro(problem, local_env, case="zzz")


def test_repro_reports_a_compile_error(tmp_path, local_env):
    directory = tmp_path / "tmp-raw"
    directory.mkdir()
    (directory / "problem.toml").write_text(RAW_TOML)
    (directory / "submissions").mkdir()
    (directory / "submissions" / "sol.cpp").write_text("int main() { return nope; }\n")
    problem = problem_mod.load(directory)
    code, text = run_repro(problem, local_env)
    assert code == 1
    assert "CE" in text and "nope" in text


def test_repro_stops_after_building_a_compile_only_problem(tmp_path, local_env):
    directory = tmp_path / "tmp-raw"
    directory.mkdir()
    (directory / "problem.toml").write_text(RAW_TOML)
    (directory / "submissions").mkdir()
    (directory / "submissions" / "sol.cpp").write_text("int main() { return 0; }\n")
    problem = problem_mod.load(directory)
    code, text = run_repro(problem, local_env)
    assert code == 0
    assert "組めたので終わります" in text


def test_repro_runs_an_exit_code_problem_once(tmp_path, local_env):
    directory = tmp_path / "tmp-raw"
    directory.mkdir()
    (directory / "problem.toml").write_text(
        RAW_TOML.replace('kind = "compile_only"', 'kind = "exit_code"')
    )
    (directory / "submissions").mkdir()
    (directory / "submissions" / "sol.cpp").write_text(
        "#include <cassert>\nint main() { assert(false); }\n"
    )
    problem = problem_mod.load(directory)
    code, text = run_repro(problem, local_env)
    assert code == 1
    assert "RE  self" in text
    assert "stderr" in text


def test_unified_diff_is_capped(tmp_path):
    expected = tmp_path / "e"
    actual = tmp_path / "a"
    expected.write_text("\n".join(str(i) for i in range(1000)) + "\n")
    actual.write_text("\n".join(str(i * 2 + 1) for i in range(1000)) + "\n")
    lines = repro_mod.unified_diff(expected, actual)
    assert len(lines) == repro_mod.DIFF_LINES + 1
    assert lines[-1].startswith("...")


def test_repro_command_matches_the_cli():
    from pj.site.build import repro_command

    assert (
        repro_command("aoj-0629", "submissions/lib-min-left.hpp", "01-01")
        == "pj repro --problem aoj-0629 --submission submissions/lib-min-left.hpp --case 01-01"
    )
    assert repro_command("p", "submissions/a.hpp", None) == "pj repro --problem p --submission submissions/a.hpp"
    assert run_mod.judge_case is not None
