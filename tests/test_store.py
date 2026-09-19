"""記録の読み書き。"""

from pj.record import Record
from pj.store import Store


def make_record(**overrides):
    fields = {
        "key": "k1",
        "problem": "p",
        "submission": "submissions/a.hpp",
        "status": "AC",
        "env": "local",
        "cpu_arch": "arm64",
        "cpu_model": "Apple M2 Max",
        "compiler_version": "clang 21",
        "cxxflags": "-O2",
        "cases_hash": "abc",
        "case_count": 3,
        "submission_hash": "sh",
        "includes": ["common.hpp"],
        "library_sha": None,
        "judge_sha": None,
        "time_max_ms": 1,
        "time_total_ms": 2,
        "algo_time_max_ns": 3,
        "algo_time_total_ns": 4,
        "memory_max_kb": 5,
        "source_bytes": 6,
        "binary_bytes": 7,
    }
    fields.update(overrides)
    return Record(**fields)


def test_append_then_read(tmp_path):
    store = Store(tmp_path)
    store.append(make_record())
    assert [r["key"] for r in store.read("p")] == ["k1"]


def test_keys_collects_every_problem(tmp_path):
    store = Store(tmp_path)
    store.append(make_record(key="k1", problem="a"))
    store.append(make_record(key="k2", problem="b"))
    assert store.keys() == {"k1", "k2"}


def test_one_file_per_problem(tmp_path):
    store = Store(tmp_path)
    store.append(make_record(problem="a"))
    store.append(make_record(problem="b"))
    assert store.problem_ids() == ["a", "b"]
    assert store.path_for("a").name == "a.jsonl"


def test_records_are_appended_not_overwritten(tmp_path):
    store = Store(tmp_path)
    store.append(make_record(key="k1"))
    store.append(make_record(key="k2"))
    assert store.count() == 2


def test_round_trip_keeps_the_fields(tmp_path):
    store = Store(tmp_path)
    store.append(make_record(includes=["a.hpp", "b.hpp"]))
    record = next(iter(store.read("p")))
    assert record["includes"] == ["a.hpp", "b.hpp"]
    assert record["failed_case"] is None
    assert record["timestamp"].endswith("Z")


def test_a_broken_line_is_skipped(tmp_path, capsys):
    store = Store(tmp_path)
    store.append(make_record(key="k1"))
    with store.path_for("p").open("a") as f:
        f.write('{"key": "half\n')
    store.append(make_record(key="k2"))
    assert store.keys() == {"k1", "k2"}
    assert "読めません" in capsys.readouterr().err


def test_empty_store(tmp_path):
    store = Store(tmp_path / "nothing")
    assert store.keys() == set()
    assert store.problem_ids() == []
    assert store.count() == 0
