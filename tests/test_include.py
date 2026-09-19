"""include 閉包の解決。欠けるとキーを誤らせるので固めておく。"""

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
