"""yukicoder の API からテストデータを取る。

Library/scripts/lib/download.py の download_yukicoder を移植した。
API トークンが要る。https://yukicoder.me/users/edit で発行して、
YUKICODER_TOKEN に入れる。
"""

from __future__ import annotations

import io
import os
import shutil
import sys
import urllib.error
import urllib.request
import zipfile
from pathlib import Path

from ..problem import Problem
from . import FetchError, replace_dir

TOKEN_ENV = "YUKICODER_TOKEN"
TIMEOUT_SEC = 120


def token() -> str | None:
    return os.environ.get(TOKEN_ENV) or None


def fetch(problem: Problem, dest: Path) -> None:
    value = token()
    if not value:
        raise FetchError(f"yukicoder には {TOKEN_ENV} が要ります")

    number = problem.testdata.name
    url = f"https://yukicoder.me/problems/no/{number}/testcase.zip"
    request = urllib.request.Request(url)
    request.add_header("Authorization", f"Bearer {value}")

    print(f"  yukicoder #{number} を落とします...", file=sys.stderr)
    try:
        with urllib.request.urlopen(request, timeout=TIMEOUT_SEC) as resp:
            payload = resp.read()
    except urllib.error.HTTPError as e:
        raise FetchError(f"yukicoder #{number}: HTTP {e.code} {e.reason}") from e
    except Exception as e:
        raise FetchError(f"yukicoder #{number}: {e}") from e

    tmp_dir = Path(str(dest) + ".tmp")
    if tmp_dir.exists():
        shutil.rmtree(tmp_dir)
    tmp_dir.mkdir(parents=True)

    try:
        with zipfile.ZipFile(io.BytesIO(payload)) as zf:
            inputs, outputs = {}, {}
            for name in zf.namelist():
                if name.endswith("/"):
                    continue
                if name.startswith("test_in/"):
                    inputs[Path(name).stem] = zf.read(name)
                elif name.startswith("test_out/"):
                    outputs[Path(name).stem] = zf.read(name)
            count = 0
            for case, data in inputs.items():
                if case not in outputs:
                    continue
                (tmp_dir / f"{case}.in").write_bytes(data)
                (tmp_dir / f"{case}.out").write_bytes(outputs[case])
                count += 1
    except (zipfile.BadZipFile, OSError) as e:
        shutil.rmtree(tmp_dir)
        raise FetchError(f"yukicoder #{number}: zip を展開できません: {e}") from e

    if count == 0:
        shutil.rmtree(tmp_dir)
        raise FetchError(f"yukicoder #{number}: ケースが 1 件もありません")
    replace_dir(tmp_dir, dest)
