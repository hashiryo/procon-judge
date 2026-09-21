"""include 閉包の解決。

`#include "..."` だけを再帰的に辿る。角括弧の include は辿らない。
探索パスはコンパイル時の -I と同じ順にする。ずれていると閉包が欠けて、
ライブラリを直しても再実行されなくなる。
"""

from __future__ import annotations

import re
from collections.abc import Sequence
from dataclasses import dataclass
from pathlib import Path

from .paths import ROOT

# 行頭から (空白を挟んで) の #include "..." だけを拾う。
# // でコメントアウトされた行は先頭が # にならないので当たらない。
# ブロックコメントの中は見分けられないが、字句解析器を書くほどの話ではない。
INCLUDE_RE = re.compile(r'^[ \t]*#[ \t]*include[ \t]*"([^"]+)"', re.MULTILINE)


@dataclass(frozen=True)
class Closure:
    """entry から辿れた include の集合。entry 自身は含まない。"""

    files: tuple[Path, ...]
    labels: tuple[str, ...]
    unresolved: tuple[str, ...]


def scan(text: str) -> list[str]:
    """引用符の include を書かれた順に返す。"""
    return INCLUDE_RE.findall(text)


def label_for(path: Path, search_paths: Sequence[Path]) -> str:
    """ハッシュと記録に使う、そのファイルの呼び名。

    探索パスの先頭から順に見て、最初に含んでいたものからの相対パスにする。
    lib/ が先頭なので、ライブラリのヘッダは `mylib/...` の形になる。
    どこから辿ったかで変わらないようにしたいので、include の書かれ方ではなく
    ファイルの位置だけで決める。
    """
    resolved = path.resolve()
    for root in search_paths:
        try:
            return resolved.relative_to(root.resolve()).as_posix()
        except ValueError:
            continue
    try:
        return resolved.relative_to(ROOT).as_posix()
    except ValueError:
        return resolved.as_posix()


def _resolve(target: str, from_dir: Path, search_paths: Sequence[Path]) -> Path | None:
    """引用符の include を解決する。まず include した側のディレクトリを見る。"""
    for base in (from_dir, *search_paths):
        candidate = (base / target).resolve()
        if candidate.is_file():
            return candidate
    return None


def direct(entry: Path, search_paths: Sequence[Path]) -> tuple[str, ...]:
    """entry が直接 include しているもののラベル。解決できたものだけ。

    閉包の中で「その提出の主題」と「巻き込まれただけ」を分けるのに使う。
    提出が名指ししているヘッダが主題で、それ以外は経由。
    """
    entry = entry.resolve()
    try:
        text = entry.read_text(errors="replace")
    except OSError:
        return ()
    labels = []
    for target in scan(text):
        path = _resolve(target, entry.parent, search_paths)
        if path is not None and path != entry:
            label = label_for(path, search_paths)
            if label not in labels:
                labels.append(label)
    return tuple(labels)


def closure(entry: Path, search_paths: Sequence[Path]) -> Closure:
    """entry から辿れる include をすべて集める。

    循環は訪問済みで止める。解決できなかった include は捨てずに返す。
    third_party を入れていないときなど、落ちてほしくない場面があるため。
    """
    entry = entry.resolve()
    seen: set[Path] = {entry}
    found: list[Path] = []
    unresolved: list[str] = []
    queue: list[Path] = [entry]

    while queue:
        current = queue.pop()
        try:
            text = current.read_text(errors="replace")
        except OSError:
            continue
        for target in scan(text):
            path = _resolve(target, current.parent, search_paths)
            if path is None:
                if target not in unresolved:
                    unresolved.append(target)
                continue
            if path in seen:
                continue
            seen.add(path)
            found.append(path)
            queue.append(path)

    labelled = sorted(
        ((label_for(p, search_paths), p) for p in found), key=lambda pair: pair[0]
    )
    return Closure(
        files=tuple(p for _, p in labelled),
        labels=tuple(label for label, _ in labelled),
        unresolved=tuple(sorted(unresolved)),
    )
