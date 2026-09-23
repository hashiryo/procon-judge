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


def test_claims_are_per_model_scope_and_run(remote):
    _, a, b = remote
    base = claims_mod.Claims("1", "x64", MODEL, job=0, repo=a)
    assert base.claim("yuki-649") is True
    other_model = claims_mod.Claims("1", "x64", "Intel Xeon 8573C", job=1, repo=b)
    other_scope = claims_mod.Claims("1", "arm", MODEL, job=1, repo=b)
    other_run = claims_mod.Claims("2", "x64", MODEL, job=1, repo=b)
    for claims in (other_model, other_scope, other_run):
        assert claims.taken() == set()
        assert claims.claim("yuki-649") is True


def test_clean_removes_this_run_and_older_finished_ones_only(remote):
    _, a, _ = remote
    for run in ("7", "8", "9", "local"):
        assert claims_mod.Claims(run, "x64-gcc", MODEL, repo=a).claim("p") is True
    assert claims_mod.clean("8", repo=a, finished=lambda run: True) == (2, 0)
    left = {ref.split("/")[2] for ref in claims_mod.list_refs(repo=a)}
    assert left == {"9", "local"}
    assert claims_mod.clean("local", repo=a, finished=lambda run: True) == (1, 0)
    assert {ref.split("/")[2] for ref in claims_mod.list_refs(repo=a)} == {"9"}


def test_clean_keeps_the_claims_of_a_run_that_is_still_going(remote):
    """concurrency group がモードごとに 2 つあるので、古い run が走っている横で collect が
    動く。走っている run の宣言を消すと、そのジョブたちが同じ問題を取り直す。"""
    _, a, _ = remote
    for run in ("7", "8"):
        assert claims_mod.Claims(run, "x64", MODEL, repo=a).claim("p") is True
        assert claims_mod.Claims(run, "x64", MODEL, repo=a).claim("q") is True
    asked = []

    def finished(run):
        asked.append(run)
        return run != "7"

    assert claims_mod.clean("8", repo=a, finished=finished) == (2, 2)
    # 自分の run は聞かない。古い run は 1 回だけ聞く。
    assert asked == ["7"]
    assert {ref.split("/")[2] for ref in claims_mod.list_refs(repo=a)} == {"7"}


def test_when_the_run_status_is_unknown_the_claims_stay(remote, monkeypatch):
    _, a, _ = remote
    monkeypatch.delenv("GITHUB_REPOSITORY", raising=False)
    for run in ("7", "8"):
        assert claims_mod.Claims(run, "x64", MODEL, repo=a).claim("p") is True
    # remote は手元のパスで GitHub ではないので、run の状態は分からない。残す側に倒す。
    assert claims_mod.clean("8", repo=a) == (1, 1)


def test_clean_deletes_in_chunks(remote, monkeypatch):
    _, a, _ = remote
    monkeypatch.setattr(claims_mod, "DELETE_CHUNK", 2)
    claims = claims_mod.Claims("3", "x64-gcc", MODEL, repo=a)
    for pid in ("p1", "p2", "p3", "p4", "p5"):
        assert claims.claim(pid) is True
    assert claims_mod.clean("3", repo=a) == (5, 0)
    assert claims_mod.list_refs(repo=a) == []


def test_any_is_a_model_less_claim(remote):
    """網羅モードの宣言はモデル無し。違うモデルのジョブも同じ名前空間を見る。"""
    _, a, b = remote
    job0 = claims_mod.Claims("1", "x64", claims_mod.ANY, job=0, repo=a)
    job1 = claims_mod.Claims("1", "x64", claims_mod.ANY, job=1, repo=b)
    assert job0.ref("p") == "refs/claims/1/x64/any/p"
    assert job0.claim("p") is True
    assert job1.claim("p") is False


def test_repo_slug_comes_from_the_environment_or_the_remote(tmp_path, monkeypatch):
    monkeypatch.setenv("GITHUB_REPOSITORY", "hashiryo/procon-judge")
    assert claims_mod.repo_slug(tmp_path) == "hashiryo/procon-judge"
    monkeypatch.delenv("GITHUB_REPOSITORY")
    repo = tmp_path / "r"
    subprocess.run(["git", "init", "-q", str(repo)], check=True)
    _git(repo, "remote", "add", "origin", "git@github.com.hashiryo:hashiryo/procon-judge.git")
    assert claims_mod.repo_slug(repo) == "hashiryo/procon-judge"
    _git(repo, "remote", "set-url", "origin", "https://github.com/hashiryo/procon-judge")
    assert claims_mod.repo_slug(repo) == "hashiryo/procon-judge"
    _git(repo, "remote", "set-url", "origin", str(tmp_path / "elsewhere.git"))
    assert claims_mod.repo_slug(repo) is None


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


def test_stale_skips_the_runs_that_are_still_active():
    refs = ["refs/claims/10/x64/M/p", "refs/claims/9/x64/M/p", "refs/claims/8/x64/M/p"]
    assert claims_mod.stale(refs, "10", active={"9"}) == [refs[0], refs[2]]
    assert claims_mod.run_of(refs[1]) == "9"
    assert claims_mod.run_of("refs/heads/main") is None
