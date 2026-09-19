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


def test_source_without_a_prefix_never_warns(tmp_path):
    body = with_source("aoj-DSL_2_B", "manual")
    p = problem_mod.load(make(tmp_path, "aoj-DSL_2_B", body))
    assert problem_mod.warnings(p) == []
