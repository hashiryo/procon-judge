"""保管庫。固めて戻したときに cases_hash が変わらないことを押さえる。"""

import shutil

import pytest

from pj import problem as problem_mod
from pj.fetch import collect_cases, compute_cases_hash, mirror

TOML = """
id = "{id}"
title = "T"

[harness]
kind = "raw"

[testdata]
source = "{source}"
name = "n"

[compare]
kind = "tokens"
"""


def make_problem(tmp_path, name="p", source="aoj"):
    directory = tmp_path / name
    directory.mkdir()
    (directory / "problem.toml").write_text(TOML.format(id=name, source=source))
    return problem_mod.load(directory)


def make_cases(directory, count=3):
    directory.mkdir(parents=True, exist_ok=True)
    for i in range(count):
        (directory / f"case_{i:02}.in").write_text(f"{i}\n")
        (directory / f"case_{i:02}.out").write_text(f"{i * 2}\n")
    (directory / "manifest.json").write_text('{"cases_hash": "x"}\n')
    return directory


needs_zstd = pytest.mark.skipif(
    shutil.which("zstd") is None, reason="zstd が入っていません"
)


def test_asset_name_is_derived_from_the_problem_id(tmp_path):
    problem = make_problem(tmp_path, "aoj-DSL_2_B")
    assert mirror.asset_name(problem) == "aoj-DSL_2_B.tar.zst"


def test_regenerable_sources_are_not_mirrored(tmp_path):
    # ジェネレータから決定的に作れるものは保管しない。容量を食うだけ。
    assert mirror.REGENERABLE_SOURCES == {"library_checker", "local"}
    assert not mirror.should_mirror(make_problem(tmp_path, "a", "library_checker"))
    assert mirror.should_mirror(make_problem(tmp_path, "c", "aoj"))
    assert mirror.should_mirror(make_problem(tmp_path, "d", "yukicoder"))
    assert mirror.should_mirror(make_problem(tmp_path, "e", "manual"))


@needs_zstd
def test_pack_and_unpack_keep_the_cases_hash(tmp_path):
    source = make_cases(tmp_path / "cases")
    before = compute_cases_hash(collect_cases(source))

    archive = tmp_path / "a.tar.zst"
    mirror.pack(source, archive)
    restored = tmp_path / "restored"
    mirror.unpack(archive, restored)

    assert compute_cases_hash(collect_cases(restored)) == before


@needs_zstd
def test_the_archive_holds_only_cases_and_the_manifest(tmp_path):
    source = make_cases(tmp_path / "cases", count=2)
    (source / "checker.bin").write_bytes(b"\x7fELF not a test case")
    (source / "checker.cpp").write_text("int main() {}\n")

    archive = tmp_path / "a.tar.zst"
    mirror.pack(source, archive)
    restored = tmp_path / "restored"
    mirror.unpack(archive, restored)

    names = sorted(p.name for p in restored.iterdir())
    assert names == [
        "case_00.in", "case_00.out", "case_01.in", "case_01.out", "manifest.json"
    ]


@needs_zstd
def test_packing_nothing_is_an_error(tmp_path):
    empty = tmp_path / "empty"
    empty.mkdir()
    with pytest.raises(mirror.MirrorError, match="固めるもの"):
        mirror.pack(empty, tmp_path / "a.tar.zst")


def test_without_a_token_it_is_unavailable(monkeypatch):
    monkeypatch.delenv(mirror.TOKEN_ENV, raising=False)
    assert mirror.token() is None
    assert not mirror.available()


def test_a_missing_token_says_so(monkeypatch, tmp_path):
    monkeypatch.delenv(mirror.TOKEN_ENV, raising=False)
    with pytest.raises(mirror.MirrorError, match=mirror.TOKEN_ENV):
        mirror.pull(make_problem(tmp_path), tmp_path / "dest")
