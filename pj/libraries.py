"""libraries.toml の読み込み。提出が include するライブラリへの外向きのリンク。

pj はライブラリが何かを知らない。閉包に出てきたラベルの接頭辞がここに合う
かどうかだけを見る。合えば説明ページへ飛ばし、ヘッダごとの逆引き JSON を出す。
"""

from __future__ import annotations

import tomllib
from dataclasses import dataclass
from pathlib import Path, PurePosixPath

from .paths import LIBRARIES_TOML


class LibrariesError(Exception):
    """libraries.toml が読めないときに投げる。"""


@dataclass(frozen=True)
class Library:
    name: str
    prefix: str
    page: str
    source: str

    def owns(self, label: str) -> bool:
        return label.startswith(self.prefix)

    def page_url(self, label: str) -> str:
        return self._format(self.page, label, sha="HEAD")

    def source_url(self, label: str, sha: str | None) -> str:
        return self._format(self.source, label, sha=sha or "HEAD")

    def _format(self, template: str, label: str, *, sha: str) -> str:
        path = label[len(self.prefix) :]
        stem = str(PurePosixPath(path).with_suffix(""))
        return template.format(path=path, stem=stem, sha=sha)


def load_all(path: Path = LIBRARIES_TOML) -> list[Library]:
    """定義を読む。ファイルが無ければ空。ライブラリを使わない使い方もある。"""
    if not path.is_file():
        return []
    with path.open("rb") as f:
        raw = tomllib.load(f)
    libraries = []
    for entry in raw.get("library", []):
        for key in ("name", "prefix", "page", "source"):
            if key not in entry:
                raise LibrariesError(f"{path}: library に {key} がありません: {entry}")
        if not entry["prefix"]:
            raise LibrariesError(f"{path}: prefix が空です: {entry}")
        libraries.append(
            Library(
                name=entry["name"],
                prefix=entry["prefix"],
                page=entry["page"],
                source=entry["source"],
            )
        )
    return libraries


def find(label: str, libraries: list[Library]) -> Library | None:
    """ラベルを持つライブラリ。接頭辞が長い方を優先する。"""
    owners = [lib for lib in libraries if lib.owns(label)]
    return max(owners, key=lambda lib: len(lib.prefix)) if owners else None
