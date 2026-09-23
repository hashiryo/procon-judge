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


def test_keys_does_not_look_at_the_status(tmp_path):
    """スキップの判定はキーだけを見る。

    同じソースを同じ条件で測り直しても結果は変わらないので、AC でない記録
    でも再実行しない。直したときはソースが変わってキーも変わる。
    """
    store = Store(tmp_path)
    for i, status in enumerate(("WA", "TLE", "MLE", "RE", "CE")):
        store.append(make_record(key=f"k{i}", status=status))
    assert store.keys() == {"k0", "k1", "k2", "k3", "k4"}


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


def write_jsonl(path, records):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("".join(r.to_json() + "\n" for r in records))


def test_absorb_takes_records_from_a_directory(tmp_path):
    artifacts = tmp_path / "artifacts"
    write_jsonl(artifacts / "problems" / "p.jsonl", [make_record(key="k1")])
    store = Store(tmp_path / "store")

    assert store.absorb([artifacts]) == (1, 0)
    assert store.keys() == {"k1"}


def test_absorb_is_idempotent(tmp_path):
    artifacts = tmp_path / "artifacts"
    write_jsonl(artifacts / "problems" / "p.jsonl", [make_record(key="k1")])
    store = Store(tmp_path / "store")

    store.absorb([artifacts])
    assert store.absorb([artifacts]) == (0, 1)
    assert store.count() == 1


def test_absorb_looks_deeper_than_one_level(tmp_path):
    # アーティファクトの展開先は run のジョブごとに 1 段深くなる。
    artifacts = tmp_path / "artifacts"
    write_jsonl(
        artifacts / "records-x64-gcc" / "problems" / "p.jsonl",
        [make_record(key="k1")],
    )
    write_jsonl(
        artifacts / "records-arm-gcc" / "problems" / "p.jsonl",
        [make_record(key="k2")],
    )
    store = Store(tmp_path / "store")

    assert store.absorb([artifacts]) == (2, 0)
    assert store.keys() == {"k1", "k2"}


def test_absorb_splits_by_problem(tmp_path):
    artifacts = tmp_path / "artifacts"
    write_jsonl(
        artifacts / "all.jsonl",
        [make_record(key="k1", problem="a"), make_record(key="k2", problem="b")],
    )
    store = Store(tmp_path / "store")

    store.absorb([artifacts])
    assert store.problem_ids() == ["a", "b"]


def test_absorb_ignores_a_broken_line(tmp_path, capsys):
    artifacts = tmp_path / "artifacts"
    path = artifacts / "problems" / "p.jsonl"
    write_jsonl(path, [make_record(key="k1")])
    with path.open("a") as f:
        f.write("{not json\n")
    store = Store(tmp_path / "store")

    assert store.absorb([artifacts]) == (1, 0)
    assert "読めません" in capsys.readouterr().err


def test_absorb_of_nothing(tmp_path):
    store = Store(tmp_path / "store")
    assert store.absorb([tmp_path / "missing"]) == (0, 0)


# --- 束 ---------------------------------------------------------------------


def test_absorb_keeps_the_same_key_from_another_batch(tmp_path):
    """束は同じキーを束ごとに 1 件ずつ作る。重複排除は (キー, 束) で見る。"""
    artifacts = tmp_path / "artifacts"
    write_jsonl(
        artifacts / "problems" / "p.jsonl",
        [
            make_record(key="k1", batch="r1/x64/0"),
            make_record(key="k1", batch="r2/x64/0"),
            make_record(key="k1", batch="r2/x64/0"),
        ],
    )
    store = Store(tmp_path / "store")
    assert store.absorb([artifacts]) == (2, 1)
    assert store.identities() == {("k1", "r1/x64/0"), ("k1", "r2/x64/0")}


def test_a_record_without_a_batch_still_dedups_by_key(tmp_path):
    artifacts = tmp_path / "artifacts"
    write_jsonl(artifacts / "problems" / "p.jsonl", [make_record(key="k1"), make_record(key="k1")])
    store = Store(tmp_path / "store")
    assert store.absorb([artifacts]) == (1, 1)


def test_batches_index_the_newest_per_problem_env_and_model(tmp_path):
    store = Store(tmp_path / "store")
    store.append(make_record(key="k1", batch="r1/x64/0", timestamp="2026-01-01T00:00:00Z"))
    store.append(make_record(key="k2", batch="r2/x64/1", timestamp="2026-01-02T00:00:00Z"))
    store.append(make_record(key="k3", problem="q", batch="r1/x64/0", timestamp="2026-01-01T00:00:00Z"))
    store.append(make_record(key="k4", problem="q"))
    batches = store.batches()
    assert batches[("p", "local", "Apple M2 Max")].id == "r2/x64/1"
    assert batches[("q", "local", "Apple M2 Max")].keys == {"k3"}
    assert len(batches) == 2
