"""yosupo06/library-checker-problems からテストデータを生成する。

問題を clone してジェネレータを回すので、判定サイトの API は叩かない。
Library/scripts/lib/download.py の download_yosupo を移植した。

clone は testdata.toml に書いたコミットに固定する。master の HEAD を追うと、
上流がジェネレータを直したときにケースの中身が変わり、cases_hash が動いて
その問題の記録が黙って測り直しになる。pin を動かすのは意図的な操作で、
そのときも中身が変わった問題だけが測り直しになる。ジェネレータは seed 固定
なので、同じコミットなら同じバイト列が出る (partition_function で実測した)。

生成したものは保管庫にも置く。作り直せるものではあるが、1 問 100 MB 前後で
130 問なら 10 GB を超え、actions/cache には載らない。再現性は pin が持ち、
速さと問題ごとの粒度は保管庫が持つ、という分担にする。
"""

from __future__ import annotations

import platform
import re
import shlex
import shutil
import subprocess
import sys
import tomllib
from pathlib import Path

from ..paths import LIBRARY_CHECKER_DIR, TESTDATA_TOML
from ..problem import Problem
from . import FetchError, replace_dir

REPO_URL = "https://github.com/yosupo06/library-checker-problems.git"
GIT_TIMEOUT_SEC = 300
GENERATE_TIMEOUT_SEC = 1800

# generate.py が要求する追加の依存。リポジトリの requirements.txt と揃える。
GENERATE_DEPS = ("colorlog",)

# manifest に書く、生成に使った上流のコミットの項目名。
UPSTREAM_KEY = "upstream_commit"

_SHA = re.compile(r"^[0-9a-f]{40}$")


def pinned_commit(path: Path = TESTDATA_TOML) -> str:
    """testdata.toml に書いた library-checker-problems のコミット。"""
    try:
        data = tomllib.loads(path.read_text())
    except OSError as e:
        raise FetchError(f"{path} を読めません: {e}") from e
    except tomllib.TOMLDecodeError as e:
        raise FetchError(f"{path} を読めません: {e}") from e
    commit = str(data.get("library_checker", {}).get("commit", ""))
    if not _SHA.match(commit):
        raise FetchError(
            f"{path}: library_checker.commit は 40 桁のコミットにしてください ({commit!r})"
        )
    return commit


def is_current(manifest: dict) -> bool:
    """この manifest のテストデータが、いまの pin で作ったものか。

    古い pin で作ったものは、中身が同じでも作り直す。同じ中身なら cases_hash も
    同じなので測り直しにはならず、manifest の上流のコミットが更新されるだけ。
    """
    return manifest.get(UPSTREAM_KEY) == pinned_commit()


def _git(directory: Path, *args: str) -> str:
    try:
        proc = subprocess.run(
            ["git", "-C", str(directory), *args],
            check=True, capture_output=True, text=True, timeout=GIT_TIMEOUT_SEC,
        )
    except subprocess.CalledProcessError as e:
        raise FetchError(
            f"git {' '.join(args)} が失敗しました: {e.stderr.strip()}"
        ) from e
    except (subprocess.SubprocessError, OSError) as e:
        raise FetchError(f"git {' '.join(args)} が動きません: {e}") from e
    return proc.stdout.strip()


def head(directory: Path = LIBRARY_CHECKER_DIR) -> str | None:
    if not (directory / ".git").is_dir():
        return None
    try:
        return _git(directory, "rev-parse", "HEAD")
    except FetchError:
        return None


def ensure_repo(directory: Path = LIBRARY_CHECKER_DIR, commit: str | None = None) -> Path:
    """clone を pin のコミットに合わせて返す。

    GitHub はコミットを名指しで fetch できるので、履歴は 1 段しか取らない
    (2.7 MB で 1 秒ほど)。別のコミットで生成すると、pin が約束する中身と
    ずれたものが保管庫へ上がるので、合わせられなければ失敗にする。
    """
    commit = commit or pinned_commit()
    if (directory / ".git").is_dir():
        if head(directory) == commit:
            return directory
        print(f"  library-checker-problems を {commit[:7]} に合わせます", file=sys.stderr)
        _git(directory, "fetch", "--depth=1", "origin", commit)
    else:
        directory.mkdir(parents=True, exist_ok=True)
        print(f"  library-checker-problems ({commit[:7]}) を取ります...", file=sys.stderr)
        _git(directory, "init", "-q")
        _git(directory, "remote", "add", "origin", REPO_URL)
        _git(directory, "fetch", "--depth=1", "origin", commit)
    # 手元の変更は捨てる。clone はキャッシュで、編集する場所ではない。
    _git(directory, "checkout", "-q", "--force", "--detach", commit)
    return directory


def problem_dir(name: str, repo: Path = LIBRARY_CHECKER_DIR) -> Path | None:
    """`data_structure/unionfind` のような name か、`unionfind` だけから問題の場所を引く。

    URL に出るのは問題名だけでカテゴリが無い。カテゴリは clone を見て補う。
    """
    if "/" in name:
        candidate = repo / name
        return candidate if (candidate / "info.toml").is_file() else None
    found = sorted(p.parent for p in repo.glob(f"*/{name}/info.toml"))
    return found[0] if found else None


def read_info(directory: Path) -> dict:
    try:
        return tomllib.loads((directory / "info.toml").read_text())
    except (OSError, tomllib.TOMLDecodeError) as e:
        raise FetchError(f"{directory}/info.toml を読めません: {e}") from e


def _generate_command(info_toml: Path) -> list[str]:
    """generate.py を走らせるコマンド。依存は uv が用意する。

    Linux ではスタックを広げてから呼ぶ。深い再帰を書いたジェネレータがあり、
    既定の 8 MB では落ちる。generate.py は Darwin と Windows のスタックだけ
    自分で面倒を見るので、Linux はこちらで用意する。macOS で
    `ulimit -s unlimited` は通らないので囲まない。
    """
    cmd = ["uv", "run", "--quiet"]
    for dep in GENERATE_DEPS:
        cmd += ["--with", dep]
    cmd += ["python", "generate.py", str(info_toml)]
    if platform.system() == "Linux":
        return ["bash", "-c", "ulimit -s unlimited && exec " + shlex.join(cmd)]
    return cmd


def fetch(problem: Problem, dest: Path) -> dict:
    """生成して dest に置く。manifest に足す項目 (生成に使ったコミット) を返す。"""
    repo = ensure_repo()
    commit = head(repo) or pinned_commit()
    name = problem.testdata.name
    directory = repo / name
    info_toml = directory / "info.toml"
    if not info_toml.is_file():
        raise FetchError(
            f"library-checker に {name!r} がありません ({info_toml})"
        )

    print(f"  {name} を生成します...", file=sys.stderr)
    try:
        subprocess.run(
            _generate_command(info_toml),
            cwd=repo, check=True, timeout=GENERATE_TIMEOUT_SEC,
        )
    except (subprocess.SubprocessError, OSError) as e:
        raise FetchError(f"{name} の生成に失敗しました: {e}") from e

    in_dir, out_dir = directory / "in", directory / "out"
    if not in_dir.is_dir():
        raise FetchError(f"{in_dir} ができていません")

    tmp_dir = Path(str(dest) + ".tmp")
    if tmp_dir.exists():
        shutil.rmtree(tmp_dir)
    tmp_dir.mkdir(parents=True)

    # コピーではなく移す。clone の側に残すと同じものを 2 度持つことになり、
    # 12 GB 出る問題もあるのでランナーの disk が尽きる。generate.py は in/out が
    # 揃っていなければ作り直すので、移してしまって困らない。
    count = 0
    for in_file in sorted(in_dir.glob("*.in")):
        out_file = out_dir / f"{in_file.stem}.out"
        if out_file.is_file():
            shutil.move(str(in_file), tmp_dir / in_file.name)
            shutil.move(str(out_file), tmp_dir / out_file.name)
            count += 1
    shutil.rmtree(in_dir, ignore_errors=True)
    shutil.rmtree(out_dir, ignore_errors=True)

    if count == 0:
        shutil.rmtree(tmp_dir)
        raise FetchError(f"{name} のケースが 1 件もありません")

    _copy_checker_assets(directory, repo, tmp_dir)
    replace_dir(tmp_dir, dest)
    return {UPSTREAM_KEY: commit}


def _copy_checker_assets(problem_dir: Path, repo: Path, dest: Path) -> None:
    """checker.cpp とそれが必要とするヘッダを揃える。

    checker.cpp は "testlib.h" と、生成で作られる "params.h" を include する。
    testlib.h が無いとコンパイルに失敗して、複数解 OK の問題を取りこぼす。
    """
    checker = problem_dir / "checker.cpp"
    if not checker.is_file():
        return
    shutil.copy2(checker, dest / "checker.cpp")
    for pattern in ("*.h", "*.hpp"):
        for header in problem_dir.glob(pattern):
            shutil.copy2(header, dest / header.name)
    testlib = repo / "common" / "testlib.h"
    if testlib.is_file():
        shutil.copy2(testlib, dest / "testlib.h")
