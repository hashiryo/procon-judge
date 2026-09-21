"""library-checker-problems の pin。同じコミットから同じケースが出ることが前提。"""

import subprocess

import pytest

from pj.fetch import FetchError, library_checker


def test_pinned_commit_is_read_from_the_toml(tmp_path):
    toml = tmp_path / "testdata.toml"
    toml.write_text('[library_checker]\ncommit = "' + "a" * 40 + '"\n')
    assert library_checker.pinned_commit(toml) == "a" * 40


def test_a_short_commit_is_refused(tmp_path):
    toml = tmp_path / "testdata.toml"
    toml.write_text('[library_checker]\ncommit = "1814c4e"\n')
    with pytest.raises(FetchError, match="40 桁"):
        library_checker.pinned_commit(toml)


def test_a_missing_toml_is_an_error(tmp_path):
    with pytest.raises(FetchError):
        library_checker.pinned_commit(tmp_path / "nope.toml")


def test_is_current_compares_the_upstream_commit(monkeypatch):
    monkeypatch.setattr(library_checker, "pinned_commit", lambda: "b" * 40)
    assert library_checker.is_current({library_checker.UPSTREAM_KEY: "b" * 40})
    assert not library_checker.is_current({library_checker.UPSTREAM_KEY: "a" * 40})
    # 保管庫にあった古いアセットには項目そのものが無い。作り直す側に倒す。
    assert not library_checker.is_current({})


def test_problem_dir_fills_in_the_category(tmp_path):
    (tmp_path / "data_structure" / "unionfind").mkdir(parents=True)
    (tmp_path / "data_structure" / "unionfind" / "info.toml").write_text("title = 'U'\n")
    found = library_checker.problem_dir("unionfind", tmp_path)
    assert found == tmp_path / "data_structure" / "unionfind"
    assert library_checker.problem_dir("data_structure/unionfind", tmp_path) == found
    assert library_checker.problem_dir("nope", tmp_path) is None
    assert library_checker.problem_dir("graph/unionfind", tmp_path) is None


def _git(directory, *args):
    return subprocess.run(
        ["git", "-C", str(directory), *args], check=True, capture_output=True, text=True,
    ).stdout.strip()


@pytest.fixture
def upstream(tmp_path):
    """2 コミット持つ偽の上流。名指しの fetch を受け付ける設定にしておく。"""
    origin = tmp_path / "origin"
    origin.mkdir()
    _git(origin, "init", "-q")
    _git(origin, "config", "user.email", "t@example.com")
    _git(origin, "config", "user.name", "t")
    _git(origin, "config", "uploadpack.allowReachableSHA1InWant", "true")
    (origin / "generate.py").write_text("v1\n")
    _git(origin, "add", "-A")
    _git(origin, "commit", "-q", "-m", "one")
    first = _git(origin, "rev-parse", "HEAD")
    (origin / "generate.py").write_text("v2\n")
    _git(origin, "commit", "-q", "-am", "two")
    second = _git(origin, "rev-parse", "HEAD")
    return origin, first, second


def test_ensure_repo_checks_out_the_pin_and_moves_with_it(tmp_path, upstream, monkeypatch):
    origin, first, second = upstream
    monkeypatch.setattr(library_checker, "REPO_URL", str(origin))
    clone = tmp_path / "clone"

    library_checker.ensure_repo(clone, first)
    assert library_checker.head(clone) == first
    assert (clone / "generate.py").read_text() == "v1\n"

    # pin を動かしたら、既にある clone を合わせる。手元の変更は捨てる。
    (clone / "generate.py").write_text("edited\n")
    library_checker.ensure_repo(clone, second)
    assert library_checker.head(clone) == second
    assert (clone / "generate.py").read_text() == "v2\n"

    # 合っていれば何もしない (ネットワークに触らない)。
    monkeypatch.setattr(library_checker, "REPO_URL", "file:///nowhere")
    assert library_checker.ensure_repo(clone, second) == clone


def test_ensure_repo_fails_when_the_pin_cannot_be_fetched(tmp_path, upstream, monkeypatch):
    origin, _, _ = upstream
    monkeypatch.setattr(library_checker, "REPO_URL", str(origin))
    with pytest.raises(FetchError):
        library_checker.ensure_repo(tmp_path / "clone", "c" * 40)
