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


# --- tokenize / normalize_cxx ------------------------------------------------


def same(a, b):
    return key_mod.normalize_cxx(a) == key_mod.normalize_cxx(b)


def test_comments_are_dropped():
    assert key_mod.tokenize("int a; // c\n/* d\n e */ int b;") == [
        "int", "a", ";", "int", "b", ";",
    ]


def test_formatting_is_ignored():
    assert same("int f(){return 1;}", "int f() {\n  return 1;\n}\n")


def test_a_comment_only_change_is_not_a_change():
    assert same("int f();", "// doc\nint f(); /* trailing */")


def test_punctuators_take_the_longest_match():
    """記号を 1 文字ずつにすると a + +b と a ++b が同じになる。"""
    assert not same("a + +b", "a ++b")
    assert key_mod.tokenize("a>>=b; x->*y; a<=>b; p::q") == [
        "a", ">>=", "b", ";", "x", "->*", "y", ";", "a", "<=>", "b", ";", "p", "::", "q",
    ]


def test_a_directive_line_is_one_token():
    assert key_mod.tokenize("#define F(x) x\nint a;") == ["#define F(x) x", "int", "a", ";"]


def test_a_directive_keeps_the_space_before_the_paren():
    """#define F(x) は関数形式、#define F (x) は置換列に (x) を持つ。別物。"""
    assert not same("#define F(x) x", "#define F (x) x")


def test_a_directive_collapses_its_whitespace():
    assert same("#  define  X   1 // c\n", "#define X 1\n")
    assert same('#include "a.hpp" /* k */ // c', '#include "a.hpp"')


def test_a_comment_marker_inside_a_string_is_kept():
    assert key_mod.tokenize('s = "http://x"; // c') == ["s", "=", '"http://x"', ";"]
    assert key_mod.tokenize('#define U "http://x" // c') == ['#define U "http://x"']


def test_a_raw_string_is_one_token():
    assert key_mod.tokenize('R"(a\n*/ b)" x') == ['R"(a\n*/ b)"', "x"]
    assert key_mod.tokenize('LR"q(a)"q)q" y') == ['LR"q(a)"q)q"', "y"]


def test_escapes_end_no_literal():
    assert key_mod.tokenize("c = '\\'' ; s = \"a\\\"b\";") == [
        "c", "=", "'\\''", ";", "s", "=", '"a\\"b"', ";",
    ]


def test_literal_prefixes_stay_attached():
    assert key_mod.tokenize("u8\"x\" L'y' U\"z\"") == ['u8"x"', "L'y'", 'U"z"']


def test_a_user_defined_suffix_stays_attached():
    assert not same('"a"_s', '"a" _s')
    assert key_mod.tokenize("1.5_km") == ["1.5_km"]


def test_numbers_follow_the_preprocessor_grammar():
    assert key_mod.tokenize("x = 1e+5 + 0x1p-3 + 1'000'000 + .5f;") == [
        "x", "=", "1e+5", "+", "0x1p-3", "+", "1'000'000", "+", ".5f", ";",
    ]


def test_line_continuations_are_joined_first():
    assert same("#define X \\\n  1\n", "#define X 1\n")
    assert same("in\\\nt a;", "int a;")


def test_crlf_and_lf_tokenize_alike():
    assert same("int a;\r\nint b;\r\n", "int a;\nint b;\n")


def test_only_cxx_files_are_tokenized(tmp_path):
    """Python は字下げに意味があるので、空白を捨ててはいけない。"""
    cxx = tmp_path / "a.hpp"
    py = tmp_path / "gen.py"
    cxx.write_text("if (x) {\n  a();\n}\n")
    before = key_mod.normalize_file(cxx)
    cxx.write_text("if (x) {\na();\n}\n")
    assert key_mod.normalize_file(cxx) == before

    py.write_text("if x:\n  a()\n")
    before = key_mod.normalize_file(py)
    py.write_text("if x:\na()\n")
    assert key_mod.normalize_file(py) != before


# --- submission_hash -------------------------------------------------------


def test_a_comment_only_change_keeps_the_hash(tmp_path):
    source = tmp_path / "sol.hpp"
    source.write_text("int f() { return 1; }\n")
    before = key_mod.submission_hash(source, [tmp_path]).submission_hash
    source.write_text("// explains f\nint f() { return 1; /* one */ }\n")
    assert key_mod.submission_hash(source, [tmp_path]).submission_hash == before


def test_file_hashes_cover_the_entry_and_the_closure(tmp_path):
    (tmp_path / "dep.hpp").write_text("int g();\n")
    source = tmp_path / "sol.hpp"
    source.write_text('#include "dep.hpp"\nint f();\n')
    files = dict(key_mod.submission_hash(source, [tmp_path]).file_hashes)
    assert set(files) == {"sol.hpp", "dep.hpp"}
    assert all(len(h) == key_mod.FILE_HASH_CHARS for h in files.values())


def test_editing_a_header_moves_only_its_file_hash(tmp_path):
    (tmp_path / "dep.hpp").write_text("int g();\n")
    source = tmp_path / "sol.hpp"
    source.write_text('#include "dep.hpp"\nint f();\n')
    before = dict(key_mod.submission_hash(source, [tmp_path]).file_hashes)
    (tmp_path / "dep.hpp").write_text("int g(int);\n")
    after = dict(key_mod.submission_hash(source, [tmp_path]).file_hashes)
    assert after["sol.hpp"] == before["sol.hpp"]
    assert after["dep.hpp"] != before["dep.hpp"]


def test_harness_key_names_its_files(tmp_path):
    shared = tmp_path / "harness"
    shared.mkdir()
    (shared / "pj.hpp").write_text("int a;\n")
    problem = make_problem(
        tmp_path, harness="base", base_cpp='#include "pj.hpp"\nint main() {}\n'
    )
    harness = key_mod.harness_key(problem, [problem.dir, shared])
    assert set(dict(harness.file_hashes)) == {"base.cpp", "pj.hpp"}
    assert harness.harness_hash == key_mod.harness_hash(problem, [problem.dir, shared])


def test_a_raw_harness_has_no_files(tmp_path):
    problem = make_problem(tmp_path, harness="raw")
    assert key_mod.harness_key(problem).file_hashes == ()


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
        common="int a;\n",
    )
    paths = [problem.dir]
    before = key_mod.harness_hash(problem, paths)
    (problem.dir / "common.hpp").write_text("int b;\n")
    assert key_mod.harness_hash(problem, paths) != before


def test_base_harness_follows_the_shared_header(tmp_path):
    """共有のハーネスヘッダを書き換えたら測り直しが起きてほしい。"""
    shared = tmp_path / "harness"
    shared.mkdir()
    (shared / "pj.hpp").write_text("int a;\n")
    problem = make_problem(
        tmp_path, harness="base", base_cpp='#include "pj.hpp"\nint main() {}\n'
    )
    paths = [problem.dir, shared]
    before = key_mod.harness_hash(problem, paths)
    (shared / "pj.hpp").write_text("int b;\n")
    assert key_mod.harness_hash(problem, paths) != before


def test_base_harness_ignores_formatting(tmp_path):
    problem = make_problem(
        tmp_path, harness="base", base_cpp='#include "pj.hpp"\nint main() {}\n'
    )
    shared = tmp_path / "harness"
    shared.mkdir()
    (shared / "pj.hpp").write_text("int a;\n")
    paths = [problem.dir, shared]
    before = key_mod.harness_hash(problem, paths)
    (shared / "pj.hpp").write_text("int  a ;   // now with a comment\n\n")
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


# --- local のテストデータはリポジトリの中にある ------------------------------

LOCAL_TOML = """
id = "x"
title = "T"

[harness]
kind = "raw"

[testdata]
source = "local"
count = 5
generator = "gen.py"
reference = "ref.py"

[compare]
kind = "tokens"
"""


def make_local_problem(tmp_path, *, generator="print(1)\n", reference="print(2)\n"):
    directory = tmp_path / "x"
    directory.mkdir(exist_ok=True)
    (directory / "problem.toml").write_text(LOCAL_TOML)
    (directory / "gen.py").write_text(generator)
    (directory / "ref.py").write_text(reference)
    return problem_mod.load(directory)


def test_problem_hash_follows_the_generator(tmp_path):
    """plan は cases_hash を借りるので、ここが動かないとジョブが立たない。"""
    problem = make_local_problem(tmp_path)
    before = key_mod.problem_hash(problem)
    (problem.dir / "gen.py").write_text("print(99)\n")
    assert key_mod.problem_hash(problem_mod.load(problem.dir)) != before


def test_problem_hash_follows_the_reference(tmp_path):
    problem = make_local_problem(tmp_path)
    before = key_mod.problem_hash(problem)
    (problem.dir / "ref.py").write_text("print(99)\n")
    assert key_mod.problem_hash(problem_mod.load(problem.dir)) != before


def test_reformatting_the_generator_keeps_the_problem_hash(tmp_path):
    """整形だけの変更で測り直さない。提出の扱いと揃える。"""
    problem = make_local_problem(tmp_path, generator="print(1)\n")
    before = key_mod.problem_hash(problem)
    (problem.dir / "gen.py").write_text("print(1)   \n\n\n")
    assert key_mod.problem_hash(problem_mod.load(problem.dir)) == before


def test_other_sources_do_not_carry_the_generator(tmp_path):
    """local 以外に足すと、既存の記録が全部測り直しになる。"""
    problem = make_problem(tmp_path)
    payload = key_mod.problem_hash(problem)
    (tmp_path / "x" / "gen.py").write_text("print(1)\n")
    assert key_mod.problem_hash(problem_mod.load(problem.dir)) == payload


def test_problem_hash_follows_the_tolerance_only_for_float(tmp_path):
    """許容誤差は float のときだけ鍵に入る。ほかの問題の鍵を動かさないため。"""
    from pj import problem as problem_mod

    base = """
id = "f"
title = "T"

[harness]
kind = "raw"

[testdata]
source = "aoj"
name = "1"

[compare]
kind = "float"
abs_tol = 1e-6
rel_tol = 1e-6
"""
    directory = tmp_path / "f"
    directory.mkdir()
    (directory / "problem.toml").write_text(base)
    before = key_mod.problem_hash(problem_mod.load(directory))
    (directory / "problem.toml").write_text(base.replace("abs_tol = 1e-6", "abs_tol = 1e-3"))
    assert key_mod.problem_hash(problem_mod.load(directory)) != before
