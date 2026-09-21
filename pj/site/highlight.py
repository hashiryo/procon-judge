"""提出のソースの色付け。

Library のサイトは shiki の github-light / github-dark で色を付けている。こちらは
npm を使わないので、鍵の計算に使っている字句解析器 (pj.key.lex) でトークンに
分けて、種類ごとに span を巻く。文法は解かないので型名までは分からないが、
キーワード、文字列、数、コメント、指令、呼び出しの名前で見た目の大半は揃う。
色の値は style.css に、shiki の同じテーマから取ったものを置いてある。
"""

from __future__ import annotations

import html
import re

from ..key import lex

KEYWORDS = frozenset(
    {
        "alignas", "alignof", "and", "and_eq", "asm", "auto", "bitand", "bitor", "bool",
        "break", "case", "catch", "char", "char8_t", "char16_t", "char32_t", "class",
        "compl", "concept", "const", "consteval", "constexpr", "constinit",
        "const_cast", "continue", "co_await", "co_return", "co_yield", "decltype",
        "default", "delete", "do", "double", "dynamic_cast", "else", "enum", "explicit",
        "export", "extern", "final", "float", "for", "friend", "goto", "if", "import",
        "inline", "int", "long", "module", "mutable", "namespace", "new", "noexcept",
        "not", "not_eq", "operator", "or", "or_eq", "override", "private", "protected",
        "public", "register", "reinterpret_cast", "requires", "return", "short",
        "signed", "sizeof", "static", "static_assert", "static_cast", "struct",
        "switch", "template", "this", "thread_local", "throw", "try", "typedef",
        "typeid", "typename", "union", "unsigned", "using", "virtual", "void",
        "volatile", "wchar_t", "while", "xor", "xor_eq",
    }
)

# 定数は数と同じ色。shiki の github テーマもそうしている。
CONSTANTS = frozenset({"true", "false", "nullptr"})

_HEAD_RE = re.compile(r"^(?P<head>[ \t]*#[ \t]*\w*)(?P<rest>.*)$", re.DOTALL)
_INCLUDE_REST_RE = re.compile(r'(?P<s>"(?:[^"\\\n]|\\.)*"|<[^<>\s]+>)|(?P<c>//.*$)|(?P<o>[^"<\n/]+|.)', re.DOTALL)
_OTHER_REST_RE = re.compile(r'(?P<s>"(?:[^"\\\n]|\\.)*")|(?P<c>//.*$)|(?P<o>[^"\n/]+|.)', re.DOTALL)


def highlight(text: str) -> str:
    """HTML にする。span を剥がすと html.escape(text) に戻る。"""
    tokens = list(lex(text))
    out: list[str] = []
    for index, (kind, piece) in enumerate(tokens):
        if kind == "directive":
            out.append(_directive(piece))
            continue
        cls = _class(kind, piece, tokens, index)
        escaped = html.escape(piece)
        out.append(f'<span class="{cls}">{escaped}</span>' if cls else escaped)
    return "".join(out)


def _class(kind: str, piece: str, tokens: list[tuple[str, str]], index: int) -> str | None:
    if kind == "comment":
        return "hl-c"
    if kind in ("string", "char", "raw"):
        return "hl-s"
    if kind == "number":
        return "hl-n"
    if kind == "ident":
        if piece in CONSTANTS:
            return "hl-n"
        if piece in KEYWORDS:
            return "hl-k"
        if _followed_by_paren(tokens, index):
            return "hl-f"
    return None


def _followed_by_paren(tokens: list[tuple[str, str]], index: int) -> bool:
    """同じ行で、次の空白でないトークンが ( か。呼び出しか定義の名前。"""
    for kind, piece in tokens[index + 1 :]:
        if kind == "space":
            if "\n" in piece:
                return False
            continue
        return kind == "punct" and piece == "("
    return False


def _directive(piece: str) -> str:
    """指令の行。先頭を指令の色にして、中の文字列と行末コメントに色を付ける。"""
    match = _HEAD_RE.match(piece)
    assert match is not None
    head, rest = match.group("head"), match.group("rest")
    pattern = _INCLUDE_REST_RE if head.split("#", 1)[1].strip() in ("include", "import") else _OTHER_REST_RE
    out = [f'<span class="hl-p">{html.escape(head)}</span>']
    for sub in pattern.finditer(rest):
        text = html.escape(sub.group())
        if sub.lastgroup == "s":
            out.append(f'<span class="hl-s">{text}</span>')
        elif sub.lastgroup == "c":
            out.append(f'<span class="hl-c">{text}</span>')
        else:
            out.append(text)
    return "".join(out)
