"""問題の題名を判定サイトから取って、problem.toml と突き合わせる。

題名は表示にしか使わないので鍵には入らないが、手で書くと違う名前を付けてしまう。
移植のときに AOJ の 36 問のうち 24 問と yukicoder の 3 問がそうなっていた。
判定サイトから取れるものは取って揃える。local や manual や none の問題には
判定サイトの名前が無いので見ない。LOJ は問題の API から取る (多くは zh_CN しか無い)。

AOJ の旧 API (judgeapi.u-aizu.ac.jp) は 410 Gone で消えている。新しいサイトの
一覧の API を 1000 件ずつ舐めて、id から名前を引く。
"""

from __future__ import annotations

import html
import json
import re
import time
import tomllib
import urllib.request
from pathlib import Path

from .paths import LIBRARY_CHECKER_DIR
from .problem import Problem

TIMEOUT_SEC = 60
AOJ_LIST = "https://onlinejudge.u-aizu.ac.jp/api/problems?page={page}&size={size}"
AOJ_PAGE_SIZE = 1000
YUKICODER = "https://yukicoder.me/api/v1/problems/no/{number}"
LIBRARY_CHECKER_RAW = (
    "https://raw.githubusercontent.com/yosupo06/library-checker-problems/master/"
    "{name}/info.toml"
)


class TitleError(Exception):
    """題名を取れないか、書き戻せないときに投げる。"""


def _get_text(url: str) -> str:
    try:
        with urllib.request.urlopen(url, timeout=TIMEOUT_SEC) as resp:
            return resp.read().decode("utf-8")
    except Exception as e:  # 呼ぶ側で問題ごとに報告する
        raise TitleError(f"{url}: {e}") from e


# AtCoder は続けて叩くと 429 を返す (133 問の取り込みで 72 問が落ちた)。要求の間隔を空ける。
PAGE_INTERVAL_SEC = 1.5
_last_page_at = 0.0


def _get_html(url: str) -> str:
    """ページの HTML。AtCoder は UA の無い要求を通さないので付け、間隔も空ける。"""
    global _last_page_at
    wait = _last_page_at + PAGE_INTERVAL_SEC - time.monotonic()
    if wait > 0:
        time.sleep(wait)
    _last_page_at = time.monotonic()
    request = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0 (procon-judge)"})
    try:
        with urllib.request.urlopen(request, timeout=TIMEOUT_SEC) as resp:
            return resp.read().decode("utf-8", errors="replace")
    except Exception as e:
        raise TitleError(f"{url}: {e}") from e


def atcoder_title(url: str) -> str:
    """AtCoder の問題ページの <title>。API が無いのでページから取る。"""
    m = re.search(r"<title>(.*?)</title>", _get_html(url), re.DOTALL)
    if not m or not m.group(1).strip():
        raise TitleError(f"{url}: <title> が読めません")
    return html.unescape(m.group(1)).strip()


def _get_json(url: str):
    try:
        return json.loads(_get_text(url))
    except json.JSONDecodeError as e:
        raise TitleError(f"{url}: JSON を読めません: {e}") from e


def plain(title: str) -> str:
    """Library Checker の題名の TeX を落とす。`$\\#_p$ Subset Sum` は `#p Subset Sum`。"""
    return re.sub(r"[$\\_{}]", "", title).strip()


class Titles:
    """取得元ごとの題名。AOJ の一覧は 1 回だけ取って使い回す。"""

    def __init__(self) -> None:
        self._aoj: dict[str, dict] | None = None

    def official(self, problem: Problem) -> str | None:
        """判定サイトの名前。判定サイトの問題でなければ None。"""
        source, name = problem.testdata.source, problem.testdata.name
        if source == "aoj":
            names = self.aoj_names()
            if name not in names:
                raise TitleError(f"AOJ の一覧に {name} がありません")
            return names[name]
        if source == "yukicoder":
            data = _get_json(YUKICODER.format(number=name))
            title = data.get("Title") if isinstance(data, dict) else None
            if not title:
                raise TitleError(f"yukicoder #{name} の Title が読めません")
            return title
        if source == "library_checker":
            return plain(_library_checker_title(name))
        if source == "loj":
            from .fetch import FetchError, loj

            try:
                return loj.title(name)
            except FetchError as e:
                raise TitleError(str(e)) from e
        url = getattr(problem, "url", "")
        if url and "atcoder.jp/contests/" in url:
            # ケースが取れないので source は none だが、題名はページから取れる。
            return atcoder_title(url)
        return None

    def aoj_entries(self) -> dict[str, dict]:
        """AOJ の一覧を id で引く形にしたもの。名前のほかに制限も持っている。"""
        if self._aoj is None:
            entries: dict[str, dict] = {}
            for page in range(100):
                chunk = _get_json(AOJ_LIST.format(page=page, size=AOJ_PAGE_SIZE))
                if not isinstance(chunk, list):
                    raise TitleError("AOJ の一覧が配列ではありません")
                for entry in chunk:
                    entries[str(entry["id"])] = entry
                if len(chunk) < AOJ_PAGE_SIZE:
                    break
            self._aoj = entries
        return self._aoj

    def aoj_names(self) -> dict[str, str]:
        return {pid: str(entry["name"]) for pid, entry in self.aoj_entries().items()}

    def aoj_limits(self, name: str) -> tuple[float, int] | None:
        """AOJ の制限 (秒, MB)。一覧の problemTimeLimit は秒、problemMemoryLimit は KB。"""
        entry = self.aoj_entries().get(name)
        if entry is None:
            return None
        try:
            return float(entry["problemTimeLimit"]), int(entry["problemMemoryLimit"]) // 1024
        except (KeyError, TypeError, ValueError):
            return None

    def loj_limits(self, name: str) -> tuple[float, int] | None:
        """LOJ の制限 (秒, MB)。問題の API から取る。取れなければ None。"""
        from .fetch import FetchError, loj

        try:
            return loj.limits(name)
        except FetchError:
            return None


def _library_checker_title(name: str) -> str:
    """手元に clone があればそこから、無ければ GitHub の raw から info.toml を読む。"""
    local = LIBRARY_CHECKER_DIR / name / "info.toml"
    text = local.read_text() if local.is_file() else _get_text(
        LIBRARY_CHECKER_RAW.format(name=name)
    )
    try:
        title = tomllib.loads(text).get("title")
    except tomllib.TOMLDecodeError as e:
        raise TitleError(f"{name}/info.toml を読めません: {e}") from e
    if not title:
        raise TitleError(f"{name}/info.toml に title がありません")
    return str(title)


def toml_string(value: str) -> str:
    """TOML の基本文字列。引用符とバックスラッシュだけ逃がす。"""
    return '"' + value.replace("\\", "\\\\").replace('"', '\\"') + '"'


def rewrite_title(toml_path: Path, title: str) -> None:
    """problem.toml の title の行だけを書き換える。他の行には触らない。"""
    text = toml_path.read_text()
    # 置き換えは関数で渡す。文字列で渡すとバックスラッシュを解釈されてしまう。
    new, count = re.subn(
        r"^title = .*$",
        lambda _: "title = " + toml_string(title),
        text,
        count=1,
        flags=re.MULTILINE,
    )
    if count != 1:
        raise TitleError(f"{toml_path}: title の行が見つかりません")
    toml_path.write_text(new)
