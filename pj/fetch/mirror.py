"""テストデータの保管庫。

private リポジトリの Release アセットに 1 問 1 アセットで置く。リアルタイムで
取得できない取得元があるうえ、取得できるものでも毎回原本を叩くとレート制限と
障害が経路に入る。一度落としたものはここから取る。

認証は fine-grained PAT 1 本。CI では secret の TESTDATA_TOKEN が渡る。
手元で使うときは同じ名前の環境変数に入れる。gh の keyring 認証は使わない
(別アカウントで認証されていて、この private リポジトリが見えない)。
"""

from __future__ import annotations

import json
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

from ..problem import Problem

REPO = os.environ.get("PJ_MIRROR_REPO", "hashiryo/procon-judge-testdata")
TAG = os.environ.get("PJ_MIRROR_TAG", "testdata")
TOKEN_ENV = "TESTDATA_TOKEN"

GH_TIMEOUT_SEC = 600

# ジェネレータから決定的に作れる取得元は保管しない。容量を食うだけで、
# 保管庫から取っても生成しても同じものが出る。
REGENERABLE_SOURCES = frozenset({"library_checker", "local"})

# アーカイブに入れるもの。checker.bin のような手元で作った成果物は入れない。
ARCHIVE_SUFFIXES = (".in", ".out")
ARCHIVE_EXTRA = ("manifest.json",)


class MirrorError(Exception):
    """保管庫とのやりとりで失敗したときに投げる。"""


def token() -> str | None:
    return os.environ.get(TOKEN_ENV) or None


def available() -> bool:
    return token() is not None and shutil.which("gh") is not None


def should_mirror(problem: Problem) -> bool:
    return problem.testdata.source not in REGENERABLE_SOURCES


def asset_name(problem: Problem) -> str:
    return f"{problem.id}.tar.zst"


def _gh(*args: str, check: bool = True) -> subprocess.CompletedProcess:
    value = token()
    if value is None:
        raise MirrorError(f"{TOKEN_ENV} が設定されていません")
    env = dict(os.environ)
    # keyring の認証を使わせない。別アカウントだと private リポジトリが見えない。
    env["GH_TOKEN"] = value
    env.pop("GITHUB_TOKEN", None)
    proc = subprocess.run(
        ["gh", *args, "-R", REPO],
        capture_output=True, text=True, check=False, env=env,
        timeout=GH_TIMEOUT_SEC,
    )
    if check and proc.returncode != 0:
        raise MirrorError(f"gh {' '.join(args)}: {proc.stderr.strip()}")
    return proc


def assets() -> list[dict]:
    """保管庫に置いてあるアセットの一覧。"""
    proc = _gh("release", "view", TAG, "--json", "assets")
    return json.loads(proc.stdout).get("assets", [])


_names: set[str] | None = None


def asset_names(*, refresh: bool = False) -> set[str]:
    """置いてあるアセットの名前。1 回引いて使い回す。

    問題ごとに問い合わせると、135 問で 135 往復になる。
    """
    global _names
    if _names is None or refresh:
        _names = {entry["name"] for entry in assets()}
    return _names


def _archive_members(directory: Path) -> list[str]:
    names = sorted(
        p.name
        for p in directory.iterdir()
        if p.is_file()
        and (p.suffix in ARCHIVE_SUFFIXES or p.name in ARCHIVE_EXTRA)
    )
    return names


def pack(directory: Path, out: Path) -> None:
    """in と out と manifest を tar.zst に固める。"""
    names = _archive_members(directory)
    if not names:
        raise MirrorError(f"{directory} に固めるものがありません")
    with tempfile.TemporaryDirectory() as tmp:
        tar = Path(tmp) / "archive.tar"
        _run(["tar", "-cf", str(tar), "-C", str(directory), *names])
        out.parent.mkdir(parents=True, exist_ok=True)
        _run(["zstd", "-q", "-f", "-19", "-T0", "-o", str(out), str(tar)])


def unpack(archive: Path, dest: Path) -> None:
    with tempfile.TemporaryDirectory() as tmp:
        tar = Path(tmp) / "archive.tar"
        _run(["zstd", "-d", "-q", "-f", "-o", str(tar), str(archive)])
        dest.mkdir(parents=True, exist_ok=True)
        _run(["tar", "-xf", str(tar), "-C", str(dest)])


def _run(cmd: list[str]) -> None:
    try:
        proc = subprocess.run(
            cmd, capture_output=True, text=True, check=False, timeout=GH_TIMEOUT_SEC
        )
    except (subprocess.SubprocessError, OSError) as e:
        raise MirrorError(f"{cmd[0]} が動きません: {e}") from e
    if proc.returncode != 0:
        raise MirrorError(f"{' '.join(cmd)}: {proc.stderr.strip()}")


def push(problem: Problem, directory: Path, *, force: bool = False) -> None:
    """1 問ぶんを保管庫へ上げる。他のアセットには触らない。

    既にあるものは上げ直さない。--clobber は消してから上げるので、run の
    ジョブが同時に走ると、片方が消した先へもう片方が上げて 404 になり、
    アセットが無い状態で残ることがある。実際に yuki-649 がそうなって、
    翌日の実行が yukicoder の原本を叩き直していた。

    force は取り直したテストデータで置き換えたいときだけ。人が 1 本で叩く
    前提で、CI からは渡さない。
    """
    name = asset_name(problem)
    if not force and name in asset_names():
        return
    with tempfile.TemporaryDirectory() as tmp:
        archive = Path(tmp) / name
        pack(directory, archive)
        size = archive.stat().st_size
        args = ["release", "upload", TAG, str(archive)]
        if force:
            args.append("--clobber")
        proc = _gh(*args, check=False)
        if proc.returncode != 0:
            # --clobber なしの upload は、既にあると失敗する。何も消さないので、
            # 競り負けただけなら上がっている。
            if not force and name in asset_names(refresh=True):
                print(f"  保管庫は別のジョブが先に上げました: {name}", file=sys.stderr)
                return
            raise MirrorError(f"gh release upload: {proc.stderr.strip()}")
    asset_names().add(name)
    print(f"  保管庫へ上げました: {name} ({size} bytes)", file=sys.stderr)


def pull(problem: Problem, dest: Path) -> bool:
    """保管庫から 1 問ぶん落とす。取れなければ False を返す。

    まだ無いのか、あるのに取れなかったのかを区別して知らせる。どちらでも
    呼ぶ側は原本へ落ちるが、後者は保管庫を置いた目的と逆なので黙らない。
    """
    name = asset_name(problem)
    if name not in asset_names():
        print(f"  保管庫にはまだありません: {name}", file=sys.stderr)
        return False
    with tempfile.TemporaryDirectory() as tmp:
        tmp_path = Path(tmp)
        proc = _gh(
            "release", "download", TAG, "-p", name, "-D", str(tmp_path), check=False
        )
        archive = tmp_path / name
        if proc.returncode != 0 or not archive.is_file():
            print(
                f"  保管庫から取れませんでした: {name}: {proc.stderr.strip()}",
                file=sys.stderr,
            )
            return False
        staging = tmp_path / "unpacked"
        unpack(archive, staging)
        if not any(staging.glob("*.in")):
            raise MirrorError(f"{name} にケースが入っていません")
        if dest.exists():
            shutil.rmtree(dest)
        dest.parent.mkdir(parents=True, exist_ok=True)
        shutil.move(str(staging), str(dest))
    print(f"  保管庫から取りました: {name}", file=sys.stderr)
    return True
