"""記録から静的なサイトを作る。

ページごとに必要な形の JSON をここで作る。ブラウザが取るのは 1 ページにつき
2 個だけで、結合も反転もすべて生成の側で済ませる。1 つの大きい JSON を
読ませると、記録が増えたときに携帯で開けなくなる。
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
from pathlib import Path

from .. import environment as env_mod
from .. import problem as problem_mod
from ..freshness import Freshness
from ..paths import ROOT
from ..record import judge_sha
from ..store import Store

TEMPLATES = Path(__file__).resolve().parent / "templates"

# 前のサイトかどうかを見分ける目印。空でないディレクトリを黙って消さないために置く。
MARKER = ".pj-site"

# 失敗したケースの説明。記録は 400 字まで持っているが、表には収まらない。
DETAIL_CHARS = 200

PLACEHOLDER = re.compile(r"\{\{(\w+)\}\}")


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
    # 今のソースで測った記録なら True、ソースが変わっていれば False。
    # 判定できなければ None。
    current: bool | None = None


@dataclass(frozen=True)
class Summary:
    out: Path
    problems: int
    records: int
    pages: int
    stale: int = 0


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
                current=judge(newest),
            )
        )
    return sorted(cells, key=lambda c: (c.env, c.cpu_model, c.submission))


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

    return {
        "id": problem_id,
        "title": problem.title if problem else problem_id,
        "source": problem.testdata.source if problem else "",
        "compare": problem.compare.kind if problem else "",
        "harness": problem.harness_kind if problem else "",
        "tle_sec": problem.limits.tle_sec if problem else 0,
        "mle_mb": problem.limits.mle_mb if problem else 0,
        "case_count": case_count,
        "generated_at": generated_at,
        "repo": repo,
        "submissions": submissions,
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
                "current": c.current,
            }
            for c in cells
        ],
    }


def _write(path: Path, text: str) -> str:
    """書いて、中身のハッシュを ?v= の形で返す。

    Pages のキャッシュヘッダは細かく制御できないので、参照する側の URL に
    混ぜて古いデータを見せないようにする。
    """
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text)
    return "?v=" + hashlib.sha256(text.encode()).hexdigest()[:12]


def _render(template: str, values: dict[str, str]) -> str:
    text = (TEMPLATES / template).read_text()
    for name, value in values.items():
        token = "{{" + name + "}}"
        if token not in text:
            raise SiteError(f"{template} に {token} がありません")
        text = text.replace(token, value)
    left = PLACEHOLDER.search(text)
    if left:
        raise SiteError(f"{template} の {left.group(0)} を埋めていません")
    return text


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

    ids = sorted(set(problems) | set(store.problem_ids()))
    rows = []
    total_records = 0
    total_stale = 0
    pages = 0

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

        combos = len(payload["combos"])
        measured = len(cells)
        stale = sum(1 for c in cells if c.current is False)
        total_stale += stale
        rows.append(
            {
                "id": problem_id,
                "title": payload["title"],
                "source": payload["source"],
                "submissions": len(payload["submissions"]),
                "measured": measured,
                "stale": stale,
                "pending": len(payload["submissions"]) * combos - measured,
                "records": len(records),
                "updated": max((c.timestamp for c in cells), default=""),
            }
        )

    index = {
        "generated_at": generated_at,
        "judge_sha": judge_sha(),
        "record_count": total_records,
        "stale_count": total_stale,
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
    )
