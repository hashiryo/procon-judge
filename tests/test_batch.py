"""束。いちばん新しい束の選び方と、今の全提出のキーが揃っているかの判定。"""

from pj import batch as batch_mod


def rec(key, batch, stamp, env="x64-gcc", model="EPYC"):
    return {"key": key, "batch": batch, "timestamp": stamp, "env": env, "cpu_model": model}


def test_the_newest_batch_is_the_one_with_the_latest_record():
    rows = [
        rec("a", "r1/x64/0", "2026-01-01T00:00:00Z"),
        rec("b", "r1/x64/0", "2026-01-01T00:01:00Z"),
        rec("a2", "r2/x64/3", "2026-01-02T00:00:00Z"),
    ]
    newest = batch_mod.newest(rows, env="x64-gcc", cpu_model="EPYC")
    assert newest is not None
    assert newest.id == "r2/x64/3"
    assert newest.keys == {"a2"}
    assert newest.timestamp == "2026-01-02T00:00:00Z"


def test_records_without_a_batch_do_not_make_one():
    rows = [rec("a", None, "2026-01-03T00:00:00Z"), rec("b", "", "2026-01-03T00:00:00Z")]
    assert batch_mod.newest(rows, env="x64-gcc", cpu_model="EPYC") is None
    assert batch_mod.index(rows) == {}


def test_batches_are_per_environment_and_model():
    rows = [
        rec("a", "r1/x64/0", "2026-01-01T00:00:00Z", model="EPYC"),
        rec("b", "r1/x64/1", "2026-01-02T00:00:00Z", model="Xeon"),
    ]
    index = batch_mod.index(rows)
    assert index[("x64-gcc", "EPYC")].id == "r1/x64/0"
    assert index[("x64-gcc", "Xeon")].id == "r1/x64/1"
    assert batch_mod.newest(rows, env="x64-clang", cpu_model="EPYC") is None


def test_the_same_timestamp_is_broken_by_the_id_so_every_reader_agrees():
    rows = [rec("a", "r1/x64/0", "t"), rec("b", "r1/x64/1", "t")]
    assert batch_mod.newest(rows, env="x64-gcc", cpu_model="EPYC").id == "r1/x64/1"


def test_covers_needs_every_key():
    rows = [rec("a", "r1/x64/0", "t"), rec("b", "r1/x64/0", "t")]
    batch = batch_mod.newest(rows, env="x64-gcc", cpu_model="EPYC")
    assert batch.covers(["a", "b"])
    assert batch.covers([])
    assert not batch.covers(["a", "c"])


def test_the_ids():
    assert batch_mod.ci_id("357", "x64", 3) == "357/x64/3"
    local = batch_mod.local_id()
    assert local.startswith("local/") and len(local) == len("local/") + 12
    assert batch_mod.local_id() != local
