"""記録から静的なサイトを作る。

ページごとに必要な形の JSON をここで作る。ブラウザが取るのは 1 ページにつき
2 個だけで、結合も反転もすべて生成の側で済ませる。1 つの大きい JSON を
読ませると、記録が増えたときに携帯で開けなくなる。

ページは 3 種類ある。問題一覧 (index.html) と順位表 (problems/<id>.html) は
テンプレートに JSON を読ませて JavaScript が描く。提出ページ
(submissions/<id>/<name>.html) は切り替えが無いので、ここで HTML まで埋める。
ヘッダごとの逆引き (data/headers/<ラベル>.json) はページではなく、ライブラリ側の
サイトが表示時に読む JSON。
"""

from __future__ import annotations

import hashlib
import html
import json
import os
import re
import shutil
import subprocess
import urllib.parse
from collections.abc import Sequence
from dataclasses import dataclass
from datetime import UTC, datetime
from pathlib import Path, PurePosixPath

from .. import build as build_mod
from .. import environment as env_mod
from .. import include as include_mod
from .. import libraries as lib_mod
from .. import problem as problem_mod
from ..freshness import Diff, Freshness
from ..paths import ROOT
from ..record import judge_sha, library_sha
from ..store import Store
from .highlight import highlight

TEMPLATES = Path(__file__).resolve().parent / "templates"

# 前のサイトかどうかを見分ける目印。空でないディレクトリを黙って消さないために置く。
MARKER = ".pj-site"

# 失敗したケースの説明。提出ページの「失敗」の節に出すので、記録が持つ長さまで残す。
DETAIL_CHARS = 2000

PLACEHOLDER = re.compile(r"\{\{(\w+)\}\}")

esc = html.escape


class SiteError(Exception):
    """サイトを作れないときに投げる。"""


@dataclass(frozen=True)
class Cell:
    """(提出, 環境, CPU モデル) 1 つぶんの表示用の値。"""

    submission: str
    env: str
    cpu_model: str
    cpu_arch: str
    compiler_version: str
    cxxflags: str
    status: str
    algo_ns: int | None
    wall_ms: int
    rss_kb: int
    source_bytes: int
    binary_bytes: int | None
    samples: int
    timestamp: str
    judge_sha: str | None
    failed: dict | None
    # AC でなかったケースの名前。failed はその最初の 1 つの明細。
    failed_cases: tuple[str, ...] = ()
    # 今のソースで測った記録なら True、ソースが変わっていれば False。
    # 判定できなければ None。
    current: bool | None = None
    # 参考に落ちた理由。current が False のときだけ入る。
    reason: Diff | None = None


@dataclass(frozen=True)
class IncludeLink:
    """提出ページの include の欄の 1 行。"""

    label: str
    href: str | None
    # ラベルを持つライブラリの名前。無ければこのリポジトリのファイル。
    library: str | None
    # ライブラリのソースへのリンク。ページとは別に置く。
    source_href: str | None
    # 提出が直接 include しているか。そうでなければ辿って間接に入るもの。
    direct: bool
    # 解決できなかった include。
    missing: bool = False


@dataclass(frozen=True)
class Summary:
    out: Path
    problems: int
    records: int
    pages: int
    stale: int = 0
    submission_pages: int = 0
    headers: int = 0


def collapse(
    records: Sequence[dict], freshness: Freshness | None = None
) -> list[Cell]:
    """記録を (提出, 環境, CPU モデル) ごとに 1 行へ畳む。

    同じ組でもソースを書き換えればキーが変わって、別の測定になる。新しい方の
    キーだけを残す。残った記録が複数あるのは同じ条件を何度か測ったときなので、
    そこは最小値を採る。順位を最小値で決めておけば、あとで標本を積み始めても
    表示の側を書き直さずに済む。

    現行のキーの記録があれば、時刻が古くてもそちらを採る。書き換えたものを
    元に戻すと前のキーが復活するので、いちばん新しい記録が現行とは限らない。
    """
    judge = freshness.current if freshness else (lambda record: None)
    groups: dict[tuple[str, str, str], list[tuple[int, dict]]] = {}
    for position, record in enumerate(records):
        group = (
            record.get("submission", ""),
            record.get("env", ""),
            record.get("cpu_model", ""),
        )
        groups.setdefault(group, []).append((position, record))

    cells = []
    for (submission, env, cpu_model), items in groups.items():
        pool = [item for item in items if judge(item[1]) is True] or items
        # 同じ時刻が並んだときは、ファイルの後ろにある方を新しいとみなす。
        _, newest = max(pool, key=lambda item: (item[1].get("timestamp") or "", item[0]))
        same = [r for _, r in items if r.get("key") == newest.get("key")]
        algo = [
            r["algo_time_max_ns"]
            for r in same
            if r.get("algo_time_max_ns") is not None
        ]
        current = judge(newest)
        cells.append(
            Cell(
                submission=submission,
                env=env,
                cpu_model=cpu_model,
                cpu_arch=newest.get("cpu_arch", ""),
                compiler_version=newest.get("compiler_version", ""),
                cxxflags=newest.get("cxxflags", ""),
                status=newest.get("status", ""),
                algo_ns=min(algo) if algo else None,
                wall_ms=min(r.get("time_max_ms") or 0 for r in same),
                rss_kb=min(r.get("memory_max_kb") or 0 for r in same),
                source_bytes=newest.get("source_bytes") or 0,
                binary_bytes=newest.get("binary_bytes"),
                samples=len(same),
                timestamp=max(r.get("timestamp") or "" for r in same),
                judge_sha=newest.get("judge_sha"),
                failed=_failed(newest),
                failed_cases=tuple(newest.get("failed_cases") or ()),
                current=current,
                reason=(
                    freshness.diff(newest)
                    if freshness is not None and current is False
                    else None
                ),
            )
        )
    return sorted(cells, key=lambda c: (c.env, c.cpu_model, c.submission))


def describe_diff(diff: Diff | None) -> str | None:
    """参考の理由を 1 行の日本語にする。"""
    if diff is None:
        return None
    if not diff.known:
        return "理由は記録に無い"
    parts = []
    if diff.changed:
        parts.append("変更 " + ", ".join(diff.changed))
    if diff.added:
        parts.append("追加 " + ", ".join(diff.added))
    if diff.removed:
        parts.append("削除 " + ", ".join(diff.removed))
    names = {"problem": "problem.toml", "cxxflags": "cxxflags"}
    for setting in diff.settings:
        parts.append(names.get(setting, setting) + " が変わった")
    return " / ".join(parts)


def _failed(record: dict) -> dict | None:
    failed = record.get("failed_case")
    if not failed:
        return None
    return {
        "name": failed.get("name") or "",
        "detail": (failed.get("detail") or "")[:DETAIL_CHARS],
    }


def _env_order() -> dict[str, int]:
    try:
        return {env.name: index for index, env in enumerate(env_mod.load_all())}
    except (env_mod.EnvironmentError_, OSError):
        return {}


def _safe_id(problem_id: str) -> bool:
    """記録から来た id をそのままファイル名にしてよいか。"""
    return bool(problem_id) and not (
        problem_id.startswith(".") or "/" in problem_id or "\\" in problem_id
    )


def _safe_label(label: str) -> bool:
    """閉包のラベルをそのまま data/headers/ の下のパスにしてよいか。"""
    parts = PurePosixPath(label).parts
    return bool(parts) and not label.startswith("/") and all(
        p not in ("", ".", "..") and not p.startswith(".") and "\\" not in p
        for p in parts
    )


def submission_page(problem_id: str, submission: str) -> str | None:
    """提出ページの、サイトのルートからのパス。作れなければ None。

    提出の名前は submissions/ を外して拡張子を落としたもの。
    submissions/lib-segtree.hpp なら submissions/<id>/lib-segtree.html になる。
    """
    if not _safe_id(problem_id):
        return None
    parts = PurePosixPath(submission).parts
    if parts and parts[0] == "submissions":
        parts = parts[1:]
    if not parts or any(
        p in ("", ".", "..") or p.startswith(".") or "\\" in p for p in parts
    ):
        return None
    name = PurePosixPath(*parts).with_suffix("")
    return f"submissions/{problem_id}/{name.as_posix()}.html"


# id の接頭辞。問題の定義の表 (DESIGN.md の「problem.toml」) と同じ。無ければ自作。
KNOWN_ORIGINS = ("yosupo", "aoj", "yuki", "atcoder", "loj", "hackerrank", "cses", "joisc")
OWN_ORIGIN = "自作"


def origin_of(problem_id: str) -> str:
    """問題一覧の件数を出どころごとに数えるための、id の接頭辞。"""
    head = problem_id.split("-", 1)[0]
    return head if "-" in problem_id and head in KNOWN_ORIGINS else OWN_ORIGIN


def origin_counts(problem_ids: Sequence[str]) -> dict[str, int]:
    """出どころごとの問題数。表の上の 1 行に出す。表は分けない。"""
    counts: dict[str, int] = {}
    for problem_id in problem_ids:
        origin = origin_of(problem_id)
        counts[origin] = counts.get(origin, 0) + 1
    order = (*KNOWN_ORIGINS, OWN_ORIGIN)
    return {name: counts[name] for name in order if name in counts}


# 取得元の表示名。内部の名前 (local / none) をそのまま出すと、テストケースが
# 判定サイトのものかどうかが読めない。
SOURCE_LABELS = {
    "library_checker": "Library Checker",
    "aoj": "AOJ",
    "yukicoder": "yukicoder",
    "manual": "手動取り込み",
    "local": "自作",
    "none": "無し (コンパイルのみ)",
}
OFFICIAL_SOURCES = frozenset({"library_checker", "aoj", "yukicoder"})
# 判定サイトのデータで測ったように見えては困るもの。注意書きを目立たせる。
CAUTION_SOURCES = frozenset({"local", "none"})


def source_label(source: str, compare_kind: str = "") -> str:
    # none は「走らせない」と「提出が自分で検証する」の 2 通りあるので、比較の種別で分ける。
    if source == "none" and compare_kind == "exit_code":
        return "無し (自己検証)"
    return SOURCE_LABELS.get(source, source)


def testdata_note(problem: problem_mod.Problem | None) -> str | None:
    """判定サイトのテストデータでない問題に出す注意書き。判定サイトのものなら None。

    AtCoder のようにテストケースが公開されない問題は自作するしかないが、公式の
    ケースで測ったように見えるのは避けたい。AC の意味が違うことをその場で言う。
    """
    if problem is None:
        return None
    td = problem.testdata
    if td.source == "local":
        return (
            f"テストケースは自作です。ジェネレータ ({td.generator}) と参照実装 "
            f"({td.reference}) で {td.count} ケースを作っています。判定サイトのデータ"
            "ではないので、ここでの AC は元の問題の AC と同じ意味ではなく、時間と"
            "メモリもこのケースに対する値です。"
        )
    if td.source == "none":
        if problem.compare.kind == "exit_code":
            return (
                "テストケースがありません。提出が自分で持っている入出力で検証して、"
                "終了コード 0 で走り切れば AC です。時間とメモリはその 1 回のものです。"
            )
        return (
            "テストケースがありません。コンパイルが通るかだけを見ていて、AC は"
            "それを表します。時間とメモリは測っていません。"
        )
    if td.source == "manual":
        return (
            "テストケースは手で取り込んだものです。出どころは問題の定義 "
            "(problem.toml) を見てください。"
        )
    return None


def problem_url(problem: problem_mod.Problem | None) -> str | None:
    """元の問題のページ。判定サイトから取っている問題だけ分かる。"""
    if problem is None:
        return None
    source, name = problem.testdata.source, problem.testdata.name
    if source == "library_checker":
        return f"https://judge.yosupo.jp/problem/{name.rsplit('/', 1)[-1]}"
    if source == "aoj":
        return f"https://onlinejudge.u-aizu.ac.jp/problems/{name}"
    if source == "yukicoder":
        return f"https://yukicoder.me/problems/no/{name}"
    return None


def problem_payload(
    problem_id: str,
    problem: problem_mod.Problem | None,
    cells: Sequence[Cell],
    generated_at: str,
    case_count: int = 0,
    repo: str | None = None,
) -> dict:
    """problems/<id>.html が読む JSON。"""
    order = _env_order()
    defined = [s.as_posix() for s in problem.submissions()] if problem else []
    submissions = sorted(set(defined) | {c.submission for c in cells})

    combos: dict[tuple[str, str], dict] = {}
    for cell in cells:
        combo = combos.setdefault(
            (cell.env, cell.cpu_model),
            {
                "env": cell.env,
                "cpu_model": cell.cpu_model,
                "cpu_arch": cell.cpu_arch,
                "compiler_version": cell.compiler_version,
                "measured": 0,
            },
        )
        combo["measured"] += 1

    cxxflags: dict[str, str] = {}
    for cell in cells:
        cxxflags.setdefault(cell.env, cell.cxxflags)

    source = problem.testdata.source if problem else ""
    return {
        "id": problem_id,
        "title": problem.title if problem else problem_id,
        "url": problem_url(problem),
        "source": source,
        "source_label": source_label(source, problem.compare.kind if problem else ""),
        "official": (source in OFFICIAL_SOURCES) if problem else None,
        "caution": source in CAUTION_SOURCES,
        "note": testdata_note(problem),
        # 自作のケースを作るファイル。GitHub へ飛ばすのに使う。
        "generator": (
            f"problems/{problem_id}/{problem.testdata.generator}"
            if problem and source == "local"
            else None
        ),
        "reference": (
            f"problems/{problem_id}/{problem.testdata.reference}"
            if problem and source == "local" and problem.testdata.reference
            else None
        ),
        "judge_sha": judge_sha(),
        "compare": problem.compare.kind if problem else "",
        "harness": problem.harness_kind if problem else "",
        "tle_sec": problem.limits.tle_sec if problem else 0,
        "mle_mb": problem.limits.mle_mb if problem else 0,
        "case_count": case_count,
        "generated_at": generated_at,
        "repo": repo,
        "submissions": submissions,
        # 提出ページへの、サイトのルートからのパス。作れない提出は None。
        "pages": {s: submission_page(problem_id, s) for s in submissions},
        "combos": sorted(
            combos.values(),
            key=lambda c: (order.get(c["env"], len(order)), c["env"], c["cpu_model"]),
        ),
        "cxxflags": cxxflags,
        "rows": [
            {
                "submission": c.submission,
                "env": c.env,
                "cpu_model": c.cpu_model,
                "status": c.status,
                "algo_ns": c.algo_ns,
                "wall_ms": c.wall_ms,
                "rss_kb": c.rss_kb,
                "source_bytes": c.source_bytes,
                "binary_bytes": c.binary_bytes,
                "samples": c.samples,
                "timestamp": c.timestamp,
                "judge_sha": c.judge_sha,
                "failed": c.failed,
                "failed_cases": list(c.failed_cases),
                "current": c.current,
                "reason": describe_diff(c.reason),
            }
            for c in cells
        ],
    }


# --- 提出ページ ---------------------------------------------------------------


@dataclass(frozen=True)
class SubmissionPage:
    """提出ページ 1 枚ぶんの材料。"""

    problem_id: str
    title: str
    url: str | None
    source: str  # 取得元
    submission: str
    cells: Sequence[Cell]
    # 記録が無くても行を出す環境。CI の環境の名前を environments.toml の順で。
    env_names: Sequence[str]
    includes: Sequence[IncludeLink] | None
    source_text: str | None
    repo: str | None
    sha: str | None
    generated_at: str
    # 判定サイトのテストデータでないときの注意書き。None なら出さない。
    note: str | None = None
    caution: bool = False
    generator: str | None = None
    reference: str | None = None
    # 取得元の表示名。空なら source から作る。none は比較の種別で表示が変わるので、
    # problem_payload が決めたものを持ち回る。
    source_label: str = ""


def _ms(ns: int | None) -> str:
    return "-" if ns is None else f"{ns / 1e6:.2f} ms"


def _mb(kb: int) -> str:
    return f"{kb / 1024:.1f} MB" if kb else "-"


def _stamp(iso: str) -> str:
    return iso[:16].replace("T", " ") if iso else ""


def _blob(repo: str | None, sha: str | None, rel: str) -> str | None:
    if not repo or not sha:
        return None
    return f"{repo}/blob/{sha}/{rel}"


def _submission_rel(problem_id: str, submission: str) -> str:
    return f"problems/{problem_id}/{submission}"


def _cell_row(page: SubmissionPage, c: Cell) -> str:
    failed = ""
    title = ""
    if c.failed and c.failed.get("name"):
        failed = f' <span class="dim">{esc(c.failed["name"])}</span>'
    if c.failed and c.failed.get("detail"):
        title = f' title="{esc(c.failed["detail"])}"'
    status = f'<td class="st st-{esc(c.status)}"{title}>{esc(c.status)}{failed}</td>'

    if c.current is True:
        fresh = "<td>現行</td>"
    elif c.current is False:
        # 理由は長いので表には入れず、表の下の「参考の理由」に出す。
        reason = describe_diff(c.reason) or "理由は記録に無い"
        fresh = f'<td><span class="chip" title="{esc(reason)}">参考</span></td>'
    else:
        fresh = '<td class="dim" title="今のソースと比べられませんでした">-</td>'

    commit = "-"
    href = _blob(page.repo, c.judge_sha, _submission_rel(page.problem_id, page.submission))
    if c.judge_sha:
        short = esc(c.judge_sha[:7])
        commit = f'<a class="mono" href="{esc(href)}">{short}</a>' if href else short

    algo_class = "n" if c.status == "AC" else "n dim"
    row_class = ' class="stale"' if c.current is False else ""
    return (
        f"<tr{row_class}>"
        f"<td>{esc(c.env)}</td>"
        f'<td title="{esc(c.cpu_model)}">{esc(c.cpu_model)}</td>'
        f"{status}"
        f'<td class="{algo_class}">{_ms(c.algo_ns)}</td>'
        f'<td class="n">{c.wall_ms} ms</td>'
        f'<td class="n">{_mb(c.rss_kb)}</td>'
        f'<td class="n">{c.samples}</td>'
        f"{fresh}"
        f'<td class="dim" title="{esc(c.timestamp)}">{esc(_stamp(c.timestamp))}</td>'
        f"<td>{commit}</td>"
        "</tr>"
    )


def _where(c: Cell) -> str:
    return f"{c.env} {c.cpu_model}"


def _reasons_html(page: SubmissionPage) -> str:
    """参考の行の理由。同じ理由の行をまとめて、どの行かを添える。"""
    stale = [c for c in page.cells if c.current is False]
    if not stale:
        return ""
    groups: dict[str, list[Cell]] = {}
    for c in stale:
        groups.setdefault(describe_diff(c.reason) or "理由は記録に無い", []).append(c)
    items = []
    for reason, cells in groups.items():
        where = "、".join(_where(c) for c in cells)
        items.append(
            f'<li>{esc(reason)} <span class="dim">({esc(where)})</span></li>'
        )
    return '<p class="note">参考の理由</p><ul class="reasons">' + "".join(items) + "</ul>"


def repro_command(problem_id: str, submission: str, case: str | None) -> str:
    command = f"pj repro --problem {problem_id} --submission {submission}"
    return command + (f" --case {case}" if case else "")


def _failures_html(page: SubmissionPage) -> str:
    """AC でなかった行の明細。ケース名、差分の先頭、手元で再現するコマンド。"""
    failed = [c for c in page.cells if c.status and c.status != "AC"]
    if not failed:
        return ""
    parts = ["<h2>失敗</h2>"]
    for c in failed:
        name = (c.failed or {}).get("name") or ""
        detail = (c.failed or {}).get("detail") or ""
        head = (
            f'<span class="st st-{esc(c.status)}">{esc(c.status)}</span> '
            f"{esc(_where(c))}"
        )
        if name:
            head += f' ケース <span class="mono">{esc(name)}</span>'
        others = [n for n in c.failed_cases if n != name]
        if others:
            head += f' <span class="dim">(ほかに {len(others)} 件: {esc(", ".join(others))})</span>'
        block = [f'<div class="failure"><p class="note">{head}</p>']
        if detail:
            block.append(f'<pre class="detail mono">{esc(detail)}</pre>')
        command = repro_command(page.problem_id, page.submission, name or None)
        block.append(
            f'<p class="note">手元で再現: <code class="mono">{esc(command)}</code></p></div>'
        )
        parts.append("".join(block))
    return "".join(parts)


def _missing_row(env: str) -> str:
    return (
        '<tr class="missing">'
        f"<td>{esc(env)}</td>"
        '<td class="dim">-</td>'
        '<td class="dim">未計測</td>'
        '<td class="n dim">-</td><td class="n dim">-</td><td class="n dim">-</td>'
        '<td class="n dim">-</td><td class="dim">-</td><td class="dim">-</td>'
        '<td class="dim">-</td>'
        "</tr>"
    )


def _rows_html(page: SubmissionPage) -> str:
    order = {name: index for index, name in enumerate(page.env_names)}
    cells = sorted(
        page.cells, key=lambda c: (order.get(c.env, len(order)), c.env, c.cpu_model)
    )
    rows = [_cell_row(page, c) for c in cells]
    measured = {c.env for c in cells}
    rows += [_missing_row(env) for env in page.env_names if env not in measured]
    if not rows:
        return '<tr><td colspan="10" class="empty">まだ記録がありません。</td></tr>'
    return "".join(rows)


def _include_list(links: Sequence[IncludeLink]) -> str:
    if not links:
        return '<p class="empty">なし</p>'
    items = []
    for link in links:
        label = esc(link.label)
        if link.href:
            body = f'<a class="mono" href="{esc(link.href)}">{label}</a>'
        else:
            body = f'<span class="mono">{label}</span>'
        tail = ""
        if link.missing:
            tail = '<span class="lib">見つからない</span>'
        elif link.library:
            tail = f'<span class="lib">{esc(link.library)}</span>'
            if link.source_href:
                tail += f' <a class="lib" href="{esc(link.source_href)}">src</a>'
        items.append(f"<li>{body}{tail}</li>")
    return '<ul class="includes">' + "".join(items) + "</ul>"


def _includes_html(links: Sequence[IncludeLink] | None) -> str:
    if links is None:
        return (
            '<p class="empty">提出のファイルが今のリポジトリに無いので、'
            "include は分かりません。</p>"
        )
    direct = [l for l in links if l.direct]
    via = [l for l in links if not l.direct]
    return (
        '<p class="note">直接 include しているもの</p>'
        + _include_list(direct)
        + '<p class="note">間接 (直接のものから辿って入るもの)</p>'
        + _include_list(via)
    )


def _source_html(text: str | None) -> str:
    if text is None:
        return '<p class="empty">提出のファイルが今のリポジトリにありません。</p>'
    return f'<pre class="source mono">{highlight(text)}</pre>'


def submission_html(page: SubmissionPage, style_v: str) -> str:
    """提出ページの HTML。切り替えが無いので JavaScript は使わない。"""
    problem_html = urllib.parse.quote(page.problem_id) + ".html"
    rel = _submission_rel(page.problem_id, page.submission)
    github = _blob(page.repo, page.sha, rel)

    subtitle = f'<span class="mono">{esc(rel)}</span>'
    if github:
        subtitle += f' / <a href="{esc(github)}">GitHub</a>'

    meta = [
        f'<span>問題 <a href="../../problems/{problem_html}">{esc(page.problem_id)}</a></span>',
    ]
    if page.source:
        meta.append(f"<span>取得元 {esc(page.source_label or source_label(page.source))}</span>")
    if page.url:
        meta.append(f'<span><a href="{esc(page.url)}">原題</a></span>')
    stale = sum(1 for c in page.cells if c.current is False)
    meta.append(f"<span>記録 {len(page.cells)} 組</span>")
    if stale:
        meta.append(f"<span>参考 {stale}</span>")

    generated = f"{esc(_stamp(page.generated_at))} 生成 (UTC)"
    if page.sha:
        generated += f" / judge {esc(page.sha[:7])}"

    name = page.submission.removeprefix("submissions/")

    return _render(
        "submission.html",
        {
            "TITLE": esc(f"{name} - {page.title}"),
            "STYLE_V": style_v,
            "PROBLEM_HTML": problem_html,
            "PROBLEM_TITLE": esc(page.title),
            "NAME": esc(name),
            "SUBTITLE": subtitle,
            "META": "".join(meta) + _note_html(page),
            "ROWS": _rows_html(page),
            "REASONS": _reasons_html(page),
            "FAILURES": _failures_html(page),
            "INCLUDES": _includes_html(page.includes),
            "SOURCE": _source_html(page.source_text),
            "GENERATED": generated,
        },
    )


def _note_html(page: SubmissionPage) -> str:
    """テストデータの注意書き。自作ならジェネレータと参照実装へのリンクを添える。"""
    if not page.note:
        return ""
    body = esc(page.note)
    links = []
    for label, rel in (("ジェネレータ", page.generator), ("参照実装", page.reference)):
        href = _blob(page.repo, page.sha, rel) if rel else None
        if href:
            links.append(f'<a href="{esc(href)}">{label}</a>')
    if links:
        body += " " + " / ".join(links)
    cls = "notice warn" if page.caution else "notice"
    return f'</div><p class="{cls}">{body}</p><div class="meta">'


def include_links(
    problem: problem_mod.Problem,
    submission: str,
    libraries: Sequence[lib_mod.Library],
    *,
    repo: str | None,
    sha: str | None,
    lib_sha: str | None,
) -> list[IncludeLink] | None:
    """提出ページの include の欄。提出のファイルが無ければ None。

    ライブラリのヘッダは説明ページへ、このリポジトリのファイルは GitHub へ飛ばす。
    どちらかはラベルの接頭辞で決める。pj はライブラリが何かを知らない。
    """
    source = problem.dir / submission
    if not source.is_file():
        return None
    search = build_mod.include_dirs(problem)
    found = include_mod.closure(source, search)
    direct = set(include_mod.direct(source, search))

    links = []
    for label, path in zip(found.labels, found.files, strict=True):
        library = lib_mod.find(label, list(libraries))
        if library is not None:
            href = library.page_url(label)
            source_href = library.source_url(label, lib_sha)
            name = library.name
        else:
            try:
                rel = path.resolve().relative_to(ROOT).as_posix()
            except ValueError:
                rel = None
            href = _blob(repo, sha, rel) if rel else None
            source_href = None
            name = None
        links.append(
            IncludeLink(
                label=label,
                href=href,
                library=name,
                source_href=source_href,
                direct=label in direct,
            )
        )
    for target in found.unresolved:
        links.append(
            IncludeLink(
                label=target, href=None, library=None, source_href=None,
                direct=True, missing=True,
            )
        )
    return links


def _read_source(problem: problem_mod.Problem | None, submission: str) -> str | None:
    if problem is None:
        return None
    path = problem.dir / submission
    try:
        return path.read_text(errors="replace")
    except OSError:
        return None


# --- ヘッダごとの逆引き ---------------------------------------------------------


def env_summary(cells: Sequence[Cell], env_names: Sequence[str]) -> list[dict]:
    """1 提出の記録を環境ごとに畳む。ライブラリ側のサイトが表に出す単位。

    同じ環境でも CPU モデルが複数あるので、状態は全部 AC のときだけ AC、
    現行は全部現行のときだけ True にする。記録の無い環境も行に出す。
    """
    by_env: dict[str, list[Cell]] = {}
    for cell in cells:
        by_env.setdefault(cell.env, []).append(cell)
    names = list(env_names) + sorted(e for e in by_env if e not in env_names)
    out = []
    for env in names:
        group = by_env.get(env)
        if not group:
            out.append({"env": env, "status": None, "current": None, "models": 0,
                        "algo_ns": None})
            continue
        bad = [c.status for c in group if c.status != "AC"]
        flags = [c.current for c in group]
        if any(f is False for f in flags):
            current: bool | None = False
        elif any(f is None for f in flags):
            current = None
        else:
            current = True
        algo = [c.algo_ns for c in group if c.status == "AC" and c.algo_ns is not None]
        out.append(
            {
                "env": env,
                "status": bad[0] if bad else "AC",
                "current": current,
                "models": len(group),
                "algo_ns": min(algo) if algo else None,
            }
        )
    return out


def header_entry(
    problem_id: str,
    title: str,
    submission: str,
    page: str,
    direct: bool,
    cells: Sequence[Cell],
    env_names: Sequence[str],
    *,
    testdata: str = "",
    official: bool | None = None,
) -> dict:
    return {
        "problem": problem_id,
        "title": title,
        "submission": submission,
        "direct": direct,
        # テストデータの取得元と、それが判定サイトのものか。自作のケースで通した
        # AC を判定サイトの AC と同じに見せないため。
        "testdata": testdata,
        "official": official,
        "page": page,
        "problem_page": f"problems/{problem_id}.html",
        "envs": env_summary(cells, env_names),
    }


def site_url() -> str | None:
    """このサイトの URL。Pages の project site の形。CI でだけ分かる。"""
    slug = os.environ.get("GITHUB_REPOSITORY")
    if not slug or "/" not in slug:
        return None
    owner, repo = slug.split("/", 1)
    return f"https://{owner.lower()}.github.io/{repo}/"


# --- 書き出し ----------------------------------------------------------------


def _write(path: Path, text: str) -> str:
    """書いて、中身のハッシュを ?v= の形で返す。

    Pages のキャッシュヘッダは細かく制御できないので、参照する側の URL に
    混ぜて古いデータを見せないようにする。
    """
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text)
    return "?v=" + hashlib.sha256(text.encode()).hexdigest()[:12]


def _render(template: str, values: dict[str, str]) -> str:
    """テンプレートの {{NAME}} を埋める。

    埋める前に目印の集合を照らし合わせる。埋めたあとの文字列を見ないのは、
    提出のソースに {{1}} のような波括弧の並びが普通に出るため。置き換えは
    1 回の走査で行い、埋めた値の中の目印を二度読みしない。
    """
    text = (TEMPLATES / template).read_text()
    names = set(PLACEHOLDER.findall(text))
    for name in sorted(set(values) - names):
        raise SiteError(f"{template} に {{{{{name}}}}} がありません")
    for name in sorted(names - set(values)):
        raise SiteError(f"{template} の {{{{{name}}}}} を埋めていません")
    return PLACEHOLDER.sub(lambda m: values[m.group(1)], text)


def _prepare(out: Path) -> None:
    if out.exists():
        if any(out.iterdir()) and not (out / MARKER).is_file():
            raise SiteError(
                f"{out} は空でなく、前のサイトでもありません。別の場所を指定してください"
            )
        shutil.rmtree(out)
    out.mkdir(parents=True)
    (out / MARKER).write_text("pj site build\n")


def repo_url(root: Path = ROOT) -> str | None:
    """このリポジトリの GitHub 上の URL。提出のソースへ飛ばすのに使う。

    CI では GITHUB_REPOSITORY がある。手元では origin の remote から作る。
    remote の host は SSH の別名 (github.com.hashiryo) が挟まって当てにならないので、
    github.com を含むことだけ確かめて末尾の 2 つを owner/repo として拾う。
    """
    slug = os.environ.get("GITHUB_REPOSITORY")
    if slug:
        server = os.environ.get("GITHUB_SERVER_URL") or "https://github.com"
        return f"{server.rstrip('/')}/{slug}"
    try:
        proc = subprocess.run(
            ["git", "remote", "get-url", "origin"],
            cwd=root, capture_output=True, text=True, check=False, timeout=10,
        )
    except (subprocess.SubprocessError, OSError):
        return None
    remote = proc.stdout.strip()
    if proc.returncode != 0 or "github.com" not in remote:
        return None
    parts = remote.removesuffix(".git").replace(":", "/").split("/")
    if len(parts) < 2:
        return None
    return "https://github.com/" + "/".join(parts[-2:])


def build(store: Store, out: Path) -> Summary:
    generated_at = (
        datetime.now(UTC).replace(microsecond=0).isoformat().replace("+00:00", "Z")
    )
    repo = repo_url()
    sha = judge_sha()
    lib_sha = library_sha()
    try:
        libraries = lib_mod.load_all()
    except lib_mod.LibrariesError as e:
        raise SiteError(str(e)) from e
    _prepare(out)

    style_v = _write(out / "style.css", (TEMPLATES / "style.css").read_text())
    index_js_v = _write(out / "index.js", (TEMPLATES / "index.js").read_text())
    problem_js_v = _write(out / "problem.js", (TEMPLATES / "problem.js").read_text())

    problems: dict[str, problem_mod.Problem] = {}
    for directory in problem_mod.all_problem_dirs():
        try:
            loaded = problem_mod.load(directory)
        except problem_mod.ProblemError:
            continue
        problems[loaded.id] = loaded

    try:
        envs = env_mod.load_all()
    except (env_mod.EnvironmentError_, OSError):
        envs = []
    # 記録が無くても提出ページに行を出す環境。手元専用の local は出さない。
    env_names = [e.name for e in envs if e.runs_on != "self"]

    ids = sorted(set(problems) | set(store.problem_ids()))
    rows = []
    total_records = 0
    total_stale = 0
    pages = 0
    submission_pages = 0
    # ラベル -> そのヘッダを閉包に持つ提出の一覧。ライブラリのヘッダだけ。
    headers: dict[str, list[dict]] = {}
    header_library: dict[str, str] = {}

    for problem_id in ids:
        if not _safe_id(problem_id):
            continue
        records = list(store.read(problem_id))
        total_records += len(records)
        problem = problems.get(problem_id)
        # 問題が repo から消えていれば、ソース側を作り直せないので判定しない。
        cells = collapse(records, Freshness(problem, envs) if problem else None)
        payload = problem_payload(
            problem_id,
            problem,
            cells,
            generated_at,
            case_count=max((r.get("case_count") or 0 for r in records), default=0),
            repo=repo,
        )

        name = f"{problem_id}.json"
        data_v = _write(
            out / "data" / "problems" / name,
            json.dumps(payload, ensure_ascii=False),
        )
        _write(
            out / "problems" / f"{problem_id}.html",
            _render(
                "problem.html",
                {
                    "TITLE": html.escape(payload["title"]),
                    "DATA_NAME": urllib.parse.quote(name),
                    "DATA_V": data_v,
                    "STYLE_V": style_v,
                    "SCRIPT_V": problem_js_v,
                },
            ),
        )
        pages += 1

        for submission in payload["submissions"]:
            page = payload["pages"][submission]
            if page is None:
                continue
            mine = [c for c in cells if c.submission == submission]
            includes = (
                include_links(
                    problem, submission, libraries, repo=repo, sha=sha, lib_sha=lib_sha
                )
                if problem
                else None
            )
            _write(
                out / page,
                submission_html(
                    SubmissionPage(
                        problem_id=problem_id,
                        title=payload["title"],
                        url=payload["url"],
                        source=payload["source"],
                        source_label=payload["source_label"],
                        submission=submission,
                        cells=mine,
                        env_names=env_names,
                        note=payload["note"],
                        caution=payload["caution"],
                        generator=payload["generator"],
                        reference=payload["reference"],
                        includes=includes,
                        source_text=_read_source(problem, submission),
                        repo=repo,
                        sha=sha,
                        generated_at=generated_at,
                    ),
                    style_v,
                ),
            )
            submission_pages += 1
            for link in includes or ():
                if link.library is None or link.missing or not _safe_label(link.label):
                    continue
                headers.setdefault(link.label, []).append(
                    header_entry(
                        problem_id, payload["title"], submission, page,
                        link.direct, mine, env_names,
                        testdata=payload["source"], official=payload["official"],
                    )
                )
                header_library[link.label] = link.library

        combos = len(payload["combos"])
        measured = len(cells)
        stale = sum(1 for c in cells if c.current is False)
        total_stale += stale
        rows.append(
            {
                "id": problem_id,
                "title": payload["title"],
                "source": payload["source"],
                "source_label": payload["source_label"],
                "caution": payload["caution"],
                "submissions": len(payload["submissions"]),
                "measured": measured,
                "stale": stale,
                "pending": len(payload["submissions"]) * combos - measured,
                "records": len(records),
                "updated": max((c.timestamp for c in cells), default=""),
            }
        )

    site = site_url()
    for label, entries in headers.items():
        _write(
            out / "data" / "headers" / f"{label}.json",
            json.dumps(
                {
                    "header": label,
                    "library": header_library[label],
                    "generated_at": generated_at,
                    "judge_sha": sha,
                    "library_sha": lib_sha,
                    # ページのパスはサイトのルートからの相対。これを前に付ける。
                    "site": site,
                    "environments": env_names,
                    "submissions": sorted(
                        entries, key=lambda e: (e["problem"], e["submission"])
                    ),
                },
                ensure_ascii=False,
            ),
        )

    index = {
        "generated_at": generated_at,
        "judge_sha": sha,
        "record_count": total_records,
        "stale_count": total_stale,
        "origins": origin_counts([row["id"] for row in rows]),
        "problems": rows,
    }
    data_v = _write(
        out / "data" / "index.json", json.dumps(index, ensure_ascii=False)
    )
    _write(
        out / "index.html",
        _render(
            "index.html",
            {"DATA_V": data_v, "STYLE_V": style_v, "SCRIPT_V": index_js_v},
        ),
    )
    return Summary(
        out=out,
        problems=len(rows),
        records=total_records,
        pages=pages,
        stale=total_stale,
        submission_pages=submission_pages,
        headers=len(headers),
    )
