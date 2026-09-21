"""キーの計算。

このキーの記録が既にあれば実行をスキップする。整形だけの変更で走り直さないよう、
ハッシュを取る前に C++ のソースをトークンの列に直す。コメントと空白は捨てる。

字句解析器はごく小さいものだが、間違えるなら「余計に測り直す」側に倒すように
作ってある。避けたいのは逆向きの誤りで、違うコードが同じトークン列になると
変更が見えなくなり、記録が静かに古くなる。それが起きうる場所が 3 つあって、
それぞれ塞いである。

- 文字列リテラルの中の // や /* をコメントと誤認すると、その中の変更が消える。
  だから文字列と文字のリテラルはコメントより先に判定する。raw string も扱う。
- 記号を 1 文字ずつにすると `a + +b` と `a ++b` が同じ列になる。だから
  コンパイラと同じ最長一致で切る。`>>` と `> >` が別扱いになるのは、余計に
  測り直すだけなので構わない。
- `#define F(x)` と `#define F (x)` は空白だけで意味が違う。だからプリプロセッサ
  の指令の行はトークンに分けず、コメントを外して空白を 1 つに潰した行のまま
  1 トークンにする。

C++ 以外のファイル (local のテストデータを作る Python など) には使わない。
Python は字下げに意味があるので、空白を捨てると違うコードが同じになる。
そちらは行末空白と空行を落とすだけの正規化に留める。

提出のパスもキーに入れる。submission_hash は中身だけから作るので、バイト単位で
同じ提出が 2 本あると同じ値になる。提出ページはパスごとに描くので、パスが違えば
別の記録が要る。
"""

from __future__ import annotations

import hashlib
import json
import re
from collections.abc import Iterator, Sequence
from dataclasses import dataclass
from pathlib import Path

from .include import Closure, closure, label_for
from .problem import Problem

SEP = "\0"
EMPTY_HASH = hashlib.sha256(b"").hexdigest()

# 記録に持つファイル別ハッシュの長さ。「同じか違うか」を見るだけなので短くてよい。
FILE_HASH_CHARS = 16

# この拡張子のファイルだけをトークン単位で正規化する。
CXX_SUFFIXES = frozenset(
    {".c", ".cc", ".cpp", ".cxx", ".c++", ".h", ".hh", ".hpp", ".hxx", ".h++",
     ".ipp", ".inl", ".tcc"}
)

# 先に書いた候補から順に試す。指令の行は行頭でしか始まらないので、行頭の空白を
# 空白として食ってしまう前に見る。リテラルは識別子より先に見て、u8"..." や
# R"(...)" の接頭辞を識別子と切り離さない。数は記号より先に見て、.5 を . と 5 に
# 分けない。
_TOKEN_RE = re.compile(
    r"""
      (?P<directive>^[ \t]*\#[^\n]*)
    | (?P<space>\s+)
    | (?P<comment>//[^\n]*|/\*.*?\*/)
    | (?P<raw>(?:u8|u|U|L)?R"(?P<delim>[^()\\\s]{0,16})\(.*?\)(?P=delim)"[A-Za-z0-9_]*)
    | (?P<string>(?:u8|u|U|L)?"(?:[^"\\\n]|\\.)*"[A-Za-z0-9_]*)
    | (?P<char>(?:u8|u|U|L)?'(?:[^'\\\n]|\\.)*'[A-Za-z0-9_]*)
    | (?P<number>\.?[0-9](?:[eEpP][+-]|'[A-Za-z0-9_]|[A-Za-z0-9_.])*)
    | (?P<ident>[A-Za-z_][A-Za-z0-9_]*)
    | (?P<punct><<=|>>=|\.\.\.|->\*|<=>|::|\.\*|->|\+=|-=|\*=|/=|%=|\^=|&=|\|=
                |==|!=|<=|>=|&&|\|\||<<|>>|\+\+|--|\#\#|.)
    """,
    re.VERBOSE | re.MULTILINE | re.DOTALL,
)

_RAW_PREFIXES = ("R", "u8R", "uR", "UR", "LR")


@dataclass(frozen=True)
class SubmissionKey:
    submission_hash: str
    includes: tuple[str, ...]
    unresolved: tuple[str, ...]
    # ラベル -> 正規化した中身の短いハッシュ。提出ファイル自身も入る。
    # 参考に落ちたとき、どのファイルが動いたかを名指しするために記録へ載せる。
    file_hashes: tuple[tuple[str, str], ...] = ()


@dataclass(frozen=True)
class HarnessKey:
    harness_hash: str
    file_hashes: tuple[tuple[str, str], ...] = ()


# --- 正規化 ------------------------------------------------------------------


def normalize(text: str) -> str:
    """行末空白除去 + 空行除去。C++ 以外のファイル用。"""
    lines = (line.rstrip() for line in text.splitlines())
    return "\n".join(line for line in lines if line)


def lex(text: str) -> Iterator[tuple[str, str]]:
    """ソースを (種類, 文字列) の列にする。空白もコメントも落とさず、全部繋ぐと元に戻る。

    種類は directive / space / comment / raw / string / char / number / ident /
    punct のどれか。正規化とサイトの色付けの両方がこれを使う。
    """
    for match in _TOKEN_RE.finditer(text):
        yield match.lastgroup or "punct", match.group()


def tokenize(text: str) -> list[str]:
    """C++ のソースをトークンの列にする。コメントと空白は落とす。"""
    # 行継続はプリプロセッサが最初に繋ぐので、ここでも先に繋ぐ。
    text = text.replace("\r\n", "\n").replace("\\\n", "")
    out: list[str] = []
    for kind, piece in lex(text):
        if kind in ("space", "comment"):
            continue
        out.append(_squash_directive(piece) if kind == "directive" else piece)
    return out


def normalize_cxx(text: str) -> str:
    """トークンの列を 1 行 1 トークンで繋いだもの。ハッシュはこれから取る。

    raw string の中の改行はトークンに残るが、raw string は必ず )delim" で
    閉じる形なので、切れ目を誤読することはない。
    """
    return "\n".join(tokenize(text))


def normalize_file(path: Path) -> str:
    """拡張子で正規化を選ぶ。読めなければ空文字。"""
    try:
        text = path.read_text(errors="replace")
    except OSError:
        return ""
    if path.suffix.lower() in CXX_SUFFIXES:
        return normalize_cxx(text)
    return normalize(text)


def _squash_directive(text: str) -> str:
    """指令の行を 1 トークンにする。コメントを外し、空白を 1 つに潰す。

    引用符の中の // や /* はコメントではないので、リテラルは丸ごと飛ばす。
    `#  define` と `#define` は同じものなので、# の直後の空白だけは消す。
    """
    out: list[str] = []
    pending_space = False
    i, n = 0, len(text)
    while i < n:
        c = text[i]
        if c in " \t\f\v":
            pending_space = True
            i += 1
            continue
        if text.startswith("//", i):
            break
        if text.startswith("/*", i):
            end = text.find("*/", i + 2)
            i = n if end < 0 else end + 2
            pending_space = True
            continue
        if c in "\"'":
            j = _literal_end(text, i, raw=_is_raw_prefix(text, i))
        else:
            j = i + 1
        if pending_space and out:
            out.append(" ")
        out.append(text[i:j])
        pending_space = False
        i = j
    joined = "".join(out)
    return "#" + joined[1:].lstrip(" ")


def _is_raw_prefix(text: str, quote: int) -> bool:
    """quote の位置の " の直前が raw string の接頭辞か。"""
    start = quote
    while start > 0 and (text[start - 1].isalnum() or text[start - 1] == "_"):
        start -= 1
    return text[start:quote] in _RAW_PREFIXES


def _literal_end(text: str, start: int, *, raw: bool) -> int:
    """start の引用符から始まるリテラルの終わりの次の位置。

    閉じていなければ行末 (raw string なら末尾) まで。ユーザー定義リテラルの
    接尾辞は続けて食う。`"a"_s` と `"a" _s` は別物なので分けない。
    """
    quote = text[start]
    n = len(text)
    if raw and quote == '"':
        paren = text.find("(", start)
        if paren < 0:
            return n
        closer = ")" + text[start + 1 : paren] + '"'
        end = text.find(closer, paren + 1)
        j = n if end < 0 else end + len(closer)
    else:
        j = start + 1
        while j < n:
            c = text[j]
            if c == "\\":
                j += 2
                continue
            if c == quote:
                j += 1
                break
            if c == "\n":
                break
            j += 1
    while j < n and (text[j].isalnum() or text[j] == "_"):
        j += 1
    return min(j, n)


# --- ハッシュ ----------------------------------------------------------------


def _sha256(*parts: str) -> str:
    return hashlib.sha256(SEP.join(parts).encode()).hexdigest()


def _short(normalized: str) -> str:
    return hashlib.sha256(normalized.encode()).hexdigest()[:FILE_HASH_CHARS]


def _hash_tree(
    entry: Path, found: Closure, search_paths: Sequence[Path]
) -> tuple[str, tuple[tuple[str, str], ...]]:
    """entry とその閉包から、全体のハッシュとファイル別ハッシュを作る。"""
    text = normalize_file(entry)
    parts = [text]
    files = [(label_for(entry, search_paths), _short(text))]
    for label, path in zip(found.labels, found.files, strict=True):
        text = normalize_file(path)
        parts.append(label)
        parts.append(text)
        files.append((label, _short(text)))
    return _sha256(*parts), tuple(files)


def submission_hash(
    source: Path, search_paths: Sequence[Path]
) -> SubmissionKey:
    """提出のソースと include 閉包からハッシュを作る。"""
    found: Closure = closure(source, search_paths)
    digest, files = _hash_tree(source, found, search_paths)
    return SubmissionKey(
        submission_hash=digest,
        includes=found.labels,
        unresolved=found.unresolved,
        file_hashes=files,
    )


def harness_key(problem: Problem, search_paths: Sequence[Path] = ()) -> HarnessKey:
    """base.cpp とその include 閉包からハッシュを作る。

    kind = "raw" の問題は base.cpp を持たないので空文字列のハッシュにする。

    閉包まで見るのは、共有のハーネスヘッダを書き換えたときに測り直しを
    起こすため。名指しで足していた問題ごとの common.hpp も、base.cpp が
    include していれば閉包から入る。
    """
    if problem.harness_kind != "base":
        return HarnessKey(harness_hash=EMPTY_HASH)
    found: Closure = closure(problem.base_cpp, search_paths)
    digest, files = _hash_tree(problem.base_cpp, found, search_paths)
    return HarnessKey(harness_hash=digest, file_hashes=files)


def harness_hash(problem: Problem, search_paths: Sequence[Path] = ()) -> str:
    return harness_key(problem, search_paths).harness_hash


def problem_hash(problem: Problem) -> str:
    """実行に影響する項目だけから作る。title のような表示用の項目は外す。

    既定値を埋めたあとの値を使う。problem.toml に既定値と同じ行を足しただけで
    走り直すのを避けるため。
    """
    payload = {
        "limits": {
            "tle_sec": problem.limits.tle_sec,
            "mle_mb": problem.limits.mle_mb,
        },
        "harness": {"kind": problem.harness_kind},
        "testdata": {
            "source": problem.testdata.source,
            "name": problem.testdata.name,
            "generator": problem.testdata.generator,
            "count": problem.testdata.count,
            "reference": problem.testdata.reference,
        },
        "compare": {"kind": problem.compare.kind},
    }
    if problem.compare.kind == "float":
        # 許容誤差を変えれば判定が変わる。float のときだけ足す。全問題に足すと
        # 既存の記録が測り直しになる。
        payload["compare"]["abs_tol"] = problem.compare.abs_tol
        payload["compare"]["rel_tol"] = problem.compare.rel_tol
    if problem.testdata.source == "local":
        # ジェネレータと参照実装はリポジトリの中にあって、中身を変えれば出る
        # ケースが変わる。ファイル名だけでは足りない。
        #
        # cases_hash なら拾えるが、あれは記録から借りることがある。借りた値は
        # 古いままなので、リポジトリの中が原因の変化はそちらに載らない。
        # 中にある原因はここへ、外にある原因 (判定サイトや上流のジェネレータ)
        # は cases_hash へ、と分けておく。
        #
        # local のときだけ足す。すべての問題に足すと、既存の記録が全部
        # 測り直しになる。
        payload["testdata"]["generator_source"] = _sha256(
            normalize_file(problem.dir / problem.testdata.generator)
        )
        payload["testdata"]["reference_source"] = _sha256(
            normalize_file(problem.dir / problem.testdata.reference)
        )
    canonical = json.dumps(
        payload, sort_keys=True, separators=(",", ":"), ensure_ascii=False
    )
    return hashlib.sha256(canonical.encode()).hexdigest()


def compute(
    *,
    submission: str,
    submission_hash: str,
    harness_hash: str,
    problem_hash: str,
    cases_hash: str,
    env: str,
    compiler_version: str,
    cxxflags: str,
    cpu_model: str,
) -> str:
    return _sha256(
        submission,
        submission_hash,
        harness_hash,
        problem_hash,
        cases_hash,
        env,
        compiler_version,
        cxxflags,
        cpu_model,
    )
