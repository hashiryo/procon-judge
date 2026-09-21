"""保管庫。固めて戻したときに cases_hash が変わらないことを押さえる。"""

import json
import shutil
import subprocess
from pathlib import Path

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


def test_only_local_is_not_mirrored(tmp_path):
    # リポジトリの中から生成するものだけ保管しない。library_checker も生成し直せるが、
    # 130 問で 10 GB を超えて actions/cache に載らないので保管する (再現性は pin が持つ)。
    assert mirror.REGENERABLE_SOURCES == {"local"}
    assert mirror.should_mirror(make_problem(tmp_path, "a", "library_checker"))
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
def test_the_archive_holds_cases_the_manifest_and_the_checker_sources(tmp_path):
    """チェッカのソースは入れる (無いと保管庫から取った問題で判定器が組めない)。
    手元で組んだ checker.bin は入れない。"""
    source = make_cases(tmp_path / "cases", count=2)
    (source / "checker-x86_64.bin").write_bytes(b"\x7fELF not a test case")
    (source / "checker.cpp").write_text("int main() {}\n")
    (source / "testlib.h").write_text("// testlib\n")
    (source / "params.h").write_text("#define N 1\n")

    archive = tmp_path / "a.tar.zst"
    mirror.pack(source, archive)
    restored = tmp_path / "restored"
    mirror.unpack(archive, restored)

    names = sorted(p.name for p in restored.iterdir())
    assert names == [
        "case_00.in", "case_00.out", "case_01.in", "case_01.out",
        "checker.cpp", "manifest.json", "params.h", "testlib.h",
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


# --- 同時に走っても壊さない -------------------------------------------------


class FakeGh:
    """gh の代わり。呼ばれた引数を覚えて、置いてあるアセットを持つ。"""

    def __init__(self, existing=()):
        self.names = set(existing)
        self.calls = []
        self.upload_fails = False

    def __call__(self, *args, check=True):
        self.calls.append(args)
        if args[:2] == ("release", "view"):
            return _proc(0, json.dumps({"assets": [{"name": n} for n in self.names]}))
        if args[:2] == ("release", "upload"):
            name = Path(args[3]).name
            if self.upload_fails:
                return _proc(1, "", "HTTP 422: already_exists")
            self.names.add(name)
            return _proc(0, "")
        if args[:2] == ("release", "download"):
            return _proc(1, "", "release asset not found")
        return _proc(0, "")


def _proc(code, out, err=""):
    return subprocess.CompletedProcess(["gh"], code, out, err)


@pytest.fixture
def fake_gh(monkeypatch):
    gh = FakeGh()
    monkeypatch.setattr(mirror, "_gh", gh)
    monkeypatch.setattr(mirror, "_names", None)
    return gh


@needs_zstd
def test_push_does_not_clobber(tmp_path, fake_gh):
    """--clobber は消してから上げるので、同時に走るとアセットが消える。"""
    problem = make_problem(tmp_path)
    mirror.push(problem, make_cases(tmp_path / "cases"))
    uploads = [c for c in fake_gh.calls if c[:2] == ("release", "upload")]
    assert uploads and all("--clobber" not in c for c in uploads)


@needs_zstd
def test_push_skips_what_is_already_there(tmp_path, fake_gh):
    problem = make_problem(tmp_path)
    fake_gh.names.add(mirror.asset_name(problem))
    mirror.push(problem, make_cases(tmp_path / "cases"))
    assert not [c for c in fake_gh.calls if c[:2] == ("release", "upload")]


@needs_zstd
def test_push_with_force_clobbers(tmp_path, fake_gh):
    """取り直したデータに入れ替えるときだけ。人が 1 本で叩く前提。"""
    problem = make_problem(tmp_path)
    fake_gh.names.add(mirror.asset_name(problem))
    mirror.push(problem, make_cases(tmp_path / "cases"), force=True)
    uploads = [c for c in fake_gh.calls if c[:2] == ("release", "upload")]
    assert uploads and "--clobber" in uploads[0]


@needs_zstd
def test_losing_the_race_is_not_an_error(tmp_path, fake_gh, capsys):
    """負けた側は何も消していない。上がっているので通す。"""
    problem = make_problem(tmp_path)
    name = mirror.asset_name(problem)
    fake_gh.upload_fails = True
    fake_gh.names.add(name)
    mirror.asset_names(refresh=True).discard(name)  # 引いた時点では無かった
    mirror.push(problem, make_cases(tmp_path / "cases"))
    assert "別のジョブが先に上げました" in capsys.readouterr().err


@needs_zstd
def test_a_real_upload_failure_raises(tmp_path, fake_gh):
    problem = make_problem(tmp_path)
    fake_gh.upload_fails = True
    with pytest.raises(mirror.MirrorError):
        mirror.push(problem, make_cases(tmp_path / "cases"))


def test_pull_says_when_it_is_not_there(tmp_path, fake_gh, capsys):
    problem = make_problem(tmp_path)
    assert mirror.pull(problem, tmp_path / "dest") is False
    assert "まだありません" in capsys.readouterr().err


def test_pull_says_when_it_could_not_download(tmp_path, fake_gh, capsys):
    """あるはずのものが取れないのは、保管庫を置いた目的と逆。黙らない。"""
    problem = make_problem(tmp_path)
    fake_gh.names.add(mirror.asset_name(problem))
    assert mirror.pull(problem, tmp_path / "dest") is False
    assert "取れませんでした" in capsys.readouterr().err
