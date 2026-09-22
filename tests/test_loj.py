"""LOJ の API からの取得。網には出ず、組の作り方と落とし方を確かめる。"""

import pytest

from pj import problem as problem_mod
from pj.fetch import FetchError, loj

TOML = """
id = "loj-2419"
title = "t"

[harness]
kind = "raw"

[testdata]
source = "loj"
name = "2419"

[compare]
kind = "tokens"
"""


def files(*names, size=None):
    return [{"filename": n, "size": size} for n in names]


def test_pairs_follow_the_subtasks():
    info = {
        "testData": files(
            "input0.txt",
            "output0.txt",
            "input1.txt",
            "output1.txt",
            "gcd1.in",
            "gcd1.ans",
        ),
        "judgeInfo": {
            "subtasks": [
                {
                    "testcases": [
                        {"inputFile": "input0.txt", "outputFile": "output0.txt"}
                    ]
                },
                # 同じケースが別のサブタスクにも出る。入力ファイルで畳む。
                {
                    "testcases": [
                        {"inputFile": "input0.txt", "outputFile": "output0.txt"},
                        {"inputFile": "input1.txt", "outputFile": "output1.txt"},
                        {"inputFile": "gcd1.in", "outputFile": "gcd1.ans"},
                    ]
                },
            ]
        },
    }
    assert loj.pairs(info) == [
        ("input0", "input0.txt", "output0.txt"),
        ("input1", "input1.txt", "output1.txt"),
        ("gcd1", "gcd1.in", "gcd1.ans"),
    ]


def test_pairs_fall_back_to_stems_without_subtasks():
    info = {
        "testData": files(
            "1.000.in", "1.000.out", "2.in", "2.ans", "lonely.in", "readme.txt"
        ),
        "judgeInfo": {"subtasks": None},
    }
    assert loj.pairs(info) == [
        ("1.000", "1.000.in", "1.000.out"),
        ("2", "2.in", "2.ans"),
    ]


def test_pairs_reject_files_missing_from_the_list():
    info = {
        "testData": files("1.in"),
        "judgeInfo": {
            "subtasks": [{"testcases": [{"inputFile": "1.in", "outputFile": "1.out"}]}]
        },
    }
    with pytest.raises(FetchError, match="1.out"):
        loj.pairs(info)


def test_pairs_reject_duplicate_case_names():
    info = {
        "testData": files("a.in", "a.out", "a.txt", "b.out"),
        "judgeInfo": {
            "subtasks": [
                {
                    "testcases": [
                        {"inputFile": "a.in", "outputFile": "a.out"},
                        {"inputFile": "a.txt", "outputFile": "b.out"},
                    ]
                }
            ]
        },
    }
    with pytest.raises(FetchError, match="重なります"):
        loj.pairs(info)


def test_limits_come_from_judge_info(monkeypatch):
    monkeypatch.setattr(
        loj,
        "problem_info",
        lambda number: {"judgeInfo": {"timeLimit": 1500, "memoryLimit": 512}},
    )
    assert loj.limits("127") == (1.5, 512)


def test_title_is_whatever_locale_the_api_returns(monkeypatch):
    monkeypatch.setattr(
        loj,
        "problem_info",
        lambda number: {
            "localizedContentsOfLocale": {"locale": "zh_CN", "title": "最大流 加强版"}
        },
    )
    assert loj.title("127") == "最大流 加强版"


def test_a_non_numeric_number_is_an_error():
    loj.problem_info.cache_clear()
    with pytest.raises(FetchError, match="数字"):
        loj.problem_info("loj-2419")


def test_fetch_writes_in_and_out_pairs_using_the_internal_id(tmp_path, monkeypatch):
    directory = tmp_path / "loj-2419"
    directory.mkdir()
    (directory / "problem.toml").write_text(TOML)
    problem = problem_mod.load(directory)
    info = {
        # downloadProblemFiles に渡すのは表示の番号ではなく内部の id。
        "meta": {"id": 40988, "displayId": 2419},
        "testData": files("1.in", "1.out", "2.in", "2.ans"),
        "judgeInfo": {"subtasks": None},
    }
    monkeypatch.setattr(loj, "problem_info", lambda number: info)
    posted = []

    def fake_post(method, payload):
        posted.append((method, payload))
        return {
            "downloadInfo": [
                {"filename": f, "downloadUrl": f"https://r2/{f}"}
                for f in payload["filenameList"]
            ]
        }

    monkeypatch.setattr(loj, "_post", fake_post)
    monkeypatch.setattr(loj, "_get", lambda url: url.rsplit("/", 1)[-1].encode())
    monkeypatch.setattr(loj, "DOWNLOAD_CHUNK", 3)  # 4 ファイルなので 2 回に分かれる

    dest = tmp_path / "dest"
    loj.fetch(problem, dest)

    assert sorted(p.name for p in dest.iterdir()) == ["1.in", "1.out", "2.in", "2.out"]
    assert (dest / "2.out").read_bytes() == b"2.ans"
    assert [method for method, _ in posted] == [
        "downloadProblemFiles",
        "downloadProblemFiles",
    ]
    assert all(payload["problemId"] == 40988 for _, payload in posted)
    assert not (tmp_path / "dest.tmp").exists()
