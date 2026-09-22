"""宣言。手元の bare リポジトリを remote にして、競合と冪等性と掃除を確かめる。"""

import subprocess
from pathlib import Path

import pytest

from pj import claims as claims_mod

MODEL = "AMD EPYC 9V74 96-Core Processor"


def _git(repo: Path, *args: str) -> str:
    return subprocess.run(
        ["git", "-C", str(repo), "-c", "user.name=t", "-c", "user.email=t@t", *args],
        check=True,
        capture_output=True,
        text=True,
    ).stdout


@pytest.fixture
def remote(tmp_path):
    """bare の remote と、それを clone した 2 つの作業ツリー (別々のジョブのつもり)。"""
    bare = tmp_path / "remote.git"
    subprocess.run(["git", "init", "-q", "--bare", str(bare)], check=True)
    clones = []
    for name in ("a", "b"):
        path = tmp_path / name
        subprocess.run(
            ["git", "clone", "-q", str(bare), str(path)],
            check=True,
            capture_output=True,
        )
        clones.append(path)
    _git(clones[0], "commit", "-q", "--allow-empty", "-m", "init")
    _git(clones[0], "push", "-q", "origin", "HEAD:main")
    return bare, clones[0], clones[1]


@pytest.fixture(autouse=True)
def no_wait(monkeypatch):
    monkeypatch.setattr(claims_mod, "RETRY_WAIT_SEC", 0.0)


def test_slug_makes_a_ref_component():
    assert claims_mod.slug(MODEL) == "AMD-EPYC-9V74-96-Core-Processor"
    assert claims_mod.slug("aoj-DSL_2_B") == "aoj-DSL_2_B"
    assert claims_mod.slug("a..b/c") == "a.b-c"
    assert claims_mod.slug("..") == "unknown"
    assert claims_mod.slug("x.lock") == "x-lock"


def test_a_claim_is_visible_and_excludes_the_other_job(remote):
    _, a, b = remote
    job0 = claims_mod.Claims("1", "x64-gcc", MODEL, job=0, repo=a)
    job1 = claims_mod.Claims("1", "x64-gcc", MODEL, job=1, repo=b)
    assert job0.taken() == set()
    assert job0.claim("yuki-649") is True
    assert job1.taken() == {"yuki-649"}
    assert job1.claim("yuki-649") is False
    assert job1.claim("aoj-DSL_2_B") is True
    assert job0.taken() == {"yuki-649", "aoj-DSL_2_B"}


def test_retrying_ones_own_claim_is_a_success(remote):
    """push がタイムアウトしたあと実は通っていた、のやり直し。同じ sha なので up-to-date。"""
    _, a, _ = remote
    job0 = claims_mod.Claims("1", "x64-gcc", MODEL, job=0, repo=a)
    assert job0.claim("yuki-649") is True
    assert job0.claim("yuki-649") is True


def test_claims_are_per_model_env_and_run(remote):
    _, a, b = remote
    base = claims_mod.Claims("1", "x64-gcc", MODEL, job=0, repo=a)
    assert base.claim("yuki-649") is True
    other_model = claims_mod.Claims("1", "x64-gcc", "Intel Xeon 8573C", job=1, repo=b)
    other_env = claims_mod.Claims("1", "x64-clang", MODEL, job=1, repo=b)
    other_run = claims_mod.Claims("2", "x64-gcc", MODEL, job=1, repo=b)
    for claims in (other_model, other_env, other_run):
        assert claims.taken() == set()
        assert claims.claim("yuki-649") is True


def test_clean_removes_this_run_and_older_ones_only(remote):
    _, a, _ = remote
    for run in ("7", "8", "9", "local"):
        assert claims_mod.Claims(run, "x64-gcc", MODEL, repo=a).claim("p") is True
    assert claims_mod.clean("8", repo=a) == 2
    left = {ref.split("/")[2] for ref in claims_mod.list_refs(repo=a)}
    assert left == {"9", "local"}
    assert claims_mod.clean("local", repo=a) == 1
    assert {ref.split("/")[2] for ref in claims_mod.list_refs(repo=a)} == {"9"}


def test_clean_deletes_in_chunks(remote, monkeypatch):
    _, a, _ = remote
    monkeypatch.setattr(claims_mod, "DELETE_CHUNK", 2)
    claims = claims_mod.Claims("3", "x64-gcc", MODEL, repo=a)
    for pid in ("p1", "p2", "p3", "p4", "p5"):
        assert claims.claim(pid) is True
    assert claims_mod.clean("3", repo=a) == 5
    assert claims_mod.list_refs(repo=a) == []


def test_an_unreachable_remote_is_an_error(remote, monkeypatch):
    _, a, _ = remote
    monkeypatch.setattr(claims_mod, "RETRIES", 1)
    broken = claims_mod.Claims("1", "x64-gcc", MODEL, repo=a, remote="nowhere")
    with pytest.raises(claims_mod.ClaimError):
        broken.taken()
    with pytest.raises(claims_mod.ClaimError):
        broken.claim("p")


def test_stale_keeps_newer_runs():
    refs = [
        "refs/claims/10/x64-gcc/M/p",
        "refs/claims/11/x64-gcc/M/p",
        "refs/claims/9/x64-gcc/M/p",
        "refs/claims/local/x64-gcc/M/p",
    ]
    assert claims_mod.stale(refs, "10") == refs[0:1] + refs[2:3]
    assert claims_mod.stale(refs, "local") == refs[3:]
