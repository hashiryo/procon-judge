"""リポジトリの中で生成するテストデータ。

gen.py が seed を引数に取ってランダムな入力を stdout に出し、参照実装が期待出力を
作る。count 個の seed を 0 から順に使うので、生成は決定的になる。参照実装は
kind = "base" なら提出と同じ形 (.hpp) でハーネスと一緒にコンパイルし、
kind = "raw" なら単体で動く .cpp をコンパイルする。

置き場は gen.py と参照実装と base.cpp の中身と count で分けてある (fetch.cache_dir_for)。
ジェネレータやハーネスを直せば別の場所に作り直すので、古い生成結果が使い回されない。
"""

from __future__ import annotations

import shutil
import subprocess
import sys
from pathlib import Path

from .. import build as build_mod
from .. import environment as env_mod
from .. import execute
from ..problem import Problem
from . import FetchError, replace_dir

GENERATE_TIMEOUT_SEC = 300
# 参照実装は素朴な書き方でよいので、提出の制限よりずっと長く待つ。
REFERENCE_TIMEOUT_SEC = 600


def case_name(seed: int) -> str:
    return f"seed_{seed:03d}"


def fetch(problem: Problem, dest: Path, env: env_mod.Environment | None = None) -> None:
    """count 個のケースを作って dest に置く。"""
    env = env or env_mod.load("local")
    td = problem.testdata
    generator = problem.dir / td.generator
    reference = Path(td.reference)

    tmp_dir = Path(str(dest) + ".tmp")
    if tmp_dir.exists():
        shutil.rmtree(tmp_dir)
    tmp_dir.mkdir(parents=True)
    build_dir = tmp_dir / "build"

    print(f"  {problem.id} の参照実装を {env.cxx} で組みます", file=sys.stderr)
    built = build_mod.build(problem, reference, env, out_dir=build_dir)
    if not built.ok or built.binary is None:
        shutil.rmtree(tmp_dir)
        raise FetchError(f"{problem.id}: 参照実装 {reference} がコンパイルできません\n{built.log}")

    print(f"  {problem.id} を {td.count} ケース生成します...", file=sys.stderr)
    for seed in range(td.count):
        name = case_name(seed)
        in_path, out_path = tmp_dir / f"{name}.in", tmp_dir / f"{name}.out"
        _generate(generator, seed, in_path, problem)
        result = execute.run(
            built.binary,
            stdin_path=in_path,
            stdout_path=out_path,
            stderr_path=tmp_dir / "reference.stderr",
            tle_sec=REFERENCE_TIMEOUT_SEC,
        )
        if result.timed_out or result.crashed:
            shutil.rmtree(tmp_dir)
            raise FetchError(
                f"{problem.id}: 参照実装が {name} で落ちました "
                f"(exit {result.exit_code}, signal {result.term_signal}, timeout {result.timed_out})"
            )

    shutil.rmtree(build_dir, ignore_errors=True)
    (tmp_dir / "reference.stderr").unlink(missing_ok=True)
    replace_dir(tmp_dir, dest)


def _generate(generator: Path, seed: int, out: Path, problem: Problem) -> None:
    """gen.py を seed 1 つで走らせて入力を書く。依存は PEP 723 の形で書いてあれば uv が用意する。"""
    cmd = ["uv", "run", "--quiet", "--script", str(generator), str(seed)]
    try:
        with out.open("wb") as f:
            proc = subprocess.run(
                cmd, cwd=problem.dir, stdout=f, stderr=subprocess.PIPE,
                check=False, timeout=GENERATE_TIMEOUT_SEC,
            )
    except (subprocess.SubprocessError, OSError) as e:
        raise FetchError(f"{problem.id}: {generator.name} が動きません: {e}") from e
    if proc.returncode != 0:
        raise FetchError(
            f"{problem.id}: {generator.name} {seed} が失敗しました: "
            f"{proc.stderr.decode(errors='replace').strip()}"
        )
