"""yosupo06/library-checker-problems からテストデータを生成する。

問題を clone してジェネレータを回すので、判定サイトの API は叩かない。
Library/scripts/lib/download.py の download_yosupo を移植した。
"""

from __future__ import annotations

import shutil
import subprocess
import sys
from pathlib import Path

from ..paths import LIBRARY_CHECKER_DIR
from ..problem import Problem
from . import FetchError, replace_dir

REPO_URL = "https://github.com/yosupo06/library-checker-problems.git"
CLONE_TIMEOUT_SEC = 300
GENERATE_TIMEOUT_SEC = 1800

# generate.py が要求する追加の依存。リポジトリの requirements.txt と揃える。
GENERATE_DEPS = ("colorlog",)


def ensure_repo(directory: Path = LIBRARY_CHECKER_DIR) -> Path:
    if (directory / ".git").is_dir():
        try:
            subprocess.run(
                ["git", "fetch", "--depth=1", "origin", "master"],
                cwd=directory, check=True, capture_output=True, timeout=120,
            )
            subprocess.run(
                ["git", "reset", "--hard", "origin/master"],
                cwd=directory, check=True, capture_output=True, timeout=60,
            )
        except (subprocess.SubprocessError, OSError) as e:
            # 更新できなくても手元の clone で生成はできる。
            print(f"  library-checker-problems の更新に失敗しました: {e}", file=sys.stderr)
        return directory

    directory.parent.mkdir(parents=True, exist_ok=True)
    print("  library-checker-problems を clone します...", file=sys.stderr)
    try:
        subprocess.run(
            ["git", "clone", "--depth=1", REPO_URL, str(directory)],
            check=True, capture_output=True, timeout=CLONE_TIMEOUT_SEC,
        )
    except (subprocess.SubprocessError, OSError) as e:
        raise FetchError(f"library-checker-problems の clone に失敗しました: {e}") from e
    return directory


def _generate_command(info_toml: Path) -> list[str]:
    """generate.py を走らせるコマンド。依存は uv が用意する。"""
    cmd = ["uv", "run", "--quiet"]
    for dep in GENERATE_DEPS:
        cmd += ["--with", dep]
    cmd += ["python", "generate.py", str(info_toml)]
    return cmd


def fetch(problem: Problem, dest: Path) -> None:
    repo = ensure_repo()
    name = problem.testdata.name
    problem_dir = repo / name
    info_toml = problem_dir / "info.toml"
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

    in_dir, out_dir = problem_dir / "in", problem_dir / "out"
    if not in_dir.is_dir():
        raise FetchError(f"{in_dir} ができていません")

    tmp_dir = Path(str(dest) + ".tmp")
    if tmp_dir.exists():
        shutil.rmtree(tmp_dir)
    tmp_dir.mkdir(parents=True)

    count = 0
    for in_file in sorted(in_dir.glob("*.in")):
        out_file = out_dir / f"{in_file.stem}.out"
        if out_file.is_file():
            shutil.copy2(in_file, tmp_dir / in_file.name)
            shutil.copy2(out_file, tmp_dir / out_file.name)
            count += 1

    if count == 0:
        shutil.rmtree(tmp_dir)
        raise FetchError(f"{name} のケースが 1 件もありません")

    _copy_checker_assets(problem_dir, repo, tmp_dir)
    replace_dir(tmp_dir, dest)


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
