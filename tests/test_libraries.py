"""libraries.toml。ラベルの接頭辞から外向きのリンクを作る。"""

import pytest

from pj import libraries as lib_mod

LIB = lib_mod.Library(
    name="Library",
    prefix="mylib/",
    page="https://example.invalid/Library/{stem}.html",
    source="https://github.com/x/Library/blob/{sha}/mylib/{path}",
)


def test_page_drops_the_prefix_and_the_suffix():
    assert (
        LIB.page_url("mylib/data_structure/SegmentTree.hpp")
        == "https://example.invalid/Library/data_structure/SegmentTree.html"
    )


def test_source_keeps_the_path_and_takes_the_sha():
    assert (
        LIB.source_url("mylib/algebra/ModInt.hpp", "abc123")
        == "https://github.com/x/Library/blob/abc123/mylib/algebra/ModInt.hpp"
    )


def test_source_falls_back_to_head_without_a_sha():
    assert LIB.source_url("mylib/a.hpp", None).endswith("/blob/HEAD/mylib/a.hpp")


def test_find_prefers_the_longest_prefix():
    inner = lib_mod.Library(name="inner", prefix="mylib/internal/", page="p", source="s")
    assert lib_mod.find("mylib/internal/x.hpp", [LIB, inner]) is inner
    assert lib_mod.find("mylib/a.hpp", [LIB, inner]) is LIB
    assert lib_mod.find("common.hpp", [LIB, inner]) is None


def test_a_missing_file_means_no_libraries(tmp_path):
    assert lib_mod.load_all(tmp_path / "none.toml") == []


def test_load_reads_the_entries(tmp_path):
    path = tmp_path / "libraries.toml"
    path.write_text(
        '[[library]]\nname = "L"\nprefix = "lib/"\npage = "p/{stem}"\nsource = "s/{path}"\n'
    )
    (lib,) = lib_mod.load_all(path)
    assert (lib.name, lib.prefix) == ("L", "lib/")


def test_load_rejects_a_missing_key(tmp_path):
    path = tmp_path / "libraries.toml"
    path.write_text('[[library]]\nname = "L"\nprefix = "lib/"\n')
    with pytest.raises(lib_mod.LibrariesError):
        lib_mod.load_all(path)


def test_the_real_file_loads():
    assert lib_mod.load_all()
