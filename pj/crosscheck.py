"""pj testdata crosscheck。source = "local" の参照実装を、愚直解と小さい入力で突き合わせる。

参照実装は期待出力を作るので、間違っていると正しい提出が WA になり、同じ間違いをした提出が
AC になる。愚直解は遅くてよいが、参照実装とは別の考え方で書く。gen.py が小さい入力を出す
seed を渡して、両方の出力を問題の比べ方 (tokens か float) で比べる。記録もテストデータも
書かない。

gen.py は 1000 以上の seed で小さい入力を出す決まりにしてある (README の「テストデータを
自作する」)。既定の seed はその範囲から取る。
"""

from __future__ import annotations

import sys
from pathlib import Path
from typing import TextIO

from . import build as build_mod
from . import compare as compare_mod
from . import execute
from .environment import Environment
from .fetch import local
from .paths import CACHE_DIR
from .problem import Problem

CROSSCHECK_CACHE_DIR = CACHE_DIR / "crosscheck"
DEFAULT_SEEDS = range(1000, 1300)
# 愚直解に待つ時間。小さい入力で走らせる前提なので、超えるなら入力が大きすぎる。
BRUTE_TIMEOUT_SEC = 60.0
# 食い違いを詳しく出す数の上限。
REPORT_MISMATCHES = 5


class CrosscheckError(Exception):
    """突き合わせられないときに投げる。"""


def parse_seeds(text: str) -> range:
    """A:B を range(A, B) にする。"""
    first, sep, last = text.partition(":")
    try:
        seeds = range(int(first), int(last)) if sep else range(int(first), int(first) + 1)
    except ValueError as e:
        raise CrosscheckError(f"--seeds は A:B の形で渡してください ({text!r})") from e
    if len(seeds) == 0:
        raise CrosscheckError(f"--seeds {text!r} は空です")
    return seeds


def crosscheck(
    problem: Problem,
    brute: Path,
    env: Environment,
    *,
    seeds: range = DEFAULT_SEEDS,
    out: TextIO | None = None,
) -> int:
    """全部一致なら 0、食い違いか落ちたものがあれば 1。brute は問題のディレクトリからの相対パス。"""
    out = out or sys.stdout
    td = problem.testdata
    if td.source != "local":
        raise CrosscheckError(f"{problem.id} は testdata.source = 'local' の問題ではありません")
    if problem.compare.kind not in ("tokens", "float"):
        raise CrosscheckError(
            f"compare.kind = {problem.compare.kind!r} の問題は突き合わせられません (tokens と float だけ)"
        )
    if not (problem.dir / brute).is_file():
        raise CrosscheckError(f"{problem.id} に愚直解 {brute} がありません")

    work = CROSSCHECK_CACHE_DIR / problem.id
    work.mkdir(parents=True, exist_ok=True)
    binaries: dict[str, Path] = {}
    for label, source in (("reference", Path(td.reference)), ("brute", brute)):
        print(f"{source} を {env.cxx} で組みます", file=out)
        built = build_mod.build(problem, source, env, out_dir=work / "build" / label)
        if not built.ok or built.binary is None:
            print(f"CE ({build_mod.summary(built)})", file=out)
            print(built.log, file=out)
            return 1
        binaries[label] = built.binary

    generator = problem.dir / td.generator
    limits = {"reference": local.REFERENCE_TIMEOUT_SEC, "brute": BRUTE_TIMEOUT_SEC}
    mismatches = failures = 0
    for seed in seeds:
        stem = f"seed_{seed:04d}"
        in_path = work / f"{stem}.in"
        local.generate(generator, seed, in_path, problem)
        outputs: dict[str, Path] = {}
        crashed = False
        for label, binary in binaries.items():
            outputs[label] = work / f"{stem}.{label}"
            result = execute.run(
                binary, stdin_path=in_path, stdout_path=outputs[label],
                stderr_path=work / f"{stem}.{label}.err", tle_sec=limits[label],
            )
            if result.timed_out or result.crashed:
                crashed = True
                failures += 1
                print(
                    f"  {stem}: {label} が落ちました (exit {result.exit_code}, "
                    f"signal {result.term_signal}, timeout {result.timed_out})  {in_path}",
                    file=out,
                )
                break
        if crashed:
            continue
        verdict = compare_mod.compare(
            problem.compare.kind,
            input_path=in_path,
            actual_path=outputs["reference"],
            expected_path=outputs["brute"],
            abs_tol=problem.compare.abs_tol,
            rel_tol=problem.compare.rel_tol,
        )
        if verdict.ok:
            continue
        mismatches += 1
        if mismatches <= REPORT_MISMATCHES:
            print(f"  {stem}: 食い違い ({verdict.detail})", file=out)
            print(f"    入力       {in_path}", file=out)
            print(f"    参照実装   {outputs['reference']}", file=out)
            print(f"    愚直解     {outputs['brute']}", file=out)

    agreed = len(seeds) - mismatches - failures
    print(
        f"\n{len(seeds)} ケース中 {agreed} 件一致 / 食い違い {mismatches} 件 / 落ちた {failures} 件",
        file=out,
    )
    return 0 if mismatches == 0 and failures == 0 else 1
