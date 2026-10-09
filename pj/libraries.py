"""libraries.toml の読み込み。提出が include するライブラリの置き場と、外向きのリンク。

pj はライブラリが何かを知らない。閉包に出てきたラベルの接頭辞がここに合う
かどうかだけを見る。合えば説明ページへ飛ばし、ヘッダごとの逆引き JSON を出す。

dir と repo を持つ項目は取りに行くライブラリで、`pj libs fetch` が repo を dir に
取ってくる。lib/ (Library) は探索パスの先頭に固定で、ほかの dir は探索パスの最後に
足す (build.include_dirs)。足した dir の -I はキーの材料に入れない。ライブラリの中身は
閉包のハッシュを通してキーに入るので、-I を入れなくても変更は見落とさない。入れると
ライブラリを足した瞬間に全部のキーが変わって、全部が測り直しになる。
"""

from __future__ import annotations

import shutil
import subprocess
import sys
import tomllib
from collections.abc import Sequence
from dataclasses import dataclass
from pathlib import Path, PurePosixPath

from .paths import LIB_DIR, LIBRARIES_TOML, ROOT

GIT_TIMEOUT_SEC = 300


class LibrariesError(Exception):
    """libraries.toml が読めないとき、ライブラリを取れないときに投げる。"""


@dataclass(frozen=True)
class Library:
    name: str
    prefix: str
    page: str
    source: str
    # 置き場 (リポジトリのルート直下のディレクトリ名) と取得元。無い項目はリンクだけを持つ。
    dir: str | None = None
    repo: str | None = None

    @property
    def path(self) -> Path | None:
        return ROOT / self.dir if self.dir else None

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
        directory = entry.get("dir")
        # ルート直下の 1 段に限る。problems/ の中などに置くと探索パスが入れ子になって、
        # ラベルがどちらの置き場から付くかが並び順で変わる。
        if directory is not None and (
            not directory or "/" in directory or directory.startswith(".")
        ):
            raise LibrariesError(f"{path}: dir はルート直下のディレクトリ名にします: {entry}")
        if entry.get("repo") and directory is None:
            raise LibrariesError(f"{path}: repo を取る先の dir がありません: {entry}")
        libraries.append(
            Library(
                name=entry["name"],
                prefix=entry["prefix"],
                page=entry["page"],
                source=entry["source"],
                dir=directory,
                repo=entry.get("repo"),
            )
        )
    names = [lib.name for lib in libraries]
    dirs = [lib.dir for lib in libraries if lib.dir]
    if len(set(names)) != len(names) or len(set(dirs)) != len(dirs):
        raise LibrariesError(f"{path}: name か dir が重なっています")
    return libraries


def find(label: str, libraries: list[Library]) -> Library | None:
    """ラベルを持つライブラリ。接頭辞が長い方を優先する。"""
    owners = [lib for lib in libraries if lib.owns(label)]
    return max(owners, key=lambda lib: len(lib.prefix)) if owners else None


def extra_dirs() -> tuple[Path, ...]:
    """lib/ のほかに探索パスへ足すライブラリの置き場。libraries.toml の順。

    置き場が無くても外さない。有無で外すと、手元と CI で記録の cxxflags が変わる。
    存在しない -I はコンパイラが黙って無視する。
    """
    return tuple(
        lib.path for lib in load_all() if lib.path is not None and lib.path != LIB_DIR
    )


def missing_dirs() -> list[Path]:
    """置き場が無いライブラリ。lib/ と、libraries.toml に dir を持つもの。"""
    dirs = dict.fromkeys([LIB_DIR, *(lib.path for lib in load_all() if lib.path is not None)])
    return [d for d in dirs if not d.is_dir()]


def _git(directory: Path, *args: str) -> str:
    try:
        proc = subprocess.run(
            ["git", "-C", str(directory), *args],
            capture_output=True, text=True, timeout=GIT_TIMEOUT_SEC, check=False,
        )
    except (subprocess.SubprocessError, OSError) as e:
        raise LibrariesError(f"git {' '.join(args[:2])}: {e}") from e
    if proc.returncode != 0:
        raise LibrariesError(f"git {' '.join(args[:2])}: {proc.stderr.strip()[-500:]}")
    return proc.stdout.strip()


def fetch(library: Library, sha: str | None = None) -> str:
    """library を置き場に取ってきて、その HEAD を返す。sha が無ければ既定のブランチの先端。

    置き場が既にあれば触らずに、その HEAD を返す。手元では本人の作業ツリーへの
    シンボリックリンクのことがあり、消したり書き換えたりすると本人の作業を壊す。
    GitHub は到達できる commit を名指しで fetch できるので、履歴は 1 段しか取らない。
    """
    path = library.path
    if path is None or not library.repo:
        raise LibrariesError(f"{library.name}: dir と repo が無いので取れません")
    if path.exists() or path.is_symlink():
        head = _git(path, "rev-parse", "HEAD")
        if sha and head != sha:
            print(
                f"warning: {library.dir}/ は既にあるので {sha[:12]} を取らずに "
                f"今の {head[:12]} を使います",
                file=sys.stderr,
            )
        return head
    path.mkdir(parents=True)
    try:
        _git(path, "init", "-q")
        _git(path, "remote", "add", "origin", library.repo)
        _git(path, "fetch", "-q", "--depth=1", "origin", sha or "HEAD")
        _git(path, "checkout", "-q", "FETCH_HEAD")
        return _git(path, "rev-parse", "HEAD")
    except LibrariesError:
        # 中途半端な置き場を残すと、HEAD の無い git の作業ツリーが探索パスに入る。
        # 消すのはここで作ったものだけ。
        shutil.rmtree(path, ignore_errors=True)
        raise


def fetch_all(
    libraries: Sequence[Library], shas: dict[str, str] | None = None
) -> tuple[dict[str, str], dict[str, str]]:
    """repo を持つライブラリを全部取る。(取れたものの HEAD, 取れなかったものの理由)。"""
    shas = shas or {}
    fetched: dict[str, str] = {}
    failed: dict[str, str] = {}
    for library in libraries:
        if not library.repo:
            continue
        try:
            fetched[library.name] = fetch(library, shas.get(library.name) or None)
        except LibrariesError as e:
            failed[library.name] = str(e)
    return fetched, failed
