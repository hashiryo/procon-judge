"""LOJ (LibreOJ) の API からテストデータを取る。

ログインは要らない。`getProblem` が `testData` (ファイルの一覧) と `judgeInfo`
(制限と、サブタスクごとの in/out の組) を返し、`downloadProblemFiles` がファイル
ごとの署名付き URL を返す。CPtools の loj_download.py は提出の結果からファイル名を
集めていたが、`getProblem` の `testData` で足りる。

Cloudflare が Python の既定の User-Agent を 403 で弾くので、UA を付ける。
"""

from __future__ import annotations

import functools
import json
import shutil
import sys
import time
import urllib.error
import urllib.request
from pathlib import Path, PurePosixPath

from ..problem import Problem
from . import FetchError, replace_dir

API = "https://api.loj.ac/api/problem/{method}"
TIMEOUT_SEC = 60
USER_AGENT = "Mozilla/5.0 (procon-judge)"
# downloadProblemFiles に一度に渡すファイル名の数。334 ファイルの問題 (loj-3165) がある。
DOWNLOAD_CHUNK = 100
# 出力ファイルの拡張子。サブタスクの無い問題で .in の相手を探すときに見る。
OUTPUT_SUFFIXES = (".out", ".ans")
# 接続が途中で切れる (SSL の EOF) ことがあるので、HTTP のエラー以外は少しだけやり直す。
RETRIES = 3
RETRY_WAIT_SEC = 2.0


def _read(request: urllib.request.Request, what: str) -> bytes:
    for attempt in range(1, RETRIES + 1):
        try:
            with urllib.request.urlopen(request, timeout=TIMEOUT_SEC) as resp:
                return resp.read()
        except urllib.error.HTTPError as e:
            raise FetchError(f"{what}: HTTP {e.code} {e.reason}") from e
        except Exception as e:
            if attempt == RETRIES:
                raise FetchError(f"{what}: {e}") from e
            time.sleep(RETRY_WAIT_SEC)
    raise AssertionError("unreachable")


def _post(method: str, payload: dict) -> dict:
    request = urllib.request.Request(
        API.format(method=method),
        data=json.dumps(payload).encode(),
        headers={
            "Content-Type": "application/json",
            "Accept": "application/json",
            "User-Agent": USER_AGENT,
        },
    )
    try:
        data = json.loads(_read(request, f"LOJ {method}"))
    except json.JSONDecodeError as e:
        raise FetchError(f"LOJ {method}: JSON を読めません: {e}") from e
    if not isinstance(data, dict):
        raise FetchError(f"LOJ {method}: 応答が JSON のオブジェクトではありません")
    if data.get("error"):
        raise FetchError(f"LOJ {method}: {data['error']}")
    return data


def _get(url: str) -> bytes:
    return _read(urllib.request.Request(url, headers={"User-Agent": USER_AGENT}), url)


@functools.cache
def problem_info(number: str) -> dict:
    """getProblem の応答。題名、制限、ファイルの一覧を持つ。

    題名と制限とテストデータで 3 回叩かないように、番号ごとに 1 回で覚える。
    """
    try:
        display_id = int(number)
    except ValueError:
        raise FetchError(f"LOJ の問題番号が数字ではありません: {number!r}") from None
    return _post(
        "getProblem",
        {
            "displayId": display_id,
            "testData": True,
            "judgeInfo": True,
            # en_US を頼んでも、無い問題は既定の言語 (ほぼ zh_CN) が返る。
            "localizedContentsOfLocale": "en_US",
        },
    )


def title(number: str) -> str:
    contents = problem_info(number).get("localizedContentsOfLocale") or {}
    name = contents.get("title")
    if not name:
        raise FetchError(f"LOJ {number}: 題名が読めません")
    return str(name)


def limits(number: str) -> tuple[float, int]:
    """判定サイトの制限 (秒, MB)。judgeInfo の timeLimit はミリ秒、memoryLimit は MB。"""
    judge = problem_info(number).get("judgeInfo") or {}
    try:
        return int(judge["timeLimit"]) / 1000, int(judge["memoryLimit"])
    except (KeyError, TypeError, ValueError):
        raise FetchError(
            f"LOJ {number}: judgeInfo に timeLimit / memoryLimit がありません"
        ) from None


def pairs(info: dict) -> list[tuple[str, str, str]]:
    """(ケース名, 入力ファイル, 出力ファイル) の一覧。

    サブタスクがあれば、その testcases の組をそのまま使う。同じケースが複数の
    サブタスクに出るので、入力ファイルで畳む。サブタスクの無い問題は、拡張子を
    除いた部分が同じ `.in` と `.out` (無ければ `.ans`) を組にする。ケース名は入力
    ファイルの拡張子を除いたもの。`1.000.in` は `1.000`、`input0.txt` は `input0`。
    """
    files = {f["filename"] for f in info.get("testData") or [] if f.get("filename")}
    judge = info.get("judgeInfo") or {}
    found: list[tuple[str, str]] = []
    seen: set[str] = set()
    for subtask in judge.get("subtasks") or []:
        for case in subtask.get("testcases") or []:
            in_file, out_file = case.get("inputFile"), case.get("outputFile")
            if not in_file or not out_file or in_file in seen:
                continue
            seen.add(in_file)
            found.append((in_file, out_file))
    if not found:
        for name in sorted(files):
            if not name.endswith(".in"):
                continue
            stem = name[: -len(".in")]
            for suffix in OUTPUT_SUFFIXES:
                if stem + suffix in files:
                    found.append((name, stem + suffix))
                    break
    missing = sorted({f for pair in found for f in pair if f not in files})
    if missing:
        raise FetchError(
            f"LOJ: サブタスクが指すファイルが一覧にありません: {', '.join(missing)}"
        )

    result: list[tuple[str, str, str]] = []
    names: set[str] = set()
    for in_file, out_file in found:
        name = PurePosixPath(in_file).stem
        if not name or name.startswith(".") or "/" in in_file:
            raise FetchError(f"LOJ: ケース名にできないファイル名です: {in_file!r}")
        if name in names:
            raise FetchError(f"LOJ: ケース名が重なります: {name!r}")
        names.add(name)
        result.append((name, in_file, out_file))
    return result


def fetch(problem: Problem, dest: Path) -> None:
    number = problem.testdata.name
    info = problem_info(number)
    cases = pairs(info)
    if not cases:
        raise FetchError(f"LOJ {number}: ケースの組が見つかりません")
    sizes = {f["filename"]: f.get("size") for f in info.get("testData") or []}
    # downloadProblemFiles に渡すのは表示の番号ではなく内部の id (loj-6787 は 40988)。
    try:
        internal_id = int(info["meta"]["id"])
    except (KeyError, TypeError, ValueError):
        raise FetchError(f"LOJ {number}: meta.id が読めません") from None

    print(f"  LOJ {number} を {len(cases)} ケース落とします...", file=sys.stderr)
    wanted = [f for _, in_file, out_file in cases for f in (in_file, out_file)]
    urls = download_urls(internal_id, wanted)

    tmp_dir = Path(str(dest) + ".tmp")
    if tmp_dir.exists():
        shutil.rmtree(tmp_dir)
    tmp_dir.mkdir(parents=True)
    mismatched: list[str] = []
    for name, in_file, out_file in cases:
        for src, suffix in ((in_file, ".in"), (out_file, ".out")):
            data = _get(urls[src])
            expected = sizes.get(src)
            # 一覧の size が無い (0 か null の) ファイルもある。あるときだけ照らす。
            if isinstance(expected, int) and expected > 0 and len(data) != expected:
                mismatched.append(src)
            (tmp_dir / f"{name}{suffix}").write_bytes(data)
    if mismatched:
        print(
            f"  warning: LOJ {number}: 一覧の大きさと違うファイルがあります "
            f"({', '.join(mismatched)})",
            file=sys.stderr,
        )
    replace_dir(tmp_dir, dest)


def download_urls(problem_id: int, filenames: list[str]) -> dict[str, str]:
    """ファイル名から署名付き URL へ。"""
    urls: dict[str, str] = {}
    for start in range(0, len(filenames), DOWNLOAD_CHUNK):
        chunk = filenames[start : start + DOWNLOAD_CHUNK]
        data = _post(
            "downloadProblemFiles",
            {"problemId": problem_id, "type": "TestData", "filenameList": chunk},
        )
        for entry in data.get("downloadInfo") or []:
            urls[entry["filename"]] = entry["downloadUrl"]
    missing = [f for f in filenames if f not in urls]
    if missing:
        raise FetchError(
            f"LOJ: ダウンロード URL が返ってきません: {', '.join(missing[:5])}"
        )
    return urls
