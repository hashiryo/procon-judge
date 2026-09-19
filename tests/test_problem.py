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


def test_planned_source_is_rejected_clearly(tmp_path):
    body = MINIMAL.format(id="x").replace('source = "none"', 'source = "aoj"\nname = "DSL_2_A"')
    with pytest.raises(problem_mod.ProblemError, match="未実装"):
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
