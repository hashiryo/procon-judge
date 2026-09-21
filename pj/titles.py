"""問題の題名を判定サイトから取って、problem.toml と突き合わせる。

題名は表示にしか使わないので鍵には入らないが、手で書くと違う名前を付けてしまう。
移植のときに AOJ の 36 問のうち 24 問と yukicoder の 3 問がそうなっていた。
判定サイトから取れるものは取って揃える。local や manual や none の問題には
判定サイトの名前が無いので見ない。

AOJ の旧 API (judgeapi.u-aizu.ac.jp) は 410 Gone で消えている。新しいサイトの
一覧の API を 1000 件ずつ舐めて、id から名前を引く。
"""

from __future__ import annotations

import json
import re
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
        self._aoj: dict[str, str] | None = None

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
        return None

    def aoj_names(self) -> dict[str, str]:
        if self._aoj is None:
            names: dict[str, str] = {}
            for page in range(100):
                chunk = _get_json(AOJ_LIST.format(page=page, size=AOJ_PAGE_SIZE))
                if not isinstance(chunk, list):
                    raise TitleError("AOJ の一覧が配列ではありません")
                for entry in chunk:
                    names[str(entry["id"])] = str(entry["name"])
                if len(chunk) < AOJ_PAGE_SIZE:
                    break
            self._aoj = names
        return self._aoj


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
