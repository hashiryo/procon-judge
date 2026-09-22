"""run のジョブが自分の仕事を宣言する。

宣言は procon-judge のリポジトリに ref を 1 本作ること。名前は
`refs/claims/<run id>/<環境>/<CPU モデル>/<問題 id>`。`refs/heads` の外なので
`on: push` は起きず、GitHub の画面にも出ない。

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
from collections.abc import Iterable
from dataclasses import dataclass
from pathlib import Path

from .paths import ROOT

NAMESPACE = "refs/claims"
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
    """1 つの run の、1 つの環境と CPU モデルについての宣言。"""

    run_id: str
    env: str
    cpu_model: str
    job: int = 0
    repo: Path = ROOT
    remote: str = "origin"

    @property
    def prefix(self) -> str:
        return (
            f"{NAMESPACE}/{slug(self.run_id)}/{slug(self.env)}/{slug(self.cpu_model)}/"
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
        message = f"claim {self.run_id} {self.env} {self.cpu_model} {problem_id} job {self.job}"
        return _git(
            self.repo, "commit-tree", empty_tree, "-m", message, env=_COMMIT_ENV
        ).strip()


def list_refs(repo: Path = ROOT, remote: str = "origin") -> list[str]:
    """宣言の ref を全部。run をまたいで並ぶ。"""
    out = _git(repo, "ls-remote", "--refs", remote, f"{NAMESPACE}/*")
    return [line.partition("\t")[2] for line in out.splitlines() if "\t" in line]


def stale(refs: Iterable[str], run_id: str) -> list[str]:
    """この run と、それより古い run の宣言。run id は増える整数なので大小で見る。

    整数に読めない run id (手元の試しなど) は、同じ id のものだけ消す。
    """
    prefix = f"{NAMESPACE}/"
    found: list[str] = []
    for ref in refs:
        if not ref.startswith(prefix):
            continue
        head = ref[len(prefix) :].split("/", 1)[0]
        older = run_id.isdigit() and head.isdigit() and int(head) < int(run_id)
        if head == run_id or older:
            found.append(ref)
    return found


def clean(run_id: str, repo: Path = ROOT, remote: str = "origin") -> int:
    """この run 以前の宣言を消す。消した本数を返す。

    消し損ねても宣言は run 単位の名前なので次の run の邪魔にはならない。次の
    collect が消す。
    """
    targets = stale(list_refs(repo, remote), run_id)
    for start in range(0, len(targets), DELETE_CHUNK):
        chunk = targets[start : start + DELETE_CHUNK]
        _git(repo, "push", "--quiet", remote, *(f":{ref}" for ref in chunk))
    return len(targets)
