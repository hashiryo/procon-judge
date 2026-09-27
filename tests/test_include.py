"""include 閉包の解決。欠けるとキーを誤らせるので固めておく。"""

import pytest

from pj import cli
from pj import include as include_mod
from pj import problem as problem_mod
from pj.include import closure, label_for, scan


def write(directory, name, text):
    path = directory / name
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text)
    return path


def test_scan_takes_quoted_only():
    text = '#include <vector>\n#include "a.hpp"\n#  include   "b/c.hpp"\n'
    assert scan(text) == ["a.hpp", "b/c.hpp"]


def test_scan_skips_line_comments():
    text = '// #include "dead.hpp"\n#include "live.hpp"\n'
    assert scan(text) == ["live.hpp"]


def test_follows_recursively(tmp_path):
    write(tmp_path, "c.hpp", "")
    write(tmp_path, "b.hpp", '#include "c.hpp"\n')
    entry = write(tmp_path, "a.hpp", '#include "b.hpp"\n')
    found = closure(entry, [tmp_path])
    assert found.labels == ("b.hpp", "c.hpp")


def test_entry_is_not_in_its_own_closure(tmp_path):
    entry = write(tmp_path, "a.hpp", '#include "b.hpp"\n')
    write(tmp_path, "b.hpp", "")
    assert closure(entry, [tmp_path]).labels == ("b.hpp",)


def test_angle_brackets_are_not_followed(tmp_path):
    write(tmp_path, "vector", "should not be read")
    entry = write(tmp_path, "a.hpp", "#include <vector>\n")
    assert closure(entry, [tmp_path]).labels == ()


def test_cycle_terminates(tmp_path):
    entry = write(tmp_path, "a.hpp", '#include "b.hpp"\n')
    write(tmp_path, "b.hpp", '#include "a.hpp"\n#include "c.hpp"\n')
    write(tmp_path, "c.hpp", '#include "b.hpp"\n')
    assert closure(entry, [tmp_path]).labels == ("b.hpp", "c.hpp")


def test_resolves_relative_to_the_including_file(tmp_path):
    write(tmp_path, "sub/dep.hpp", "")
    write(tmp_path, "sub/b.hpp", '#include "dep.hpp"\n')
    entry = write(tmp_path, "a.hpp", '#include "sub/b.hpp"\n')
    assert closure(entry, [tmp_path]).labels == ("sub/b.hpp", "sub/dep.hpp")


def test_search_paths_are_tried_in_order(tmp_path):
    lib = tmp_path / "lib"
    other = tmp_path / "other"
    write(lib, "x.hpp", "// from lib\n")
    write(other, "x.hpp", "// from other\n")
    entry = write(tmp_path, "a.hpp", '#include "x.hpp"\n')
    found = closure(entry, [lib, other])
    assert found.files[0].read_text() == "// from lib\n"


def test_label_drops_the_search_path_prefix(tmp_path):
    lib = tmp_path / "lib"
    header = write(lib, "mylib/data_structure/SegmentTree.hpp", "")
    assert label_for(header, [lib]) == "mylib/data_structure/SegmentTree.hpp"


def test_label_does_not_depend_on_how_it_was_reached(tmp_path):
    lib = tmp_path / "lib"
    write(lib, "mylib/leaf.hpp", "")
    write(lib, "mylib/mid.hpp", '#include "leaf.hpp"\n')
    entry = write(tmp_path, "a.hpp", '#include "mylib/mid.hpp"\n')
    found = closure(entry, [lib])
    assert found.labels == ("mylib/leaf.hpp", "mylib/mid.hpp")


def test_unresolved_is_reported_not_raised(tmp_path):
    entry = write(tmp_path, "a.hpp", '#include "missing.hpp"\n')
    found = closure(entry, [tmp_path])
    assert found.labels == ()
    assert found.unresolved == ("missing.hpp",)


def test_labels_are_sorted(tmp_path):
    write(tmp_path, "z.hpp", "")
    write(tmp_path, "m.hpp", "")
    write(tmp_path, "a.hpp", "")
    entry = write(tmp_path, "entry.hpp", '#include "z.hpp"\n#include "m.hpp"\n#include "a.hpp"\n')
    assert closure(entry, [tmp_path]).labels == ("a.hpp", "m.hpp", "z.hpp")


# --- 山括弧で読んでいる手元のヘッダ ------------------------------------------

RAW_TOML = """
id = "{id}"
title = "t"

[harness]
kind = "{kind}"

[testdata]
source = "none"

[compare]
kind = "compile_only"
"""


@pytest.fixture
def simde(tmp_path, monkeypatch):
    """SIMDe の置き場。本物の third_party/simde の代わりに一時ディレクトリを使う。"""
    directory = tmp_path / "simde"
    write(directory, "simde/x86/avx2.h", '#include "../simde-common.h"\n#include <common.hpp>\n')
    write(directory, "simde/simde-common.h", "#include <lib.hpp>\n")
    monkeypatch.setattr(include_mod, "SIMDE_DIR", directory)
    return directory


def make_problem(tmp_path, files, kind="raw"):
    directory = tmp_path / f"tmp-{kind}"
    write(directory, "problem.toml", RAW_TOML.format(id=directory.name, kind=kind))
    for name, text in files.items():
        write(directory, name, text)
    return problem_mod.load(directory)


def angles(problem, search_paths):
    found = include_mod.AngleIncludes().of(problem, search_paths)
    return [(a.path.name, a.line, a.target) for a in found]


def test_an_angle_include_of_a_local_header_is_found(tmp_path, simde):
    problem = make_problem(tmp_path, {
        "common.hpp": "int x;\n",
        "submissions/a.cpp": "#include <vector>\n#include <common.hpp>\nint main() {}\n",
    })
    assert angles(problem, [problem.dir, simde]) == [("a.cpp", 2, "common.hpp")]


def test_quoted_and_system_includes_are_fine(tmp_path, simde):
    problem = make_problem(tmp_path, {
        "common.hpp": "int x;\n",
        "submissions/a.cpp": '#include <vector>\n#include "common.hpp"\n// #include <common.hpp>\n',
    })
    assert angles(problem, [problem.dir, simde]) == []


def test_headers_in_the_closure_are_looked_at(tmp_path, simde):
    # 提出が引用符で読んだライブラリのヘッダが、別のヘッダを山括弧で読んでいる。
    lib = tmp_path / "lib"
    write(lib, "mylib/a.hpp", "#include <mylib/b.hpp>\n")
    write(lib, "mylib/b.hpp", "int b;\n")
    problem = make_problem(tmp_path, {"submissions/s.cpp": '#include "mylib/a.hpp"\n'})
    assert angles(problem, [lib, problem.dir, simde]) == [("a.hpp", 1, "mylib/b.hpp")]


def test_the_harness_is_looked_at(tmp_path, simde):
    problem = make_problem(tmp_path, {
        "common.hpp": "int x;\n",
        "base.cpp": "#include <common.hpp>\n#include SUBMISSION_HPP\nint main() {}\n",
        "submissions/s.hpp": "int s;\n",
    }, kind="base")
    assert angles(problem, [problem.dir, simde]) == [("base.cpp", 1, "common.hpp")]


def test_simde_is_read_with_angle_brackets_by_design(tmp_path, simde):
    # SIMDe の中で見つかるものは指摘しない。SIMDe のファイルの中の include も見ない。
    problem = make_problem(tmp_path, {
        "common.hpp": "int x;\n",
        "lib.hpp": "int y;\n",
        "submissions/a.cpp": '#include <simde/x86/avx2.h>\n#include "simde/x86/avx2.h"\n',
    })
    assert angles(problem, [problem.dir, simde]) == []


def test_problems_check_warns_without_failing(tmp_path, simde, monkeypatch, capsys):
    problem = make_problem(tmp_path, {
        "common.hpp": "int x;\n",
        "submissions/a.cpp": "#include <common.hpp>\nint main() {}\n",
    })
    monkeypatch.setattr(cli.problem_mod, "all_problem_dirs", lambda: [problem.dir])
    assert cli.main(["problems", "check"]) == 0
    out = capsys.readouterr().out
    assert f"WARN {problem.id}: submissions/a.cpp:1 が手元のヘッダ <common.hpp> を山括弧で" in out
    assert f"OK  {problem.id}" in out
