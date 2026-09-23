"""run のジョブが自分の仕事を宣言する。

宣言は procon-judge のリポジトリに ref を 1 本作ること。名前は
`refs/claims/<run id>/<組>/<CPU モデル>/<問題 id>`。組は CI のジョブの単位 (x64 / arm)
で、1 本のジョブがその組の全環境 (gcc と clang) を測る。`refs/heads` の外なので
`on: push` は起きず、GitHub の画面にも出ない。網羅モード (pj.plan の cover) の宣言は
モデルの位置が固定の `any` で、組の全ジョブが同じ名前空間を見る。1 つの問題を測るのは
run の中で 1 本だけになり、どのモデルで測るかは当たりで決まる。

作るのは `git push --force-with-lease=<ref>:` で、期待値を空にした lease は
「その ref がまだ無いこと」を条件にする。既にあれば server が stale info で弾く。
同じ問題を同じ瞬間に取ろうとしても、通るのは片方だけ。

押す commit は宣言ごとに一意の、親の無いもの。同じ sha を押すと client が
「Everything up-to-date」と判定して server に届かないので、run id と環境と
モデルと問題とジョブ番号を message に入れて sha を分ける。裏返しで、同じ
ジョブが同じ宣言をやり直すと同じ sha になり、up-to-date で成功扱いになる。
push がタイムアウトしたあと実は通っていた、が起きても二重に数えない。

REST の API は使わない。GITHUB_TOKEN は 1 リポジトリ 1 時間 1000 リクエストで、
全記録が参考に落ちる回 (数千の宣言) に足りない。git の push はこの枠に入らない。
"""

from __future__ import annotations

import os
import re
import subprocess
import time
from collections.abc import Callable, Collection, Iterable
from dataclasses import dataclass
from pathlib import Path

from .paths import ROOT

NAMESPACE = "refs/claims"
# 網羅モードの宣言の、CPU モデルの位置に入る固定の名前。
ANY = "any"
GIT_TIMEOUT_SEC = 120
# 網の都合で失敗したときのやり直し。弾かれた (取られていた) のはやり直さない。
RETRIES = 3
RETRY_WAIT_SEC = 2.0
# 掃除で 1 回の push に載せる ref の数。
DELETE_CHUNK = 100

# 宣言の commit の作者。宣言の sha を決めるのは message だけにしたいので、日時も固定する。
_COMMIT_ENV = {
    "GIT_AUTHOR_NAME": "procon-judge",
    "GIT_AUTHOR_EMAIL": "claims@procon-judge",
    "GIT_AUTHOR_DATE": "2000-01-01T00:00:00Z",
    "GIT_COMMITTER_NAME": "procon-judge",
    "GIT_COMMITTER_EMAIL": "claims@procon-judge",
    "GIT_COMMITTER_DATE": "2000-01-01T00:00:00Z",
}


class ClaimError(Exception):
    """宣言の読み書きが網やリポジトリの都合でできないときに投げる。"""


def slug(text: str) -> str:
    """ref の 1 成分にできる名前にする。

    CPU モデルは "AMD EPYC 9V74 96-Core Processor" のように空白を含む。ref に
    使えない文字は `-` にまとめ、`..` と先頭末尾の `.` と `.lock` の末尾を避ける。
    問題 id は元から使える文字しか無いので、そのまま通る。
    """
    cleaned = re.sub(r"[^A-Za-z0-9._-]+", "-", text)
    cleaned = re.sub(r"\.{2,}", ".", cleaned).strip("-.")
    if cleaned.endswith(".lock"):
        cleaned = cleaned[: -len(".lock")] + "-lock"
    return cleaned or "unknown"


def _git(repo: Path, *args: str, env: dict[str, str] | None = None) -> str:
    try:
        proc = subprocess.run(
            ["git", "-C", str(repo), *args],
            capture_output=True,
            text=True,
            timeout=GIT_TIMEOUT_SEC,
            env={**os.environ, **(env or {})},
            check=False,
        )
    except (subprocess.SubprocessError, OSError) as e:
        raise ClaimError(f"git {' '.join(args[:2])}: {e}") from e
    if proc.returncode != 0:
        raise ClaimError(f"git {' '.join(args[:2])}: {proc.stderr.strip()[-500:]}")
    return proc.stdout


@dataclass(frozen=True)
class Claims:
    """1 つの run の、1 つの組 (ジョブの単位) と CPU モデルについての宣言。"""

    run_id: str
    scope: str
    cpu_model: str
    job: int = 0
    repo: Path = ROOT
    remote: str = "origin"

    @property
    def prefix(self) -> str:
        return (
            f"{NAMESPACE}/{slug(self.run_id)}/{slug(self.scope)}/{slug(self.cpu_model)}/"
        )

    def ref(self, problem_id: str) -> str:
        return self.prefix + slug(problem_id)

    def taken(self) -> set[str]:
        """今ある宣言。ref の末尾 (問題 id の slug) の集合。1 往復で取れる。"""
        last: ClaimError | None = None
        for attempt in range(1, RETRIES + 1):
            try:
                out = _git(
                    self.repo, "ls-remote", "--refs", self.remote, self.prefix + "*"
                )
                break
            except ClaimError as e:
                last = e
                if attempt < RETRIES:
                    time.sleep(RETRY_WAIT_SEC)
        else:
            assert last is not None
            raise last
        found: set[str] = set()
        for line in out.splitlines():
            _, _, name = line.partition("\t")
            if name.startswith(self.prefix):
                found.add(name[len(self.prefix) :])
        return found

    def claim(self, problem_id: str) -> bool:
        """宣言する。取れたら True、他のジョブが先に取っていたら False。

        網の都合で push が終わらなかったときは少しやり直す。同じ sha を押すので、
        実は通っていた回のやり直しは up-to-date で True になる。
        """
        ref = self.ref(problem_id)
        commit = self._commit(problem_id)
        for attempt in range(1, RETRIES + 1):
            try:
                proc = subprocess.run(
                    [
                        "git",
                        "-C",
                        str(self.repo),
                        "push",
                        "--quiet",
                        f"--force-with-lease={ref}:",
                        self.remote,
                        f"{commit}:{ref}",
                    ],
                    capture_output=True,
                    text=True,
                    timeout=GIT_TIMEOUT_SEC,
                    check=False,
                )
            except (subprocess.SubprocessError, OSError) as e:
                if attempt == RETRIES:
                    raise ClaimError(f"{ref} を push できません: {e}") from e
                time.sleep(RETRY_WAIT_SEC)
                continue
            if proc.returncode == 0:
                return True
            if "[rejected]" in proc.stderr:
                # stale info (lease) か fetch first (非 fast-forward)。どちらも先に取られている。
                return False
            if attempt == RETRIES:
                raise ClaimError(
                    f"{ref} を push できません: {proc.stderr.strip()[-500:]}"
                )
            time.sleep(RETRY_WAIT_SEC)
        raise AssertionError("unreachable")

    def _commit(self, problem_id: str) -> str:
        """この宣言の commit。親は無く、中身は空の tree。sha は message だけで決まる。"""
        empty_tree = _git(
            self.repo, "hash-object", "-w", "-t", "tree", os.devnull
        ).strip()
        message = f"claim {self.run_id} {self.scope} {self.cpu_model} {problem_id} job {self.job}"
        return _git(
            self.repo, "commit-tree", empty_tree, "-m", message, env=_COMMIT_ENV
        ).strip()


def list_refs(repo: Path = ROOT, remote: str = "origin") -> list[str]:
    """宣言の ref を全部。run をまたいで並ぶ。"""
    out = _git(repo, "ls-remote", "--refs", remote, f"{NAMESPACE}/*")
    return [line.partition("\t")[2] for line in out.splitlines() if "\t" in line]


def run_of(ref: str) -> str | None:
    """宣言の ref の run id の成分。名前空間の外の ref なら None。"""
    prefix = f"{NAMESPACE}/"
    if not ref.startswith(prefix):
        return None
    return ref[len(prefix) :].split("/", 1)[0]


def stale(
    refs: Iterable[str], run_id: str, *, active: Collection[str] = ()
) -> list[str]:
    """この run と、それより古い run の宣言。run id は増える整数なので大小で見る。

    整数に読めない run id (手元の試しなど) は、同じ id のものだけ消す。active に
    入っている run (まだ走っている) のものは、古くても残す。
    """
    found: list[str] = []
    for ref in refs:
        head = run_of(ref)
        if head is None or head in active:
            continue
        older = run_id.isdigit() and head.isdigit() and int(head) < int(run_id)
        if head == run_id or older:
            found.append(ref)
    return found


def clean(
    run_id: str,
    repo: Path = ROOT,
    remote: str = "origin",
    *,
    finished: Callable[[str], bool] | None = None,
) -> tuple[int, int]:
    """この run の宣言と、終わっている古い run の宣言を消す。(消した本数, 残した本数)。

    走っている run の宣言を消すと、そのジョブたちが同じ問題を取り直して二重に測る。
    concurrency group がモードごとに 2 つあるので、古い run がまだ走っている横で
    collect が動くことがある。古い run ごとに終わったかを 1 回見て (run_finished)、
    終わっていなければ残す。残したものは次の collect が見直す。消し損ねても宣言は
    run 単位の名前なので、次の run の邪魔にはならない。
    """
    refs = list_refs(repo, remote)
    older = {run_of(ref) for ref in stale(refs, run_id)} - {run_id, None}
    if finished is None:
        # 同じ repo と remote で見る。ROOT の remote で見ると、手元の試し (remote が
        # 別の場所) でも本物の GitHub に聞いてしまう。
        def finished(run: str) -> bool:
            return run_finished(run, repo, remote)

    active = {head for head in sorted(older) if head and not finished(head)}
    targets = stale(refs, run_id, active=active)
    for start in range(0, len(targets), DELETE_CHUNK):
        chunk = targets[start : start + DELETE_CHUNK]
        _git(repo, "push", "--quiet", remote, *(f":{ref}" for ref in chunk))
    kept = sum(1 for ref in refs if run_of(ref) in active)
    return len(targets), kept


def repo_slug(repo: Path = ROOT, remote: str = "origin") -> str | None:
    """owner/name。CI の GITHUB_REPOSITORY か、remote の URL から。

    remote の host は SSH の別名 (github.com.hashiryo) が挟まって当てにならないので、
    site.build.repo_url と同じく github.com を含むことだけ確かめて末尾の 2 つを拾う。
    """
    slug = os.environ.get("GITHUB_REPOSITORY")
    if slug:
        return slug
    try:
        url = _git(repo, "remote", "get-url", remote).strip()
    except ClaimError:
        return None
    if "github.com" not in url:
        return None
    parts = url.rstrip("/").removesuffix(".git").replace(":", "/").split("/")
    return "/".join(parts[-2:]) if len(parts) >= 2 and all(parts[-2:]) else None


def run_finished(run_id: str, repo: Path = ROOT, remote: str = "origin") -> bool:
    """GitHub Actions の run が終わっているか。分からなければ False (残す側に倒す)。

    gh で REST を 1 回叩く。認証は CI では GH_TOKEN (github.token)、手元では gh の
    ログイン。1 run に 1 回なので GITHUB_TOKEN の 1000 req/h には触らない。run が
    消えている (404、保持期間切れ) なら終わっている扱い。
    """
    slug = repo_slug(repo, remote)
    if slug is None or not run_id.isdigit():
        return False
    try:
        proc = subprocess.run(
            ["gh", "api", f"repos/{slug}/actions/runs/{run_id}", "--jq", ".status"],
            capture_output=True,
            text=True,
            timeout=GIT_TIMEOUT_SEC,
            check=False,
        )
    except (subprocess.SubprocessError, OSError):
        return False
    if proc.returncode != 0:
        return "HTTP 404" in proc.stderr
    return proc.stdout.strip() == "completed"
