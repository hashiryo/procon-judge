"""サイト生成。記録の畳み方と、書き出したファイルの形。"""

from __future__ import annotations

import json

import pytest

from pj import problem as problem_mod
from pj.site import build as site_build
from pj.store import Store


def rec(**over):
    base = {
        "key": "k1",
        "problem": "p",
        "submission": "submissions/a.hpp",
        "status": "AC",
        "env": "x64-gcc",
        "cpu_arch": "x86_64",
        "cpu_model": "EPYC",
        "compiler_version": "g++-15",
        "cxxflags": "-O2 -Ilib",
        "cases_hash": "h",
        "case_count": 3,
        "submission_hash": "s",
        "includes": [],
        "library_sha": None,
        "judge_sha": None,
        "time_max_ms": 100,
        "time_total_ms": 200,
        "algo_time_max_ns": 1000,
        "algo_time_total_ns": 2000,
        "memory_max_kb": 1024,
        "source_bytes": 10,
        "binary_bytes": 20,
        "failed_case": None,
        "timestamp": "2026-01-01T00:00:00Z",
    }
    base.update(over)
    return base


def store_with(tmp_path, records, problem_id="p"):
    store = Store(tmp_path / "results")
    for record in records:
        store.append_raw(problem_id, json.dumps(record))
    return store


# --- collapse --------------------------------------------------------------


def test_one_cell_per_submission_env_and_cpu():
    cells = site_build.collapse(
        [
            rec(),
            rec(env="arm-gcc", cpu_model="Neoverse-N2"),
            rec(submission="submissions/b.hpp"),
        ]
    )
    assert len(cells) == 3


def test_only_the_newest_key_counts():
    """ソースを書き換えるとキーが変わる。古い測定を順位に混ぜない。"""
    cells = site_build.collapse(
        [
            rec(key="old", time_max_ms=10, algo_time_max_ns=10, timestamp="2026-01-01T00:00:00Z"),
            rec(key="new", time_max_ms=90, algo_time_max_ns=900, timestamp="2026-02-01T00:00:00Z"),
        ]
    )
    assert len(cells) == 1
    assert cells[0].wall_ms == 90
    assert cells[0].algo_ns == 900
    assert cells[0].samples == 1


def test_the_same_key_takes_the_minimum():
    """同じ条件を何度か測ったら最小値を採る。標本の数も出す。"""
    cells = site_build.collapse(
        [
            rec(time_max_ms=120, algo_time_max_ns=1200, memory_max_kb=2048),
            rec(time_max_ms=100, algo_time_max_ns=900, memory_max_kb=1024,
                timestamp="2026-01-02T00:00:00Z"),
        ]
    )
    assert (cells[0].wall_ms, cells[0].algo_ns, cells[0].rss_kb) == (100, 900, 1024)
    assert cells[0].samples == 2


def test_algo_may_be_missing():
    cells = site_build.collapse([rec(algo_time_max_ns=None)])
    assert cells[0].algo_ns is None


def test_the_failed_case_is_trimmed():
    cells = site_build.collapse(
        [
            rec(
                status="WA",
                failed_case={"name": "random_00", "status": "WA", "time_ms": 1,
                             "memory_kb": 1, "detail": "x" * 900},
            )
        ]
    )
    assert cells[0].failed["name"] == "random_00"
    assert len(cells[0].failed["detail"]) == site_build.DETAIL_CHARS


# --- build -----------------------------------------------------------------


@pytest.fixture
def no_problem_dirs(monkeypatch):
    """記録だけがあって問題の定義が無い状態にする。"""
    monkeypatch.setattr(problem_mod, "all_problem_dirs", list)


def test_build_writes_a_page_and_a_json_per_problem(tmp_path, no_problem_dirs):
    store = store_with(tmp_path, [rec(), rec(submission="submissions/b.hpp")])
    out = tmp_path / "site"
    summary = site_build.build(store, out)

    assert summary.problems == 1
    assert summary.records == 2
    for name in ("index.html", "index.js", "problem.js", "style.css",
                 "data/index.json", "data/problems/p.json", "problems/p.html"):
        assert (out / name).is_file(), name


def test_the_html_points_at_the_hash_of_the_json(tmp_path, no_problem_dirs):
    """Pages のキャッシュを避けるため、参照側の URL に中身のハッシュを混ぜる。"""
    out = tmp_path / "site"
    site_build.build(store_with(tmp_path, [rec()]), out)
    page = (out / "problems" / "p.html").read_text()
    data = (out / "data" / "problems" / "p.json").read_text()

    import hashlib

    digest = hashlib.sha256(data.encode()).hexdigest()[:12]
    assert f"p.json?v={digest}" in page


def test_no_placeholder_is_left_in_the_pages(tmp_path, no_problem_dirs):
    out = tmp_path / "site"
    site_build.build(store_with(tmp_path, [rec()]), out)
    for page in ("index.html", "problems/p.html"):
        assert "{{" not in (out / page).read_text(), page


def test_index_counts_the_holes(tmp_path, no_problem_dirs):
    """提出 2 本 × 組み合わせ 2 つで 4 枠、記録は 3 つなので 1 つ空く。"""
    store = store_with(
        tmp_path,
        [
            rec(),
            rec(submission="submissions/b.hpp"),
            rec(env="arm-gcc", cpu_model="Neoverse-N2"),
        ],
    )
    out = tmp_path / "site"
    site_build.build(store, out)
    index = json.loads((out / "data" / "index.json").read_text())
    row = index["problems"][0]
    assert (row["submissions"], row["measured"], row["pending"]) == (2, 3, 1)


def test_build_does_not_wipe_a_directory_it_did_not_make(tmp_path, no_problem_dirs):
    out = tmp_path / "somewhere"
    out.mkdir()
    (out / "大事なもの.txt").write_text("消さないで")
    with pytest.raises(site_build.SiteError):
        site_build.build(store_with(tmp_path, [rec()]), out)
    assert (out / "大事なもの.txt").is_file()


def test_build_replaces_its_own_previous_output(tmp_path, no_problem_dirs):
    out = tmp_path / "site"
    site_build.build(store_with(tmp_path, [rec()]), out)
    stale = out / "problems" / "gone.html"
    stale.write_text("古いページ")
    site_build.build(store_with(tmp_path, [rec()]), out)
    assert not stale.exists()


def test_ids_that_would_escape_the_directory_are_skipped(tmp_path, no_problem_dirs):
    store = store_with(tmp_path, [rec(problem="../evil")], problem_id="../evil")
    out = tmp_path / "site"
    summary = site_build.build(store, out)
    assert summary.problems == 0


# --- ソースへのリンク ------------------------------------------------------


def test_rows_carry_the_commit_that_was_measured(tmp_path, no_problem_dirs):
    """リンク先はいまの main ではなく、その数字を出したコミット。"""
    store = store_with(tmp_path, [rec(judge_sha="abc123", library_sha="def456")])
    out = tmp_path / "site"
    site_build.build(store, out, library_url="https://example.invalid/lib")
    data = json.loads((out / "data" / "problems" / "p.json").read_text())
    assert data["rows"][0]["judge_sha"] == "abc123"
    assert data["rows"][0]["library_sha"] == "def456"
    assert data["library"] == "https://example.invalid/lib"


def test_without_a_library_url_there_is_no_link(tmp_path, no_problem_dirs):
    """このリポジトリはライブラリの在処を持たない。渡されなければ出さない。"""
    out = tmp_path / "site"
    site_build.build(store_with(tmp_path, [rec()]), out)
    data = json.loads((out / "data" / "problems" / "p.json").read_text())
    assert data["library"] is None


def test_repo_url_comes_from_the_ci_environment(monkeypatch):
    monkeypatch.setenv("GITHUB_REPOSITORY", "owner/repo")
    monkeypatch.delenv("GITHUB_SERVER_URL", raising=False)
    assert site_build.repo_url() == "https://github.com/owner/repo"


def make_repo(tmp_path, remote):
    import subprocess

    subprocess.run(["git", "init", "-q"], cwd=tmp_path, check=True)
    subprocess.run(["git", "remote", "add", "origin", remote], cwd=tmp_path, check=True)
    return tmp_path


def test_repo_url_survives_an_ssh_alias(tmp_path, monkeypatch):
    """remote の host は別名が挟まるので当てにしない。owner/repo だけ拾う。"""
    monkeypatch.delenv("GITHUB_REPOSITORY", raising=False)
    root = make_repo(tmp_path, "git@github.com.hashiryo:hashiryo/procon-judge.git")
    assert site_build.repo_url(root) == "https://github.com/hashiryo/procon-judge"


def test_repo_url_is_none_when_the_remote_is_not_github(tmp_path, monkeypatch):
    monkeypatch.delenv("GITHUB_REPOSITORY", raising=False)
    root = make_repo(tmp_path, "git@ghe.example.invalid:team/thing.git")
    assert site_build.repo_url(root) is None
