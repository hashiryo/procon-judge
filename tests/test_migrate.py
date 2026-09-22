"""competitive-verifier のテストを raw の問題に直す取り込み。"""

from pathlib import Path

import pytest

from pj import migrate

HEADER = """// competitive-verifier: PROBLEM https://judge.yosupo.jp/problem/unionfind
// competitive-verifier: TLE 0.5
// competitive-verifier: MLE 64
"""
BODY = """#include <iostream>
#include "mylib/data_structure/UnionFind.hpp"
signed main() { return 0; }
"""


def test_annotations_are_parsed():
    notes = migrate.parse_annotations(HEADER + BODY)
    assert notes.problem == "https://judge.yosupo.jp/problem/unionfind"
    assert notes.tle == 0.5
    assert notes.mle == 64
    assert notes.error is None
    assert not notes.standalone and not notes.ignore


def test_standalone_and_error_and_ignore():
    text = (
        "// competitive-verifier: STANDALONE\n"
        "// competitive-verifier: ERROR 1e-6\n"
        "// competitive-verifier: IGNORE\n"
    )
    notes = migrate.parse_annotations(text)
    assert notes.standalone and notes.ignore
    assert notes.error == 1e-6


def test_stripping_removes_only_the_annotation_lines():
    assert migrate.strip_annotations(HEADER + BODY) == BODY
    # 途中に混ざっていても落とす。ほかの行は 1 文字も変えない。
    text = "int a;\n// competitive-verifier: TLE 1\n  int b; // keep\n"
    assert migrate.strip_annotations(text) == "int a;\n  int b; // keep\n"


@pytest.mark.parametrize(
    "url, source, name, problem_id",
    [
        ("https://judge.yosupo.jp/problem/sharp_p_subset_sum", "library_checker",
         "sharp_p_subset_sum", "yosupo-sharp-p-subset-sum"),
        ("https://onlinejudge.u-aizu.ac.jp/problems/2603", "aoj", "2603", "aoj-2603"),
        ("https://onlinejudge.u-aizu.ac.jp/courses/lesson/1/ALDS1/14/ALDS1_14_B", "aoj",
         "ALDS1_14_B", "aoj-ALDS1_14_B"),
        ("http://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=DSL_2_B", "aoj",
         "DSL_2_B", "aoj-DSL_2_B"),
        ("https://yukicoder.me/problems/no/649", "yukicoder", "649", "yuki-649"),
        ("https://atcoder.jp/contests/typical90/tasks/typical90_bp", "none", "",
         "atcoder-typical90-bp"),
    ],
)
def test_origin_of(url, source, name, problem_id):
    origin = migrate.origin_of(url)
    assert (origin.source, origin.name, origin.id) == (source, name, problem_id)


def test_an_unknown_host_is_refused():
    with pytest.raises(migrate.MigrateError, match="出どころ"):
        migrate.origin_of("https://example.com/problems/1")


@pytest.fixture
def lc_repo(tmp_path):
    """偽の library-checker-problems。"""
    repo = tmp_path / "lc"
    d = repo / "data_structure" / "unionfind"
    d.mkdir(parents=True)
    (d / "info.toml").write_text("title = 'Unionfind'\ntimelimit = 5.0\n")
    (d / "checker.cpp").write_text("int main() {}\n")
    e = repo / "polynomial" / "inv_of_formal_power_series"
    e.mkdir(parents=True)
    (e / "info.toml").write_text("title = 'Inv of $F(x)$'\ntimelimit = 10.0\n")
    return repo


def write_test(directory, name, url=None, extra="", body=BODY):
    path = directory / name
    head = f"// competitive-verifier: PROBLEM {url}\n" if url else ""
    path.write_text(head + extra + body)
    return path


def test_a_single_implementation_becomes_lib_cpp(tmp_path, lc_repo):
    src = write_test(tmp_path, "unionfind.test.cpp", "https://judge.yosupo.jp/problem/unionfind")
    items = migrate.plan([src], existing=set(), library_checker_dir=lc_repo)
    assert len(items) == 1
    item = items[0]
    assert not item.skipped
    assert item.id == "yosupo-unionfind"
    assert item.title == "Unionfind"
    assert item.source == "library_checker"
    assert item.name == "data_structure/unionfind"
    assert item.compare == "checker"
    assert (item.tle_sec, item.mle_mb) == (5.0, migrate.LIBRARY_CHECKER_MLE_MB)
    assert item.submissions == (("lib.cpp", src),)


def test_the_title_loses_its_tex_and_the_time_limit_is_the_official_one(tmp_path, lc_repo):
    src = write_test(
        tmp_path, "inv_of_FPS.test.cpp",
        "https://judge.yosupo.jp/problem/inv_of_formal_power_series",
    )
    item = migrate.plan([src], existing=set(), library_checker_dir=lc_repo)[0]
    assert item.title == "Inv of F(x)"
    assert item.tle_sec == 10.0
    # checker.cpp が無い問題はトークン比較。
    assert item.compare == "tokens"


def test_a_bigger_annotation_raises_the_limit(tmp_path, lc_repo):
    """締めた注釈 (TLE 0.5 / MLE 64) は採らないが、大きい方の注釈には合わせる。"""
    small = write_test(
        tmp_path, "unionfind.test.cpp", "https://judge.yosupo.jp/problem/unionfind",
        extra="// competitive-verifier: TLE 0.5\n// competitive-verifier: MLE 64\n",
    )
    item = migrate.plan([small], existing=set(), library_checker_dir=lc_repo)[0]
    assert (item.tle_sec, item.mle_mb) == (5.0, 1024)
    big = write_test(
        tmp_path, "inv_of_FPS.test.cpp",
        "https://judge.yosupo.jp/problem/inv_of_formal_power_series",
        extra="// competitive-verifier: TLE 20\n// competitive-verifier: MLE 2048\n",
    )
    item = migrate.plan([big], existing=set(), library_checker_dir=lc_repo)[0]
    assert (item.tle_sec, item.mle_mb) == (20.0, 2048)


def test_several_implementations_are_named_after_the_file(tmp_path, lc_repo):
    url = "https://judge.yosupo.jp/problem/unionfind"
    a = write_test(tmp_path, "unionfind.test.cpp", url)
    b = write_test(tmp_path, "unionfind.Weighted_UF.test.cpp", url)
    items = migrate.plan([a, b], existing=set(), library_checker_dir=lc_repo)
    assert len(items) == 1
    assert items[0].submissions == (("lib.cpp", a), ("lib-weighted-uf.cpp", b))


def test_single_only_skips_problems_with_several_implementations(tmp_path, lc_repo):
    url = "https://judge.yosupo.jp/problem/unionfind"
    a = write_test(tmp_path, "unionfind.test.cpp", url)
    b = write_test(tmp_path, "unionfind.two.test.cpp", url)
    items = migrate.plan([a, b], existing=set(), single_only=True, library_checker_dir=lc_repo)
    assert items[0].skipped and "2 本" in items[0].skip


def test_existing_problems_are_skipped(tmp_path, lc_repo):
    src = write_test(tmp_path, "unionfind.test.cpp", "https://judge.yosupo.jp/problem/unionfind")
    items = migrate.plan([src], existing={"yosupo-unionfind"}, library_checker_dir=lc_repo)
    assert items[0].skipped and "既に" in items[0].skip


def test_ignored_tests_are_skipped(tmp_path, lc_repo):
    a = write_test(
        tmp_path, "a.test.cpp", "https://judge.yosupo.jp/problem/unionfind",
        extra="// competitive-verifier: IGNORE\n",
    )
    alone = write_test(
        tmp_path, "b.test.cpp",
        extra="// competitive-verifier: STANDALONE\n// competitive-verifier: IGNORE\n",
    )
    items = migrate.plan([a, alone], existing=set(), library_checker_dir=lc_repo)
    assert all(i.skipped and "IGNORE" in i.skip for i in items)


def test_a_problem_missing_upstream_is_skipped_not_fatal(tmp_path, lc_repo):
    src = write_test(tmp_path, "x.test.cpp", "https://judge.yosupo.jp/problem/nope")
    items = migrate.plan([src], existing=set(), library_checker_dir=lc_repo)
    assert items[0].skipped and "nope" in items[0].skip


def test_standalone_is_exit_code_and_atcoder_is_compile_only(tmp_path, lc_repo):
    alone = write_test(tmp_path, "constexpr_modint.test.cpp", extra="// competitive-verifier: STANDALONE\n")
    at = write_test(tmp_path, "abc123_d.test.cpp", "https://atcoder.jp/contests/abc123/tasks/abc123_d")
    items = {i.id: i for i in migrate.plan([alone, at], existing=set(), library_checker_dir=lc_repo)}
    assert items["constexpr-modint"].compare == "exit_code"
    assert items["constexpr-modint"].source == "none"
    assert items["atcoder-abc123-d"].compare == "compile_only"
    assert items["atcoder-abc123-d"].source == "none"


def test_write_makes_a_loadable_problem(tmp_path, lc_repo):
    from pj import problem as problem_mod

    src = write_test(
        tmp_path, "unionfind.test.cpp", "https://judge.yosupo.jp/problem/unionfind",
        extra="// competitive-verifier: TLE 0.5\n",
    )
    item = migrate.plan([src], existing=set(), library_checker_dir=lc_repo)[0]
    problems = tmp_path / "problems"
    directory = migrate.write(item, problems)
    loaded = problem_mod.load(directory)
    assert loaded.id == "yosupo-unionfind"
    assert loaded.harness_kind == "raw"
    assert loaded.testdata.name == "data_structure/unionfind"
    assert loaded.compare.kind == "checker"
    assert loaded.limits.tle_sec == 5.0
    assert loaded.submissions() == [Path("submissions/lib.cpp")]
    assert (directory / "submissions" / "lib.cpp").read_text() == BODY
    with pytest.raises(migrate.MigrateError, match="既に"):
        migrate.write(item, problems)


def test_directories_are_expanded(tmp_path, lc_repo):
    d = tmp_path / "tests"
    d.mkdir()
    write_test(d, "unionfind.test.cpp", "https://judge.yosupo.jp/problem/unionfind")
    (d / "not_a_test.cpp").write_text("int main() {}\n")
    assert [p.name for p in migrate.collect_files([d])] == ["unionfind.test.cpp"]


class FakeTitles:
    """AOJ の一覧の代わり。名前と制限を返す。"""

    def __init__(self, limits=(1.0, 128)):
        self.limits = limits

    def official(self, probe):
        return f"Title of {probe.testdata.name}"

    def aoj_limits(self, name):
        return self.limits


def test_error_becomes_float_with_both_tolerances(tmp_path):
    src = write_test(
        tmp_path, "CGL_1_A.test.cpp",
        "https://onlinejudge.u-aizu.ac.jp/courses/library/4/CGL/all/CGL_1_A",
        extra="// competitive-verifier: ERROR 0.00000001\n",
    )
    item = migrate.plan([src], existing=set(), titles=FakeTitles())[0]
    assert item.compare == "float"
    assert item.tolerance == 1e-8
    assert item.title == "Title of CGL_1_A"
    toml = migrate.problem_toml(item)
    assert "abs_tol = 1e-08" in toml and "rel_tol = 1e-08" in toml
    from pj import problem as problem_mod

    directory = migrate.write(item, tmp_path / "problems")
    loaded = problem_mod.load(directory)
    assert loaded.compare.kind == "float" and loaded.compare.abs_tol == 1e-8


def test_aoj_limits_never_go_below_the_defaults(tmp_path):
    src = write_test(tmp_path, "2603.test.cpp", "https://onlinejudge.u-aizu.ac.jp/problems/2603")
    tight = migrate.plan([src], existing=set(), titles=FakeTitles((1.0, 64)))[0]
    assert (tight.tle_sec, tight.mle_mb) == (5.0, 512)
    loose = migrate.plan([src], existing=set(), titles=FakeTitles((8.0, 1024)))[0]
    assert (loose.tle_sec, loose.mle_mb) == (8.0, 1024)


def test_atcoder_ignore_is_still_imported_as_compile_only(tmp_path):
    src = write_test(
        tmp_path, "abc123_d.test.cpp", "https://atcoder.jp/contests/abc123/tasks/abc123_d",
        extra="// competitive-verifier: IGNORE\n",
    )
    item = migrate.plan([src], existing=set())[0]
    assert not item.skipped and item.compare == "compile_only"
    other = write_test(
        tmp_path, "2603.test.cpp", "https://onlinejudge.u-aizu.ac.jp/problems/2603",
        extra="// competitive-verifier: IGNORE\n",
    )
    assert migrate.plan([other], existing=set(), titles=FakeTitles())[0].skipped


@pytest.mark.parametrize(
    "url, problem_id",
    [
        ("https://codeforces.com/contest/1140/problem/F", "cf-1140-f"),
        ("https://codeforces.com/gym/102576/problem/C", "cf-gym102576-c"),
        ("https://oj.uz/problem/view/APIO16_fireworks", "ojuz-APIO16_fireworks"),
        ("https://www.codechef.com/problems/CCDSAP", "codechef-CCDSAP"),
        ("https://icpc.kattis.com/problems/conquertheworld", "kattis-conquertheworld"),
        ("https://www.luogu.com.cn/problem/P5055", "luogu-P5055"),
        ("https://www.hackerrank.com/challenges/library-query/problem", "hackerrank-library-query"),
        ("https://www.hackerrank.com/contests/w33/challenges/bonnie-and-clyde", "hackerrank-bonnie-and-clyde"),
        ("https://cses.fi/problemset/task/2132/", "cses-2132"),
        ("https://www2.ioi-jp.org/camp/2019/2019-sp-tasks/day1/examination.pdf", "joisc-2019-examination"),
        ("https://www2.ioi-jp.org/camp/2010/2010-sp-tasks/2010-sp-day2_21.pdf#2", "joisc-2010-sp-day2-21-2"),
    ],
)
def test_manual_judges_get_a_prefixed_id(url, problem_id):
    origin = migrate.origin_of(url)
    assert origin.source == "manual"
    assert origin.id == problem_id
    assert origin.name == problem_id


def test_a_manual_problem_writes_its_url(tmp_path):
    src = write_test(tmp_path, "2132.test.cpp", "https://cses.fi/problemset/task/2132/")
    item = migrate.plan([src], existing=set())[0]
    assert (item.source, item.name, item.compare) == ("manual", "cses-2132", "tokens")
    toml = migrate.problem_toml(item)
    assert 'url = "https://cses.fi/problemset/task/2132/"' in toml
    from pj import problem as problem_mod

    loaded = problem_mod.load(migrate.write(item, tmp_path / "problems"))
    assert loaded.url == "https://cses.fi/problemset/task/2132/"


def test_loj_is_fetched_from_its_api():
    """LOJ はログイン無しの API で取れるので manual ではない。name は番号だけ。"""
    origin = migrate.origin_of("https://loj.ac/p/2419")
    assert (origin.source, origin.name, origin.id) == ("loj", "2419", "loj-2419")


def test_a_loj_problem_does_not_write_its_url(tmp_path):
    src = write_test(tmp_path, "2419.test.cpp", "https://loj.ac/p/2419")
    item = migrate.plan([src], existing=set())[0]
    assert (item.source, item.name, item.compare) == ("loj", "2419", "tokens")
    assert "url =" not in migrate.problem_toml(item)


def test_standalone_takes_its_id_from_the_url_in_a_comment(tmp_path):
    src = write_test(
        tmp_path, "typical90_bp.test.cpp",
        extra="// competitive-verifier: STANDALONE\n\n// https://atcoder.jp/contests/typical90/tasks/typical90_bp\n",
    )

    class Titles:
        def official(self, probe):
            assert probe.testdata.source == "none"
            return "068 - Paired Information（★5）" if "typical90" in probe.url else None

    item = migrate.plan([src], existing=set(), titles=Titles())[0]
    assert item.id == "atcoder-typical90-bp"
    assert (item.source, item.compare) == ("none", "exit_code")
    assert item.title == "068 - Paired Information（★5）"
    assert item.url.endswith("typical90_bp")
    # yukicoder の問題を自分で確かめる形にしたものは yuki- の id になる (yuki-1421 と同じ)。
    src2 = write_test(
        tmp_path, "yukicoder_1420.test.cpp",
        extra="// competitive-verifier: STANDALONE\n// https://yukicoder.me/problems/no/1420\n",
    )
    assert migrate.plan([src2], existing=set())[0].id == "yuki-1420"


def test_standalone_without_a_url_uses_the_whole_file_name(tmp_path):
    src = write_test(tmp_path, "mat.unit_test.cpp", extra="// competitive-verifier: STANDALONE\n")
    item = migrate.plan([src], existing=set())[0]
    assert item.id == "mat-unit-test"
    assert item.submissions == (("lib.cpp", src),)
    assert item.url == ""
    assert "url =" not in migrate.problem_toml(item)


def test_atcoder_titles_come_from_the_page_when_available(tmp_path):
    src = write_test(tmp_path, "abc1_a.test.cpp", "https://atcoder.jp/contests/abc1/tasks/abc1_a")

    class Titles:
        def official(self, probe):
            return "A - Hello"

    item = migrate.plan([src], existing=set(), titles=Titles())[0]
    assert item.title == "A - Hello"
    assert 'url = "https://atcoder.jp/contests/abc1/tasks/abc1_a"' in migrate.problem_toml(item)


def test_atcoder_ex_tasks_are_normalized_to_h():
    origin = migrate.origin_of("https://atcoder.jp/contests/abc234/tasks/abc234_Ex")
    assert origin.id == "atcoder-abc234-h"
    assert origin.url == "https://atcoder.jp/contests/abc234/tasks/abc234_h"
