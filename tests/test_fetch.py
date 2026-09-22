"""テストデータの置き場と、キャッシュを再利用してよいかの判定。

置き場は取得元と名前だけで決まり、中身は関係しない。ここを内容由来だと
読み違えると、キャッシュが消えただけで記録が測り直しになると思い込む。
実際に起きるのは再取得と再生成だけで、同じ内容が出ればキーは動かない。
"""

from __future__ import annotations

import json

import pytest

from pj import fetch
from pj import problem as problem_mod
from pj.paths import TESTCASE_CACHE_DIR

TOML = """
id = "{id}"
title = "t"

[harness]
kind = "raw"

[testdata]
source = "{source}"
name = "{name}"

[compare]
kind = "tokens"
"""

LOCAL_TOML = """
id = "{id}"
title = "t"

[harness]
kind = "raw"

[testdata]
source = "local"
count = {count}
generator = "gen.py"
reference = "ref.py"

[compare]
kind = "tokens"
"""


def make(tmp_path, problem_id, source, name):
    directory = tmp_path / problem_id
    directory.mkdir()
    (directory / "problem.toml").write_text(
        TOML.format(id=problem_id, source=source, name=name)
    )
    return problem_mod.load(directory)


def make_local(tmp_path, problem_id, *, count=3, generator="a", reference="b"):
    directory = tmp_path / problem_id
    directory.mkdir()
    (directory / "problem.toml").write_text(
        LOCAL_TOML.format(id=problem_id, count=count)
    )
    (directory / "gen.py").write_text(generator)
    (directory / "ref.py").write_text(reference)
    return problem_mod.load(directory)


def rel(problem):
    return fetch.cache_dir_for(problem).relative_to(TESTCASE_CACHE_DIR).as_posix()


# --- 置き場 ----------------------------------------------------------------


def test_the_directory_is_readable(tmp_path):
    """ログに出る名前なので読める形にする。ハッシュだと cases_hash と紛れる。"""
    assert rel(make(tmp_path, "p1", "aoj", "DSL_2_B")) == "aoj/DSL_2_B"
    assert rel(make(tmp_path, "p2", "yukicoder", "649")) == "yukicoder/649"


def test_the_name_keeps_its_hierarchy(tmp_path):
    """library_checker の name はカテゴリを含む。"""
    problem = make(tmp_path, "p1", "library_checker", "data_structure/point_add_range_sum")
    assert rel(problem) == "library_checker/data_structure/point_add_range_sum"


def test_the_name_cannot_escape_the_cache(tmp_path):
    """判定サイト側の文字列なので素通しにしない。"""
    problem = make(tmp_path, "p1", "aoj", "../../etc/passwd")
    assert rel(problem) == "aoj/etc/passwd"


def test_odd_characters_are_replaced(tmp_path):
    assert rel(make(tmp_path, "p1", "aoj", "a b/c:d")) == "aoj/a_b/c_d"


def test_the_directory_does_not_depend_on_the_contents(tmp_path):
    """取り直して別の内容になっても同じ場所へ上書きする。

    内容で分けるのは cases_hash の役目で、置き場の役目ではない。
    """
    first = make(tmp_path, "p1", "aoj", "DSL_2_B")
    second = make(tmp_path, "p2", "aoj", "DSL_2_B")
    assert fetch.cache_dir_for(first) == fetch.cache_dir_for(second)


def test_a_problem_without_testdata_still_has_a_place(tmp_path):
    """source = "none" は名前を持たない。走らせないが ensure が場所を引く。"""
    directory = tmp_path / "p1"
    directory.mkdir()
    (directory / "problem.toml").write_text(
        TOML.format(id="p1", source="none", name="").replace(
            'kind = "tokens"', 'kind = "compile_only"'
        )
    )
    assert rel(problem_mod.load(directory)) == "none/p1"


def test_local_splits_on_the_generator(tmp_path):
    """同じ問題 id から別の内容が出るので、そこだけ中身で分ける。"""
    base = make_local(tmp_path, "p1", generator="print(1)")
    same = make_local(tmp_path, "p2", generator="print(1)")
    other = make_local(tmp_path, "p3", generator="print(2)")
    assert fetch.cache_dir_for(base).name == fetch.cache_dir_for(same).name
    assert fetch.cache_dir_for(base).name != fetch.cache_dir_for(other).name


def test_local_splits_on_the_harness(tmp_path):
    """kind = "base" の参照実装は base.cpp と一緒に組むので、base.cpp が変われば期待出力も変わる。"""
    def make_base(problem_id, harness):
        directory = tmp_path / problem_id
        directory.mkdir()
        (directory / "problem.toml").write_text(
            LOCAL_TOML.format(id=problem_id, count=3).replace('kind = "raw"', 'kind = "base"')
        )
        (directory / "gen.py").write_text("a")
        (directory / "ref.py").write_text("b")
        (directory / "base.cpp").write_text(harness)
        return problem_mod.load(directory)

    base = make_base("p1", "constexpr int MOD = 7;")
    same = make_base("p2", "constexpr int MOD = 7;")
    other = make_base("p3", "constexpr int MOD = 11;")
    assert fetch.cache_dir_for(base).name == fetch.cache_dir_for(same).name
    assert fetch.cache_dir_for(base).name != fetch.cache_dir_for(other).name


def test_local_splits_on_the_case_count(tmp_path):
    base = make_local(tmp_path, "p1", count=3)
    other = make_local(tmp_path, "p2", count=4)
    assert fetch.cache_dir_for(base).name != fetch.cache_dir_for(other).name


# --- キャッシュを再利用してよいか -------------------------------------------


def cases_in(directory, contents):
    for name, (text_in, text_out) in contents.items():
        (directory / f"{name}.in").write_text(text_in)
        (directory / f"{name}.out").write_text(text_out)
    return fetch.collect_cases(directory)


def test_hidden_files_are_not_cases(tmp_path):
    """macOS の tar が付ける ._ (AppleDouble) を Linux が実ファイルとして展開しても、ケースにしない。"""
    cases = cases_in(tmp_path, {"a": ("1\n", "2\n"), "._a": ("\x00\x05\x16\x07", "\x00\x05\x16\x07")})
    assert [c.name for c in cases] == ["a"]


def test_a_matching_cache_is_reused(tmp_path):
    cases = cases_in(tmp_path, {"a": ("1\n", "2\n"), "b": ("3\n", "4\n")})
    fetch.write_manifest(tmp_path, cases, "aoj", "X")
    recorded = json.loads((tmp_path / fetch.MANIFEST_NAME).read_text())["cases"]
    assert fetch._matches_manifest(cases, recorded) is True


def test_a_changed_case_is_not_reused(tmp_path):
    """数だけ見ていると、manifest の cases_hash で別の中身を走らせてしまう。"""
    cases = cases_in(tmp_path, {"a": ("1\n", "2\n"), "b": ("3\n", "4\n")})
    fetch.write_manifest(tmp_path, cases, "aoj", "X")
    recorded = json.loads((tmp_path / fetch.MANIFEST_NAME).read_text())["cases"]
    (tmp_path / "a.in").write_text("999999\n")
    assert fetch._matches_manifest(fetch.collect_cases(tmp_path), recorded) is False


def test_a_renamed_case_is_not_reused(tmp_path):
    cases = cases_in(tmp_path, {"a": ("1\n", "2\n"), "b": ("3\n", "4\n")})
    fetch.write_manifest(tmp_path, cases, "aoj", "X")
    recorded = json.loads((tmp_path / fetch.MANIFEST_NAME).read_text())["cases"]
    (tmp_path / "a.in").rename(tmp_path / "c.in")
    (tmp_path / "a.out").rename(tmp_path / "c.out")
    assert fetch._matches_manifest(fetch.collect_cases(tmp_path), recorded) is False


def test_a_missing_case_is_not_reused(tmp_path):
    cases = cases_in(tmp_path, {"a": ("1\n", "2\n"), "b": ("3\n", "4\n")})
    fetch.write_manifest(tmp_path, cases, "aoj", "X")
    recorded = json.loads((tmp_path / fetch.MANIFEST_NAME).read_text())["cases"]
    (tmp_path / "b.in").unlink()
    assert fetch._matches_manifest(fetch.collect_cases(tmp_path), recorded) is False


def test_a_manifest_without_sizes_still_passes(tmp_path):
    """古い manifest はサイズを持たないことがある。そこは数だけで通す。"""
    cases = cases_in(tmp_path, {"a": ("1\n", "2\n")})
    assert fetch._matches_manifest(cases, [{"name": "a"}]) is True


# --- cases_hash は経路に依存しない ------------------------------------------


def test_the_cases_hash_is_the_same_wherever_it_came_from(tmp_path):
    """保管庫から取っても原本から取っても同じ値になる必要がある。"""
    first = tmp_path / "from-mirror"
    second = tmp_path / "from-origin"
    for directory in (first, second):
        directory.mkdir()
        cases_in(directory, {"a": ("1\n", "2\n"), "b": ("3\n", "4\n")})
    assert fetch.compute_cases_hash(
        fetch.collect_cases(first)
    ) == fetch.compute_cases_hash(fetch.collect_cases(second))


def test_the_cases_hash_moves_when_the_content_moves(tmp_path):
    before = cases_in(tmp_path, {"a": ("1\n", "2\n")})
    first = fetch.compute_cases_hash(before)
    (tmp_path / "a.in").write_text("5\n")
    assert fetch.compute_cases_hash(fetch.collect_cases(tmp_path)) != first


def test_an_unusable_name_is_refused(tmp_path):
    with pytest.raises(fetch.FetchError):
        fetch.cache_dir_for(make(tmp_path, "p1", "aoj", "///"))


# --- pin と保管庫 -----------------------------------------------------------


@pytest.fixture
def isolated_cache(tmp_path, monkeypatch):
    monkeypatch.setattr(fetch, "TESTCASE_CACHE_DIR", tmp_path / "cache")
    return tmp_path / "cache"


def _fake_generate(commit):
    from pj.fetch import library_checker

    def generate(problem, dest):
        dest.mkdir(parents=True, exist_ok=True)
        (dest / "a.in").write_text("1\n")
        (dest / "a.out").write_text("2\n")
        return {library_checker.UPSTREAM_KEY: commit}

    return generate


def test_the_manifest_carries_extra_fields(tmp_path):
    cases = cases_in(tmp_path, {"a": ("1\n", "2\n")})
    fetch.write_manifest(tmp_path, cases, "library_checker", "x", {"upstream_commit": "abc"})
    manifest = json.loads((tmp_path / fetch.MANIFEST_NAME).read_text())
    assert manifest["upstream_commit"] == "abc"
    assert manifest["cases_hash"]


def test_a_local_cache_from_an_old_pin_is_rebuilt(tmp_path, isolated_cache, monkeypatch):
    """pin を動かしたら、手元にあるものでも作り直す。"""
    from pj.fetch import library_checker, mirror

    monkeypatch.setattr(library_checker, "pinned_commit", lambda: "new")
    monkeypatch.setattr(mirror, "available", lambda: False)
    calls = []

    def generate(problem, dest):
        calls.append(problem.id)
        return _fake_generate("new")(problem, dest)

    monkeypatch.setattr(library_checker, "fetch", generate)
    problem = make(tmp_path, "yosupo-x", "library_checker", "data_structure/x")

    first = fetch.ensure(problem)
    assert calls == ["yosupo-x"]
    manifest = json.loads((first.dir / fetch.MANIFEST_NAME).read_text())
    assert manifest[library_checker.UPSTREAM_KEY] == "new"

    # 同じ pin なら手元のものを使う。
    fetch.ensure(problem)
    assert calls == ["yosupo-x"]

    # pin が動いたら作り直す。
    monkeypatch.setattr(library_checker, "pinned_commit", lambda: "newer")
    monkeypatch.setattr(library_checker, "fetch", lambda p, d: (calls.append(p.id), _fake_generate("newer")(p, d))[1])
    fetch.ensure(problem)
    assert calls == ["yosupo-x", "yosupo-x"]


def test_a_stale_mirror_asset_is_rebuilt_and_replaced(tmp_path, isolated_cache, monkeypatch):
    """保管庫のものが古い pin なら、作り直して置き換える (force)。"""
    from pj.fetch import library_checker, mirror

    monkeypatch.setattr(library_checker, "pinned_commit", lambda: "new")
    monkeypatch.setattr(library_checker, "fetch", _fake_generate("new"))
    monkeypatch.setattr(mirror, "available", lambda: True)

    def pull(problem, dest):
        dest.mkdir(parents=True, exist_ok=True)
        (dest / "a.in").write_text("1\n")
        (dest / "a.out").write_text("2\n")
        (dest / fetch.MANIFEST_NAME).write_text(
            json.dumps({"cases_hash": "x", library_checker.UPSTREAM_KEY: "old"})
        )
        return True

    pushes = []
    monkeypatch.setattr(mirror, "pull", pull)
    monkeypatch.setattr(mirror, "push", lambda p, d, force=False: pushes.append(force))
    problem = make(tmp_path, "yosupo-x", "library_checker", "data_structure/x")

    result = fetch.ensure(problem)
    assert pushes == [True]
    manifest = json.loads((result.dir / fetch.MANIFEST_NAME).read_text())
    assert manifest[library_checker.UPSTREAM_KEY] == "new"


def test_a_current_mirror_asset_is_used_as_is(tmp_path, isolated_cache, monkeypatch):
    from pj.fetch import library_checker, mirror

    monkeypatch.setattr(library_checker, "pinned_commit", lambda: "new")
    monkeypatch.setattr(
        library_checker, "fetch", lambda p, d: pytest.fail("原本を叩いてはいけない")
    )
    monkeypatch.setattr(mirror, "available", lambda: True)

    def pull(problem, dest):
        dest.mkdir(parents=True, exist_ok=True)
        (dest / "a.in").write_text("1\n")
        (dest / "a.out").write_text("2\n")
        (dest / fetch.MANIFEST_NAME).write_text(
            json.dumps({"cases_hash": "x", library_checker.UPSTREAM_KEY: "new"})
        )
        return True

    monkeypatch.setattr(mirror, "pull", pull)
    monkeypatch.setattr(mirror, "push", lambda p, d, force=False: pytest.fail("上げ直さない"))
    problem = make(tmp_path, "yosupo-x", "library_checker", "data_structure/x")

    result = fetch.ensure(problem)
    manifest = json.loads((result.dir / fetch.MANIFEST_NAME).read_text())
    # 書き直した manifest にも上流のコミットが残る。
    assert manifest[library_checker.UPSTREAM_KEY] == "new"
    assert manifest["cases_hash"] == result.cases_hash


def test_other_sources_do_not_care_about_the_pin(tmp_path, isolated_cache, monkeypatch):
    from pj.fetch import mirror

    monkeypatch.setattr(mirror, "available", lambda: False)
    problem = make(tmp_path, "aoj-x", "aoj", "x")
    dest = fetch.cache_dir_for(problem)
    dest.mkdir(parents=True)
    cases = cases_in(dest, {"a": ("1\n", "2\n")})
    fetch.write_manifest(dest, cases, "aoj", "x")
    monkeypatch.setattr(fetch, "_fetch_from_origin", lambda p, d: pytest.fail("取り直さない"))
    assert fetch.ensure(problem).cases_hash == fetch.compute_cases_hash(cases)


def test_evict_removes_the_cache_directory(tmp_path, isolated_cache):
    problem = make(tmp_path, "aoj-x", "aoj", "x")
    dest = fetch.cache_dir_for(problem)
    dest.mkdir(parents=True)
    (dest / "a.in").write_text("1\n")
    fetch.evict(problem)
    assert not dest.exists()
    fetch.evict(problem)  # 無くても平気


def test_aoj_truncation_is_only_when_shorter():
    """header の inputSize は末尾の改行を数えないことがある。長い方は切り詰めではない。"""
    from pj.fetch import aoj

    assert aoj._short(b"1 2\n", 5) is True
    assert aoj._short(b"1 2\n", 4) is False
    assert aoj._short(b"1 2\n", 3) is False
    assert aoj._short(b"1 2\n", None) is False


def test_a_mirror_asset_without_the_checker_is_rebuilt(tmp_path, isolated_cache, monkeypatch):
    """checker を入れずに上げたアセットは、作り直して置き換える。"""
    from pj.fetch import library_checker, mirror

    monkeypatch.setattr(library_checker, "pinned_commit", lambda: "new")

    def generate(problem, dest):
        extra = _fake_generate("new")(problem, dest)
        (dest / "checker.cpp").write_text("int main() {}\n")
        return extra

    monkeypatch.setattr(library_checker, "fetch", generate)
    monkeypatch.setattr(mirror, "available", lambda: True)

    def pull(problem, dest):
        dest.mkdir(parents=True, exist_ok=True)
        (dest / "a.in").write_text("1\n")
        (dest / "a.out").write_text("2\n")
        (dest / fetch.MANIFEST_NAME).write_text(
            json.dumps({"cases_hash": "x", library_checker.UPSTREAM_KEY: "new"})
        )
        return True

    pushes = []
    monkeypatch.setattr(mirror, "pull", pull)
    monkeypatch.setattr(mirror, "push", lambda p, d, force=False: pushes.append(force))
    directory = tmp_path / "yosupo-x"
    directory.mkdir()
    (directory / "problem.toml").write_text(
        TOML.format(id="yosupo-x", source="library_checker", name="data_structure/x").replace(
            'kind = "tokens"', 'kind = "checker"'
        )
    )
    problem = problem_mod.load(directory)

    result = fetch.ensure(problem)
    assert pushes == [True]
    assert result.checker_source() is not None
