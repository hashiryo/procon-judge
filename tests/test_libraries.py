"""libraries.toml。ラベルの接頭辞から外向きのリンクを作り、ライブラリの置き場を決める。"""

import json
import subprocess

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


# --- 置き場と取得 (dir と repo) ------------------------------------------------

NEO_TOML = (
    '[[library]]\nname = "NeoLibrary"\nprefix = "neo/"\npage = "p/{stem}"\nsource = "s/{path}"\n'
    'dir = "neolib"\nrepo = "https://example.invalid/NeoLibrary.git"\n'
)


def test_load_reads_the_dir_and_the_repo(tmp_path):
    path = tmp_path / "libraries.toml"
    path.write_text(NEO_TOML)
    (lib,) = lib_mod.load_all(path)
    assert (lib.dir, lib.repo) == ("neolib", "https://example.invalid/NeoLibrary.git")
    assert lib.path == lib_mod.ROOT / "neolib"


def test_an_entry_without_a_dir_only_links(tmp_path):
    path = tmp_path / "libraries.toml"
    path.write_text(
        '[[library]]\nname = "L"\nprefix = "lib/"\npage = "p/{stem}"\nsource = "s/{path}"\n'
    )
    (lib,) = lib_mod.load_all(path)
    assert lib.path is None and lib.repo is None


@pytest.mark.parametrize("directory", ["", "a/b", ".hidden", "../up"])
def test_load_keeps_the_dir_right_under_the_root(tmp_path, directory):
    path = tmp_path / "libraries.toml"
    path.write_text(NEO_TOML.replace('dir = "neolib"', f'dir = "{directory}"'))
    with pytest.raises(lib_mod.LibrariesError):
        lib_mod.load_all(path)


def test_load_rejects_a_repo_without_a_dir(tmp_path):
    path = tmp_path / "libraries.toml"
    path.write_text(NEO_TOML.replace('dir = "neolib"\n', ""))
    with pytest.raises(lib_mod.LibrariesError):
        lib_mod.load_all(path)


def test_load_rejects_two_entries_in_one_dir(tmp_path):
    path = tmp_path / "libraries.toml"
    path.write_text(NEO_TOML + NEO_TOML.replace('name = "NeoLibrary"', 'name = "Other"'))
    with pytest.raises(lib_mod.LibrariesError):
        lib_mod.load_all(path)


def test_the_real_file_keeps_library_in_lib():
    """lib は探索パスの先頭で、キーの材料にも -Ilib のまま入っている。動かせない。"""
    (library,) = [lib for lib in lib_mod.load_all() if lib.name == "Library"]
    assert library.path == lib_mod.LIB_DIR
    assert library.repo == "https://github.com/hashiryo/Library.git"


def test_extra_dirs_leave_out_lib(monkeypatch):
    library = lib_mod.Library(
        name="Library", prefix="mylib/", page="p", source="s", dir="lib", repo="r"
    )
    neo = lib_mod.Library(name="N", prefix="neo/", page="p", source="s", dir="neolib", repo="r")
    link_only = lib_mod.Library(name="A", prefix="atcoder/", page="p", source="s")
    monkeypatch.setattr(lib_mod, "load_all", lambda path=None: [library, link_only, neo])
    assert lib_mod.extra_dirs() == (lib_mod.ROOT / "neolib",)


def _git(cwd, *args):
    subprocess.run(["git", "-C", str(cwd), *args], check=True, capture_output=True)


def _commit(repo, name, text):
    (repo / name).write_text(text)
    _git(repo, "add", name)
    _git(repo, "-c", "user.name=t", "-c", "user.email=t@t", "commit", "-q", "-m", name)
    return subprocess.run(
        ["git", "-C", str(repo), "rev-parse", "HEAD"], check=True, capture_output=True, text=True
    ).stdout.strip()


@pytest.fixture
def origin(tmp_path):
    """取得元の代わりの手元のリポジトリ。commit を 2 つ持つ。"""
    repo = tmp_path / "origin"
    repo.mkdir()
    _git(repo, "init", "-q", "-b", "main")
    # 先端でない commit を名指しで取るため。GitHub は到達できる commit なら取らせる。
    _git(repo, "config", "uploadpack.allowAnySHA1InWant", "true")
    first = _commit(repo, "a.hpp", "int a;\n")
    second = _commit(repo, "a.hpp", "int a, b;\n")
    return repo, first, second


@pytest.fixture
def judge_root(tmp_path, monkeypatch):
    """置き場の基準 (リポジトリのルート) を tmp に移す。本物のルートに clone しない。"""
    root = tmp_path / "judge"
    root.mkdir()
    monkeypatch.setattr(lib_mod, "ROOT", root)
    return root


def _neo(origin_repo):
    return lib_mod.Library(
        name="NeoLibrary", prefix="neo/", page="p", source="s",
        dir="neolib", repo=f"file://{origin_repo}",
    )


def test_fetch_takes_the_tip_by_default(origin, judge_root):
    repo, _, second = origin
    assert lib_mod.fetch(_neo(repo)) == second
    assert (judge_root / "neolib" / "a.hpp").read_text() == "int a, b;\n"


def test_fetch_takes_the_commit_it_is_given(origin, judge_root):
    repo, first, _ = origin
    assert lib_mod.fetch(_neo(repo), first) == first
    assert (judge_root / "neolib" / "a.hpp").read_text() == "int a;\n"


def test_fetch_leaves_an_existing_dir_alone(origin, judge_root):
    """手元の置き場は本人の作業ツリーへのリンクのことがある。書き換えない。"""
    repo, first, _ = origin
    mine = judge_root.parent / "mine"
    mine.mkdir()
    _git(mine, "init", "-q")
    own = _commit(mine, "x.hpp", "int x;\n")
    (judge_root / "neolib").symlink_to(mine)
    assert lib_mod.fetch(_neo(repo), first) == own
    assert not (mine / "a.hpp").exists()


def test_a_failed_fetch_leaves_no_dir(tmp_path, judge_root):
    with pytest.raises(lib_mod.LibrariesError):
        lib_mod.fetch(_neo(tmp_path / "nowhere"))
    assert not (judge_root / "neolib").exists()


def test_fetch_all_reports_what_it_could_not_take(tmp_path, origin, judge_root):
    repo, _, second = origin
    gone = lib_mod.Library(
        name="Gone", prefix="gone/", page="p", source="s",
        dir="gone", repo=f"file://{tmp_path / 'nowhere'}",
    )
    link_only = lib_mod.Library(name="A", prefix="atcoder/", page="p", source="s")
    fetched, failed = lib_mod.fetch_all([_neo(repo), gone, link_only])
    assert fetched == {"NeoLibrary": second}
    assert list(failed) == ["Gone"]


def test_libs_fetch_writes_the_shas_for_the_later_jobs(tmp_path, origin, judge_root, monkeypatch):
    from pj import cli

    repo, first, _ = origin
    monkeypatch.setattr(lib_mod, "load_all", lambda path=None: [_neo(repo)])
    out = tmp_path / "github_output"
    assert cli.main(["libs", "fetch", "--shas", json.dumps({"NeoLibrary": first}),
                     "--github-output", str(out)]) == 0
    assert out.read_text() == f'library_shas={json.dumps({"NeoLibrary": first})}\n'


def test_libs_fetch_fails_only_when_strict(tmp_path, judge_root, monkeypatch):
    from pj import cli

    gone = lib_mod.Library(
        name="Gone", prefix="gone/", page="p", source="s",
        dir="gone", repo=f"file://{tmp_path / 'nowhere'}",
    )
    monkeypatch.setattr(lib_mod, "load_all", lambda path=None: [gone])
    assert cli.main(["libs", "fetch"]) == 0
    assert cli.main(["libs", "fetch", "--strict"]) == 1
