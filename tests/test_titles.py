"""題名を判定サイトと突き合わせる。取得元ごとの引き方と、書き戻し。"""

import pytest

from pj import problem as problem_mod
from pj import titles as titles_mod

TOML = """
id = "{id}"
title = "{title}"

[harness]
kind = "raw"

[testdata]
source = "{source}"
name = "{name}"

[compare]
kind = "{compare}"
"""


def make_problem(tmp_path, *, pid, title, source, name, compare="tokens"):
    directory = tmp_path / pid
    directory.mkdir()
    (directory / "problem.toml").write_text(
        TOML.format(id=pid, title=title, source=source, name=name, compare=compare)
    )
    (directory / "submissions").mkdir()
    return problem_mod.load(directory)


def test_aoj_names_come_from_the_paged_list(tmp_path, monkeypatch):
    pages = {
        0: [{"id": "0000", "name": "QQ"}] * 999 + [{"id": "0629", "name": "Geologic Fault"}],
        1: [{"id": "DSL_2_B", "name": "Range Sum Query (RSQ)"}],
    }
    calls = []

    def fake_get_json(url):
        page = int(url.split("page=")[1].split("&")[0])
        calls.append(page)
        return pages[page]

    monkeypatch.setattr(titles_mod, "_get_json", fake_get_json)
    titles = titles_mod.Titles()
    p = make_problem(tmp_path, pid="aoj-0629", title="Sweeping", source="aoj", name="0629")
    assert titles.official(p) == "Geologic Fault"
    q = make_problem(tmp_path, pid="aoj-DSL_2_B", title="x", source="aoj", name="DSL_2_B")
    assert titles.official(q) == "Range Sum Query (RSQ)"
    # 一覧は 1 回しか取らない。2 ページ目で 1000 件に満たないので止まる。
    assert calls == [0, 1]


def test_an_unknown_aoj_id_is_an_error(tmp_path, monkeypatch):
    monkeypatch.setattr(titles_mod, "_get_json", lambda url: [{"id": "0000", "name": "QQ"}])
    p = make_problem(tmp_path, pid="aoj-9999", title="x", source="aoj", name="9999")
    with pytest.raises(titles_mod.TitleError):
        titles_mod.Titles().official(p)


def test_yukicoder_title_comes_from_its_api(tmp_path, monkeypatch):
    monkeypatch.setattr(
        titles_mod, "_get_json", lambda url: {"No": 703, "Title": "ゴミ拾い Easy"}
    )
    p = make_problem(tmp_path, pid="yuki-703", title="Mirror Dungeon", source="yukicoder", name="703")
    assert titles_mod.Titles().official(p) == "ゴミ拾い Easy"


def test_library_checker_title_comes_from_the_local_clone(tmp_path, monkeypatch):
    clone = tmp_path / "lc"
    (clone / "math" / "sharp_p_subset_sum").mkdir(parents=True)
    (clone / "math" / "sharp_p_subset_sum" / "info.toml").write_text(
        'title = "$\\\\#_p$ Subset Sum"\ntimelimit = 5.0\n'
    )
    monkeypatch.setattr(titles_mod, "LIBRARY_CHECKER_DIR", clone)
    monkeypatch.setattr(titles_mod, "_get_text", lambda url: pytest.fail("網に出てはいけない"))
    p = make_problem(
        tmp_path, pid="yosupo-sharp-p-subset-sum", title="x",
        source="library_checker", name="math/sharp_p_subset_sum",
    )
    assert titles_mod.Titles().official(p) == "#p Subset Sum"


def test_other_sources_have_no_official_title(tmp_path):
    p = make_problem(
        tmp_path, pid="gf2-64", title="mine", source="none", name="", compare="compile_only"
    )
    assert titles_mod.Titles().official(p) is None


def test_plain_strips_tex():
    assert titles_mod.plain("$\\#_p$ Subset Sum") == "#p Subset Sum"
    assert titles_mod.plain("Point Add Range Sum") == "Point Add Range Sum"


def test_rewrite_touches_only_the_title_line(tmp_path):
    p = make_problem(tmp_path, pid="aoj-1508", title="old", source="aoj", name="1508")
    path = p.dir / "problem.toml"
    before = path.read_text()
    titles_mod.rewrite_title(path, 'Networking the "Death Star" \\ more')
    after = path.read_text()
    assert problem_mod.load(p.dir).title == 'Networking the "Death Star" \\ more'
    assert after.replace(after.splitlines()[2], "") == before.replace(before.splitlines()[2], "")


def test_rewrite_without_a_title_line_fails(tmp_path):
    path = tmp_path / "problem.toml"
    path.write_text('id = "x"\n')
    with pytest.raises(titles_mod.TitleError):
        titles_mod.rewrite_title(path, "t")
