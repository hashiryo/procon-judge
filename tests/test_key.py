"""キーの計算。ここが壊れると、走らせるべきものを走らせなくなる。"""

import hashlib

import pytest

from pj import key as key_mod
from pj import problem as problem_mod

BASE_TOML = """
id = "x"
title = "T"

[limits]
tle_sec = 5.0
mle_mb = 256

[harness]
kind = "{harness}"

[testdata]
source = "none"

[compare]
kind = "compile_only"
"""


def make_problem(tmp_path, *, harness="raw", extra="", base_cpp=None, common=None):
    directory = tmp_path / "x"
    directory.mkdir(exist_ok=True)
    (directory / "problem.toml").write_text(
        BASE_TOML.format(harness=harness) + extra
    )
    if base_cpp is not None:
        (directory / "base.cpp").write_text(base_cpp)
    if common is not None:
        (directory / "common.hpp").write_text(common)
    return problem_mod.load(directory)


# --- normalize -------------------------------------------------------------


def test_normalize_drops_trailing_whitespace():
    assert key_mod.normalize("a   \nb\t\n") == "a\nb"


def test_normalize_drops_blank_lines():
    assert key_mod.normalize("a\n\n\n   \nb\n") == "a\nb"


def test_normalize_makes_crlf_and_lf_equal():
    assert key_mod.normalize("a\r\nb\r\n") == key_mod.normalize("a\nb\n")


def test_normalize_keeps_indentation():
    assert key_mod.normalize("  a\n") == "  a"


# --- submission_hash -------------------------------------------------------


def test_formatting_only_change_keeps_the_hash(tmp_path):
    source = tmp_path / "sol.hpp"
    source.write_text("int f() {\n  return 1;\n}\n")
    before = key_mod.submission_hash(source, [tmp_path]).submission_hash
    source.write_text("int f() {   \n\n  return 1;\n}\n\n\n")
    assert key_mod.submission_hash(source, [tmp_path]).submission_hash == before


def test_content_change_moves_the_hash(tmp_path):
    source = tmp_path / "sol.hpp"
    source.write_text("int f() { return 1; }\n")
    before = key_mod.submission_hash(source, [tmp_path]).submission_hash
    source.write_text("int f() { return 2; }\n")
    assert key_mod.submission_hash(source, [tmp_path]).submission_hash != before


def test_header_in_the_closure_moves_the_hash(tmp_path):
    header = tmp_path / "dep.hpp"
    header.write_text("int g() { return 1; }\n")
    source = tmp_path / "sol.hpp"
    source.write_text('#include "dep.hpp"\nint f() { return g(); }\n')
    before = key_mod.submission_hash(source, [tmp_path]).submission_hash
    header.write_text("int g() { return 2; }\n")
    assert key_mod.submission_hash(source, [tmp_path]).submission_hash != before


def test_includes_are_reported(tmp_path):
    (tmp_path / "dep.hpp").write_text("")
    source = tmp_path / "sol.hpp"
    source.write_text('#include "dep.hpp"\n')
    assert key_mod.submission_hash(source, [tmp_path]).includes == ("dep.hpp",)


def test_renaming_a_header_moves_the_hash(tmp_path):
    # パスもハッシュに入れるので、中身が同じでも置き場所が変われば別物になる。
    (tmp_path / "a.hpp").write_text("int g();\n")
    (tmp_path / "b.hpp").write_text("int g();\n")
    source = tmp_path / "sol.hpp"
    source.write_text('#include "a.hpp"\n')
    before = key_mod.submission_hash(source, [tmp_path]).submission_hash
    source.write_text('#include "b.hpp"\n')
    assert key_mod.submission_hash(source, [tmp_path]).submission_hash != before


# --- harness_hash / problem_hash -------------------------------------------


def test_raw_harness_is_the_empty_hash(tmp_path):
    problem = make_problem(tmp_path, harness="raw")
    assert key_mod.harness_hash(problem) == hashlib.sha256(b"").hexdigest()


def test_base_harness_follows_base_cpp(tmp_path):
    problem = make_problem(tmp_path, harness="base", base_cpp="int main() {}\n")
    before = key_mod.harness_hash(problem)
    (problem.dir / "base.cpp").write_text("int main() { return 1; }\n")
    assert key_mod.harness_hash(problem) != before


def test_base_harness_follows_common_hpp(tmp_path):
    problem = make_problem(
        tmp_path,
        harness="base",
        base_cpp='#include "common.hpp"\nint main() {}\n',
        common="// a\n",
    )
    paths = [problem.dir]
    before = key_mod.harness_hash(problem, paths)
    (problem.dir / "common.hpp").write_text("// b\n")
    assert key_mod.harness_hash(problem, paths) != before


def test_base_harness_follows_the_shared_header(tmp_path):
    """共有のハーネスヘッダを書き換えたら測り直しが起きてほしい。"""
    shared = tmp_path / "harness"
    shared.mkdir()
    (shared / "pj.hpp").write_text("// a\n")
    problem = make_problem(
        tmp_path, harness="base", base_cpp='#include "pj.hpp"\nint main() {}\n'
    )
    paths = [problem.dir, shared]
    before = key_mod.harness_hash(problem, paths)
    (shared / "pj.hpp").write_text("// b\n")
    assert key_mod.harness_hash(problem, paths) != before


def test_base_harness_ignores_formatting(tmp_path):
    problem = make_problem(
        tmp_path, harness="base", base_cpp='#include "pj.hpp"\nint main() {}\n'
    )
    shared = tmp_path / "harness"
    shared.mkdir()
    (shared / "pj.hpp").write_text("// a\n")
    paths = [problem.dir, shared]
    before = key_mod.harness_hash(problem, paths)
    (shared / "pj.hpp").write_text("// a   \n\n")
    assert key_mod.harness_hash(problem, paths) == before


def test_problem_hash_ignores_the_title(tmp_path):
    problem = make_problem(tmp_path)
    before = key_mod.problem_hash(problem)
    path = problem.dir / "problem.toml"
    path.write_text(path.read_text().replace('title = "T"', 'title = "renamed"'))
    assert key_mod.problem_hash(problem_mod.load(problem.dir)) == before


def test_problem_hash_follows_the_limits(tmp_path):
    problem = make_problem(tmp_path)
    before = key_mod.problem_hash(problem)
    path = problem.dir / "problem.toml"
    path.write_text(path.read_text().replace("tle_sec = 5.0", "tle_sec = 4.0"))
    assert key_mod.problem_hash(problem_mod.load(problem.dir)) != before


def test_problem_hash_ignores_an_explicit_default(tmp_path):
    with_default = make_problem(tmp_path)
    before = key_mod.problem_hash(with_default)
    path = with_default.dir / "problem.toml"
    path.write_text(
        path.read_text().replace(
            '[testdata]\nsource = "none"', '[testdata]\nsource = "none"\ngenerator = "gen.py"'
        )
    )
    assert key_mod.problem_hash(problem_mod.load(with_default.dir)) == before


# --- compute ---------------------------------------------------------------


FIELDS = {
    "submission": "submissions/a.hpp",
    "submission_hash": "s",
    "harness_hash": "h",
    "problem_hash": "p",
    "cases_hash": "c",
    "env": "local",
    "compiler_version": "g++ 15",
    "cxxflags": "-O2",
    "cpu_model": "Apple M2 Max",
}


@pytest.mark.parametrize("field", sorted(FIELDS))
def test_every_field_moves_the_key(field):
    before = key_mod.compute(**FIELDS)
    after = key_mod.compute(**{**FIELDS, field: FIELDS[field] + "!"})
    assert before != after


def test_fields_do_not_run_together():
    # 区切りを入れていないと ("ab", "c") と ("a", "bc") が同じキーになる。
    left = key_mod.compute(**{**FIELDS, "env": "ab", "compiler_version": "c"})
    right = key_mod.compute(**{**FIELDS, "env": "a", "compiler_version": "bc"})
    assert left != right


def test_the_key_is_stable():
    assert key_mod.compute(**FIELDS) == key_mod.compute(**FIELDS)


def test_the_path_separates_identical_submissions():
    # 中身が同じでも別のファイルなら別の提出として数える。
    # そうしないと片方の提出ページに記録が出ない。
    left = key_mod.compute(**{**FIELDS, "submission": "submissions/a.hpp"})
    right = key_mod.compute(**{**FIELDS, "submission": "submissions/b.hpp"})
    assert left != right
