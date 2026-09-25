"""problem.toml の読み込みと検証。"""

import pytest

from pj import problem as problem_mod

MINIMAL = """
id = "{id}"
title = "T"

[harness]
kind = "raw"

[testdata]
source = "none"

[compare]
kind = "compile_only"
"""


def make(tmp_path, name, body=None):
    directory = tmp_path / name
    directory.mkdir()
    (directory / "problem.toml").write_text(
        body if body is not None else MINIMAL.format(id=name)
    )
    return directory


def test_loads_minimal(tmp_path):
    p = problem_mod.load(make(tmp_path, "x"))
    assert p.id == "x"
    assert p.harness_kind == "raw"
    assert p.limits.tle_sec == 5.0
    assert p.limits.mle_mb == 256


def test_id_must_match_directory(tmp_path):
    directory = make(tmp_path, "x", MINIMAL.format(id="other"))
    with pytest.raises(problem_mod.ProblemError, match="一致しません"):
        problem_mod.load(directory)


def test_base_harness_needs_base_cpp(tmp_path):
    body = MINIMAL.format(id="x").replace('kind = "raw"', 'kind = "base"')
    with pytest.raises(problem_mod.ProblemError, match="base.cpp"):
        problem_mod.load(make(tmp_path, "x", body))


def test_unknown_source_is_rejected(tmp_path):
    body = MINIMAL.format(id="x").replace('source = "none"', 'source = "codeforces"\nname = "1"')
    with pytest.raises(problem_mod.ProblemError, match="testdata.source"):
        problem_mod.load(make(tmp_path, "x", body))


def test_aoj_needs_a_name(tmp_path):
    body = MINIMAL.format(id="x").replace('source = "none"', 'source = "aoj"')
    body = body.replace('kind = "compile_only"', 'kind = "tokens"')
    with pytest.raises(problem_mod.ProblemError, match="name"):
        problem_mod.load(make(tmp_path, "x", body))


def test_library_checker_needs_name(tmp_path):
    body = MINIMAL.format(id="x").replace('source = "none"', 'source = "library_checker"')
    body = body.replace('kind = "compile_only"', 'kind = "checker"')
    with pytest.raises(problem_mod.ProblemError, match="name"):
        problem_mod.load(make(tmp_path, "x", body))


def test_none_source_requires_compile_only(tmp_path):
    body = MINIMAL.format(id="x").replace('kind = "compile_only"', 'kind = "tokens"')
    with pytest.raises(problem_mod.ProblemError, match="compile_only"):
        problem_mod.load(make(tmp_path, "x", body))


def test_none_source_accepts_exit_code(tmp_path):
    body = MINIMAL.format(id="x").replace('kind = "compile_only"', 'kind = "exit_code"')
    p = problem_mod.load(make(tmp_path, "x", body))
    assert p.compare.kind == "exit_code"


def test_exit_code_requires_none_source(tmp_path):
    body = with_source("x", "aoj", "1").replace('kind = "compile_only"', 'kind = "exit_code"')
    with pytest.raises(problem_mod.ProblemError, match="exit_code"):
        problem_mod.load(make(tmp_path, "x", body))


def test_submissions_skip_underscore_prefix(tmp_path):
    directory = make(tmp_path, "x")
    (directory / "submissions").mkdir()
    (directory / "submissions" / "_helper.cpp").write_text("")
    (directory / "submissions" / "sol.cpp").write_text("")
    (directory / "submissions" / "notes.md").write_text("")
    p = problem_mod.load(directory)
    assert [s.as_posix() for s in p.submissions()] == ["submissions/sol.cpp"]


def test_real_problem_loads():
    p = problem_mod.load_by_id("yosupo-point-add-range-sum")
    assert p.harness_kind == "base"
    assert p.compare.kind == "checker"
    assert "submissions/naive.hpp" in [s.as_posix() for s in p.submissions()]


def with_source(problem_id, source, name="1"):
    body = MINIMAL.format(id=problem_id)
    return body.replace('source = "none"', f'source = "{source}"\nname = "{name}"')


def test_prefix_matching_the_source_does_not_warn(tmp_path):
    body = with_source("yuki-649", "yukicoder", "649")
    p = problem_mod.load(make(tmp_path, "yuki-649", body))
    assert problem_mod.warnings(p) == []


def test_prefix_not_matching_the_source_warns(tmp_path):
    body = with_source("aoj-649", "yukicoder", "649")
    p = problem_mod.load(make(tmp_path, "aoj-649", body))
    assert "'yuki-'" in problem_mod.warnings(p)[0]


def test_missing_prefix_warns(tmp_path):
    body = with_source("gf2-64", "library_checker", "math/x")
    p = problem_mod.load(make(tmp_path, "gf2-64", body))
    assert "'yosupo-'" in problem_mod.warnings(p)[0]


def test_loj_prefix_matches_the_source(tmp_path):
    body = with_source("loj-2419", "loj", "2419")
    p = problem_mod.load(make(tmp_path, "loj-2419", body))
    assert problem_mod.warnings(p) == []
    assert (p.testdata.source, p.testdata.name) == ("loj", "2419")


def test_loj_needs_a_name(tmp_path):
    body = MINIMAL.format(id="loj-1").replace('source = "none"', 'source = "loj"')
    with pytest.raises(problem_mod.ProblemError, match="name"):
        problem_mod.load(make(tmp_path, "loj-1", body))


def test_source_without_a_prefix_never_warns(tmp_path):
    body = with_source("aoj-DSL_2_B", "manual")
    p = problem_mod.load(make(tmp_path, "aoj-DSL_2_B", body))
    assert problem_mod.warnings(p) == []


def test_float_needs_a_tolerance(tmp_path):
    body = MINIMAL.format(id="x").replace('source = "none"', 'source = "aoj"\nname = "1"')
    body = body.replace('kind = "compile_only"', 'kind = "float"')
    with pytest.raises(problem_mod.ProblemError, match="abs_tol"):
        problem_mod.load(make(tmp_path, "x", body))
    p = problem_mod.load(
        make(tmp_path, "y", body.replace('id = "x"', 'id = "y"') + "abs_tol = 1e-6\nrel_tol = 1e-6\n")
    )
    assert p.compare.kind == "float"
    assert p.compare.abs_tol == 1e-6 and p.compare.rel_tol == 1e-6


def test_tolerances_belong_to_float_only(tmp_path):
    body = MINIMAL.format(id="x") + "abs_tol = 1e-6\n"
    with pytest.raises(problem_mod.ProblemError, match="float"):
        problem_mod.load(make(tmp_path, "x", body))


def test_url_is_optional_and_kept(tmp_path):
    p = problem_mod.load(make(tmp_path, "x"))
    assert p.url == ""
    body = MINIMAL.format(id="y").replace('title = "T"', 'title = "T"\nurl = "https://atcoder.jp/contests/abc1/tasks/abc1_a"')
    assert problem_mod.load(make(tmp_path, "y", body)).url.endswith("abc1_a")


# --- 階層 -------------------------------------------------------------------


def make_nested(tmp_path, *parts, body=None):
    directory = tmp_path.joinpath("problems", *parts)
    directory.mkdir(parents=True)
    (directory / "problem.toml").write_text(body or MINIMAL.format(id="-".join(parts)))
    (directory / "submissions").mkdir()
    return directory


def test_the_id_is_the_path_below_problems_joined_with_dashes(tmp_path):
    assert problem_mod.problem_id_of(make_nested(tmp_path, "atcoder", "abc172-d")) == "atcoder-abc172-d"
    assert problem_mod.problem_id_of(make_nested(tmp_path, "self", "gf2-64", "pow")) == "self-gf2-64-pow"
    assert problem_mod.problem_id_of(make_nested(tmp_path, "warshall-floyd")) == "warshall-floyd"


def test_a_directory_outside_problems_uses_its_own_name(tmp_path):
    assert problem_mod.problem_id_of(make(tmp_path, "x")) == "x"


def test_a_nested_problem_loads_with_the_joined_id(tmp_path):
    p = problem_mod.load(make_nested(tmp_path, "yosupo", "unionfind"))
    assert p.id == "yosupo-unionfind"


def test_a_nested_problem_with_the_wrong_id_is_rejected(tmp_path):
    directory = make_nested(tmp_path, "atcoder", "abc172-d", body=MINIMAL.format(id="abc172-d"))
    with pytest.raises(problem_mod.ProblemError, match="atcoder-abc172-d"):
        problem_mod.load(directory)


def test_all_problem_dirs_walks_the_tree_and_skips_underscore_dirs(tmp_path, monkeypatch):
    monkeypatch.setattr(problem_mod, "PROBLEMS_DIR", tmp_path / "problems")
    a = make_nested(tmp_path, "atcoder", "abc172-d")
    b = make_nested(tmp_path, "gf2-64", "pow")
    c = make_nested(tmp_path, "warshall-floyd")
    (tmp_path / "problems" / "_shared" / "gf2-64").mkdir(parents=True)
    (tmp_path / "problems" / "_shared" / "gf2-64" / "problem.toml").write_text("id = 'nope'\n")
    (tmp_path / "problems" / "atcoder" / "__pycache__").mkdir()
    assert problem_mod.all_problem_dirs() == sorted([a, b, c], key=problem_mod.problem_id_of)


def test_two_directories_with_the_same_id_are_an_error(tmp_path, monkeypatch):
    monkeypatch.setattr(problem_mod, "PROBLEMS_DIR", tmp_path / "problems")
    make_nested(tmp_path, "gf2-64", "pow")
    make_nested(tmp_path, "gf2-64-pow")
    with pytest.raises(problem_mod.ProblemError, match="2 か所"):
        problem_mod.all_problem_dirs()


def test_load_by_id_finds_a_nested_problem(tmp_path, monkeypatch):
    monkeypatch.setattr(problem_mod, "PROBLEMS_DIR", tmp_path / "problems")
    make_nested(tmp_path, "yuki", "1234")
    assert problem_mod.load_by_id("yuki-1234").id == "yuki-1234"
    with pytest.raises(problem_mod.ProblemError, match="ありません"):
        problem_mod.load_by_id("yuki-9999")


def test_new_problems_go_under_the_judge_prefix(tmp_path):
    root = tmp_path / "problems"
    assert problem_mod.dir_for_new("yosupo-unionfind", root) == root / "yosupo" / "unionfind"
    assert problem_mod.dir_for_new("atcoder-abc172-d", root) == root / "atcoder" / "abc172-d"
    # 自作の問題も self/ の下。族のディレクトリは人が決めるので、self/ の中では平らに置く。
    assert problem_mod.dir_for_new("self-mat-unit-test", root) == root / "self" / "mat-unit-test"
    assert problem_mod.dir_for_new("gf2-64-pow", root) == root / "gf2-64-pow"
