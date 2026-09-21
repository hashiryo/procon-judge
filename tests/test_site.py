"""サイト生成。記録の畳み方と、書き出したファイルの形。"""

from __future__ import annotations

import json

import pytest

from pj import build as build_mod
from pj import environment as env_mod
from pj import key as key_mod
from pj import problem as problem_mod
from pj.freshness import Freshness
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
    out = tmp_path / "site"
    site_build.build(store_with(tmp_path, [rec(judge_sha="abc123")]), out)
    data = json.loads((out / "data" / "problems" / "p.json").read_text())
    assert data["rows"][0]["judge_sha"] == "abc123"


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


# --- 現行 / 参考 の判定 ----------------------------------------------------

FRESH_TOML = """
id = "tmp-fresh"
title = "t"

[harness]
kind = "raw"

[testdata]
source = "none"

[compare]
kind = "compile_only"
"""

SOURCE = "int main() { return 0; }\n"


@pytest.fixture
def envs():
    return env_mod.load_all()


def fresh_problem(tmp_path):
    directory = tmp_path / "tmp-fresh"
    directory.mkdir()
    (directory / "problem.toml").write_text(FRESH_TOML)
    (directory / "submissions").mkdir()
    (directory / "submissions" / "a.cpp").write_text(SOURCE)
    return problem_mod.load(directory)


def rewrite(problem, text):
    (problem.dir / "submissions" / "a.cpp").write_text(text)


def record_for(problem, env, submission="submissions/a.cpp"):
    """今のソースをその環境で測ったことにした記録。"""
    search = build_mod.include_dirs(problem)
    cxxflags = build_mod.effective_cxxflags(env, problem)
    sub = key_mod.submission_hash(problem.dir / submission, search)
    key = key_mod.compute(
        submission=submission,
        submission_hash=sub.submission_hash,
        harness_hash=key_mod.harness_hash(problem, search),
        problem_hash=key_mod.problem_hash(problem),
        cases_hash="h",
        env=env.name,
        compiler_version="g++-15",
        cxxflags=cxxflags,
        cpu_model="EPYC",
    )
    return rec(key=key, submission=submission, env=env.name, cxxflags=cxxflags)


def test_a_record_of_the_current_source_is_current(tmp_path, envs):
    problem = fresh_problem(tmp_path)
    record = record_for(problem, envs[0])
    assert Freshness(problem, envs).current(record) is True


def test_editing_the_submission_makes_the_record_stale(tmp_path, envs):
    problem = fresh_problem(tmp_path)
    record = record_for(problem, envs[0])
    rewrite(problem, "int main() { return 1; }\n")
    assert Freshness(problem, envs).current(record) is False


def test_reformatting_keeps_the_record_current(tmp_path, envs):
    problem = fresh_problem(tmp_path)
    record = record_for(problem, envs[0])
    rewrite(problem, "\nint main() { return 0; }   \n\n")
    assert Freshness(problem, envs).current(record) is True


def test_an_unresolvable_include_cannot_be_judged(tmp_path, envs):
    """lib/ を取っていない回に、表が丸ごと参考になっては困る。"""
    problem = fresh_problem(tmp_path)
    record = record_for(problem, envs[0])
    rewrite(problem, '#include "nowhere/missing.hpp"\n' + SOURCE)
    assert Freshness(problem, envs).current(record) is None


def test_a_deleted_submission_cannot_be_judged(tmp_path, envs):
    problem = fresh_problem(tmp_path)
    record = record_for(problem, envs[0])
    (problem.dir / "submissions" / "a.cpp").unlink()
    assert Freshness(problem, envs).current(record) is None


def test_a_record_from_an_unknown_env_cannot_be_judged(tmp_path, envs):
    problem = fresh_problem(tmp_path)
    record = record_for(problem, envs[0])
    assert Freshness(problem, envs).current({**record, "env": "gone"}) is None


def test_collapse_marks_the_cell(tmp_path, envs):
    problem = fresh_problem(tmp_path)
    record = record_for(problem, envs[0])
    assert site_build.collapse([record], Freshness(problem, envs))[0].current is True
    rewrite(problem, "int main() { return 1; }\n")
    assert site_build.collapse([record], Freshness(problem, envs))[0].current is False


def test_collapse_without_a_judge_leaves_the_cell_unknown():
    assert site_build.collapse([rec()])[0].current is None


def test_collapse_prefers_the_current_key_over_the_newest(tmp_path, envs):
    """書き換えたものを元に戻すと、現行のキーの記録が最新ではなくなる。"""
    problem = fresh_problem(tmp_path)
    before = record_for(problem, envs[0])
    rewrite(problem, "int main() { return 1; }\n")
    after = record_for(problem, envs[0])
    after["timestamp"] = "2026-02-01T00:00:00Z"
    after["algo_time_max_ns"] = 9999
    rewrite(problem, SOURCE)

    cells = site_build.collapse([before, after], Freshness(problem, envs))
    assert len(cells) == 1
    assert cells[0].current is True
    assert cells[0].algo_ns == 1000


# --- 提出ページ ------------------------------------------------------------


def test_submission_page_is_the_leaderboard_transposed(tmp_path, no_problem_dirs):
    """1 提出を固定して (環境, CPU モデル) を並べる。記録の無い CI の環境も行に出す。"""
    out = tmp_path / "site"
    site_build.build(
        store_with(
            tmp_path,
            [rec(), rec(env="arm-gcc", cpu_model="Neoverse-N2", status="TLE")],
        ),
        out,
    )
    page = (out / "submissions" / "p" / "a.html").read_text()
    assert "EPYC" in page and "Neoverse-N2" in page
    assert "st-AC" in page and "st-TLE" in page
    assert "未計測" in page  # x64-clang と arm-clang には記録が無い
    assert "{{" not in page


def test_submission_page_name_drops_the_prefix_and_the_suffix():
    assert site_build.submission_page("p", "submissions/lib-a.hpp") == "submissions/p/lib-a.html"
    assert site_build.submission_page("p", "submissions/x/y.cpp") == "submissions/p/x/y.html"
    assert site_build.submission_page("p", "submissions/../evil.hpp") is None
    assert site_build.submission_page("../p", "submissions/a.hpp") is None


def test_problem_payload_maps_submissions_to_their_pages(tmp_path, no_problem_dirs):
    out = tmp_path / "site"
    site_build.build(store_with(tmp_path, [rec()]), out)
    data = json.loads((out / "data" / "problems" / "p.json").read_text())
    assert data["pages"] == {"submissions/a.hpp": "submissions/p/a.html"}


def test_render_tolerates_braces_in_the_values():
    """提出のソースには {{1}} のような並びが普通に出る。埋めた値は見ない。"""
    page = site_build._render(
        "submission.html",
        {
            "TITLE": "t", "STYLE_V": "", "PROBLEM_HTML": "p.html", "PROBLEM_TITLE": "P",
            "NAME": "a", "SUBTITLE": "", "META": "", "ROWS": "", "INCLUDES": "",
            "SOURCE": "<pre>int x{{1}};</pre>", "GENERATED": "",
        },
    )
    assert "int x{{1}};" in page


def test_render_rejects_an_unknown_value():
    with pytest.raises(site_build.SiteError):
        site_build._render("index.html", {"DATA_V": "", "STYLE_V": "", "SCRIPT_V": "", "X": ""})


def test_describe_diff_names_files_and_settings():
    from pj.freshness import Diff

    assert site_build.describe_diff(None) is None
    assert site_build.describe_diff(Diff()) == "理由は記録に無い"
    text = site_build.describe_diff(
        Diff(changed=("a.hpp",), added=("b.hpp",), removed=("c.hpp",), settings=("problem",))
    )
    assert text == "変更 a.hpp / 追加 b.hpp / 削除 c.hpp / problem.toml が変わった"


def test_problem_url_knows_the_judges(tmp_path):
    def with_source(source, name):
        directory = tmp_path / f"q-{name.replace('/', '-')}"
        directory.mkdir()
        (directory / "problem.toml").write_text(
            FRESH_TOML.replace('source = "none"', f'source = "{source}"\nname = "{name}"')
            .replace('id = "tmp-fresh"', f'id = "{directory.name}"')
        )
        (directory / "submissions").mkdir()
        return problem_mod.load(directory)

    assert (
        site_build.problem_url(with_source("library_checker", "tree/lca"))
        == "https://judge.yosupo.jp/problem/lca"
    )
    assert (
        site_build.problem_url(with_source("aoj", "DSL_2_B"))
        == "https://onlinejudge.u-aizu.ac.jp/problems/DSL_2_B"
    )
    assert (
        site_build.problem_url(with_source("yukicoder", "274"))
        == "https://yukicoder.me/problems/no/274"
    )
    assert site_build.problem_url(None) is None


# --- include の欄と、ヘッダごとの逆引き ---------------------------------------


@pytest.fixture
def fake_library(tmp_path, monkeypatch):
    """tmp の lib/ を探索パスの先頭に置き、その接頭辞をライブラリとして登録する。"""
    lib = tmp_path / "lib"
    (lib / "mylib" / "internal").mkdir(parents=True)
    (lib / "mylib" / "internal" / "helper.hpp").write_text("int helper();\n")
    (lib / "mylib" / "Tree.hpp").write_text(
        '#include "mylib/internal/helper.hpp"\nstruct Tree { int n = helper(); };\n'
    )
    from pj import libraries as lib_mod
    from pj.paths import HARNESS_DIR, SIMDE_DIR

    monkeypatch.setattr(
        build_mod, "include_dirs", lambda problem: [lib, problem.dir, HARNESS_DIR, SIMDE_DIR]
    )
    monkeypatch.setattr(
        lib_mod,
        "load_all",
        lambda path=None: [
            lib_mod.Library(
                name="Library",
                prefix="mylib/",
                page="https://lib.invalid/{stem}.html",
                source="https://github.com/x/Library/blob/{sha}/mylib/{path}",
            )
        ],
    )
    return lib


def lib_problem(tmp_path):
    directory = tmp_path / "tmp-lib"
    directory.mkdir()
    (directory / "problem.toml").write_text(FRESH_TOML.replace("tmp-fresh", "tmp-lib"))
    (directory / "common.hpp").write_text("int shared();\n")
    (directory / "submissions").mkdir()
    (directory / "submissions" / "lib-tree.cpp").write_text(
        '#include "common.hpp"\n#include "mylib/Tree.hpp"\nint main() { Tree t; return t.n; }\n'
    )
    (directory / "submissions" / "plain.cpp").write_text("int main() { return 0; }\n")
    return problem_mod.load(directory)


def test_include_links_split_direct_from_via(tmp_path, fake_library):
    problem = lib_problem(tmp_path)
    from pj import libraries as lib_mod

    links = site_build.include_links(
        problem, "submissions/lib-tree.cpp", lib_mod.load_all(),
        repo="https://github.com/x/judge", sha="abc", lib_sha="lib123",
    )
    by_label = {link.label: link for link in links}
    assert set(by_label) == {"common.hpp", "mylib/Tree.hpp", "mylib/internal/helper.hpp"}
    assert by_label["mylib/Tree.hpp"].direct is True
    assert by_label["mylib/internal/helper.hpp"].direct is False
    assert by_label["mylib/Tree.hpp"].href == "https://lib.invalid/Tree.html"
    assert by_label["mylib/Tree.hpp"].source_href.endswith("/blob/lib123/mylib/Tree.hpp")
    assert by_label["mylib/Tree.hpp"].library == "Library"
    # このリポジトリのファイルは GitHub の blob へ。
    assert by_label["common.hpp"].library is None
    assert by_label["common.hpp"].href is None or "/blob/abc/" in by_label["common.hpp"].href


def test_include_links_report_unresolved_includes(tmp_path, fake_library):
    problem = lib_problem(tmp_path)
    (problem.dir / "submissions" / "plain.cpp").write_text('#include "nowhere.hpp"\n')
    links = site_build.include_links(
        problem, "submissions/plain.cpp", [], repo=None, sha=None, lib_sha=None
    )
    assert [(l.label, l.missing) for l in links] == [("nowhere.hpp", True)]


def test_include_links_are_none_without_the_file(tmp_path, fake_library):
    problem = lib_problem(tmp_path)
    assert (
        site_build.include_links(
            problem, "submissions/gone.cpp", [], repo=None, sha=None, lib_sha=None
        )
        is None
    )


def test_build_writes_one_json_per_library_header(tmp_path, fake_library, monkeypatch):
    problem = lib_problem(tmp_path)
    monkeypatch.setattr(problem_mod, "all_problem_dirs", lambda: [problem.dir])
    store = store_with(
        tmp_path,
        [rec(problem="tmp-lib", submission="submissions/lib-tree.cpp")],
        problem_id="tmp-lib",
    )
    out = tmp_path / "site"
    summary = site_build.build(store, out)

    assert summary.headers == 2
    assert summary.submission_pages == 2
    tree = json.loads((out / "data" / "headers" / "mylib" / "Tree.hpp.json").read_text())
    helper = json.loads(
        (out / "data" / "headers" / "mylib" / "internal" / "helper.hpp.json").read_text()
    )
    # 問題ごとの common.hpp は名前が問題をまたいで衝突するので、逆引きは出さない。
    assert not (out / "data" / "headers" / "common.hpp.json").exists()

    (entry,) = tree["submissions"]
    assert entry["problem"] == "tmp-lib"
    assert entry["submission"] == "submissions/lib-tree.cpp"
    assert entry["direct"] is True
    assert entry["page"] == "submissions/tmp-lib/lib-tree.html"
    assert helper["submissions"][0]["direct"] is False
    assert tree["environments"] == [e.name for e in env_mod.load_all() if e.runs_on != "self"]

    page = (out / "submissions" / "tmp-lib" / "lib-tree.html").read_text()
    assert "https://lib.invalid/Tree.html" in page
    assert "直接 include しているもの" in page and "間接" in page
    assert "Tree t;" in page  # ソースを埋め込む


def test_env_summary_folds_models_into_environments():
    cells = [
        rec_cell(env="x64-gcc", cpu_model="A", status="AC", current=True),
        rec_cell(env="x64-gcc", cpu_model="B", status="TLE", current=False),
        rec_cell(env="arm-gcc", cpu_model="N2", status="AC", current=True),
    ]
    summary = {row["env"]: row for row in site_build.env_summary(cells, ["x64-gcc", "arm-gcc", "arm-clang"])}
    assert summary["x64-gcc"]["status"] == "TLE"
    assert summary["x64-gcc"]["current"] is False
    assert summary["x64-gcc"]["models"] == 2
    assert summary["arm-gcc"] == {"env": "arm-gcc", "status": "AC", "current": True, "models": 1, "algo_ns": 1000}
    assert summary["arm-clang"]["status"] is None


def rec_cell(**over):
    """collapse を通した Cell に current だけ上書きしたもの。"""
    from dataclasses import replace

    current = over.pop("current", None)
    return replace(site_build.collapse([rec(**over)])[0], current=current)


# --- 問題一覧 --------------------------------------------------------------


def test_origin_is_the_known_prefix_or_own():
    assert site_build.origin_of("yosupo-lca") == "yosupo"
    assert site_build.origin_of("aoj-DSL_2_B") == "aoj"
    assert site_build.origin_of("yuki-274") == "yuki"
    assert site_build.origin_of("gf2-64") == "自作"
    assert site_build.origin_of("warshall-floyd") == "自作"


def test_index_counts_problems_per_origin(tmp_path, no_problem_dirs):
    store = Store(tmp_path / "results")
    for pid in ("yosupo-a", "yosupo-b", "aoj-1", "gf2-64"):
        store.append_raw(pid, json.dumps(rec(problem=pid)))
    out = tmp_path / "site"
    site_build.build(store, out)
    index = json.loads((out / "data" / "index.json").read_text())
    assert index["origins"] == {"yosupo": 2, "aoj": 1, "自作": 1}
    page = (out / "index.html").read_text()
    assert 'id="filter"' in page
