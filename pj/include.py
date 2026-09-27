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

from .paths import ROOT, SIMDE_DIR
from .problem import Problem

# 行頭から (空白を挟んで) の #include "..." だけを拾う。
# // でコメントアウトされた行は先頭が # にならないので当たらない。
# ブロックコメントの中は見分けられないが、字句解析器を書くほどの話ではない。
INCLUDE_RE = re.compile(r'^[ \t]*#[ \t]*include[ \t]*"([^"]+)"', re.MULTILINE)
# 山括弧の #include <...>。閉包では辿らない。手元のヘッダをこれで読んでいないかを見るのに使う。
ANGLE_RE = re.compile(r"^[ \t]*#[ \t]*include[ \t]*<([^>]+)>")


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


def resolve(target: str, from_dir: Path, search_paths: Sequence[Path]) -> Path | None:
    """引用符の include を解決する。まず include した側のディレクトリを見る。

    pj bundle の展開もこれで探す。閉包と展開で探し方がずれると、測ったものと
    判定サイトへ出すものが別のファイルを読む。
    """
    for base in (from_dir, *search_paths):
        candidate = (base / target).resolve()
        if candidate.is_file():
            return candidate
    return None


def direct(entry: Path, search_paths: Sequence[Path]) -> tuple[str, ...]:
    """entry が直接 include しているもののラベル。解決できたものだけ。

    閉包の中で「直接 include しているもの」と「そこから辿って間接に入るもの」を
    分けるのに使う。
    """
    entry = entry.resolve()
    try:
        text = entry.read_text(errors="replace")
    except OSError:
        return ()
    labels = []
    for target in scan(text):
        path = resolve(target, entry.parent, search_paths)
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
            path = resolve(target, current.parent, search_paths)
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


@dataclass(frozen=True)
class AngleInclude:
    """山括弧で読んでいる手元のヘッダ。"""

    path: Path
    line: int
    target: str


class AngleIncludes:
    """問題のハーネスと提出が、山括弧で読んでいる手元のヘッダを探す。閉包の中も見る。

    コンパイラは山括弧でも -I の中を探すので組めてしまうが、閉包は引用符しか辿らない。
    そのヘッダはキーに入らず、直しても測り直されない。pj bundle も展開しないので、判定
    サイトでは CE になる。SIMDe は山括弧で読む決まりなので、SIMDe の中で見つかるものと、
    SIMDe のファイルの中の include は見ない。

    ファイルの中身は問題をまたいで使い回す。pj problems check が全問題を見るとき、
    Library のヘッダを提出ごとに読み直すと、それだけで数秒かかる。
    """

    def __init__(self) -> None:
        self._lines: dict[Path, tuple[tuple[str, ...], tuple[tuple[int, str], ...]]] = {}
        self._exists: dict[tuple[Path, str], bool] = {}

    def of(self, problem: Problem, search_paths: Sequence[Path]) -> list[AngleInclude]:
        simde = SIMDE_DIR.resolve()
        local = [d for d in search_paths if d.resolve() != simde]
        entries = [problem.dir / s for s in problem.submissions()]
        if problem.harness_kind == "base":
            entries.insert(0, problem.base_cpp)
        # 閉包と同じ辿り方で、全部の入口から一度に辿る。
        queue = [e.resolve() for e in entries]
        seen = set(queue)
        found: list[AngleInclude] = []
        while queue:
            path = queue.pop()
            if path.is_relative_to(simde):
                continue
            quoted, angled = self._scan(path)
            for target in quoted:
                child = resolve(target, path.parent, search_paths)
                if child is not None and child not in seen:
                    seen.add(child)
                    queue.append(child)
            for number, target in angled:
                if any(self._is_file(d, target) for d in local):
                    found.append(AngleInclude(path=path, line=number, target=target))
        return sorted(found, key=lambda a: (str(a.path), a.line))

    def _scan(self, path: Path) -> tuple[tuple[str, ...], tuple[tuple[int, str], ...]]:
        """引用符の include と、山括弧の include (行番号付き)。"""
        if path not in self._lines:
            try:
                text = path.read_text(errors="replace")
            except OSError:
                text = ""
            angled = tuple(
                (number, m.group(1))
                for number, line in enumerate(text.splitlines(), start=1)
                if (m := ANGLE_RE.match(line))
            )
            self._lines[path] = (tuple(scan(text)), angled)
        return self._lines[path]

    def _is_file(self, directory: Path, target: str) -> bool:
        key = (directory, target)
        if key not in self._exists:
            self._exists[key] = (directory / target).is_file()
        return self._exists[key]
