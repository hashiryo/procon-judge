#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""u32 の argsort の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。ケースは self-sort-u32 と同じ。

入力は配列の作り方だけで、配列そのものはハーネスが作る (_shared/sort/_common.hpp)。
1 行目が配列の個数 T、続く T 行が「分布 N seed 引数」で、引数を使わない分布でも 0 を書く。
期待出力は参照実装 (problem.toml の reference) がハーネスと一緒に作るので、ここでは入力だけを書く。
seed 0 の小さい配列の選び方は、Python の random の版による違いを避けるため splitmix64 で決める。
"""
import sys

MASK64 = (1 << 64) - 1
U32_MAX = (1 << 32) - 1
M = 10**6


class SplitMix64:
    def __init__(self, seed: int) -> None:
        self.x = seed & MASK64

    def next(self) -> int:
        self.x = (self.x + 0x9E3779B97F4A7C15) & MASK64
        z = self.x
        z = ((z ^ (z >> 30)) * 0xBF58476D1CE4E5B9) & MASK64
        z = ((z ^ (z >> 27)) * 0x94D049BB133111EB) & MASK64
        return z ^ (z >> 31)

    def below(self, n: int) -> int:
        return (self.next() * n) >> 64

    def choice(self, xs: list):
        return xs[self.below(len(xs))]


# 分布ごとの引数の候補 (seed 0 の小さい配列で使う)。
SMALL_ARGS = {
    "range": [U32_MAX, 10**9, (1 << 16) - 1, 255, 1],
    "few": [1, 2, 3, 16],
    "equal": [0],
    "sorted": [0],
    "reversed": [0],
    "nearly": [1, 3, 10],
    "mask": [0xFFFF0000, 0xF0F0F0F0, 0x80000001, 0],
}


def small_case() -> list[tuple[str, int, int]]:
    """N が 0 から 100 の配列を 100 個。端 (N = 0, 1, 2) を先に並べ、残りは分布も N もランダムに選ぶ。"""
    rows = [("range", 0, U32_MAX), ("range", 1, U32_MAX), ("range", 2, U32_MAX), ("equal", 2, 0), ("sorted", 3, 0), ("reversed", 3, 0)]
    rng = SplitMix64(20261008)
    dists = list(SMALL_ARGS)
    while len(rows) < 100:
        dist = rng.choice(dists)
        rows.append((dist, rng.below(101), rng.choice(SMALL_ARGS[dist])))
    return rows


# seed -> 配列の並び (分布, N, 引数)
CASES = {
    0: small_case(),
    1: [("range", 1000, U32_MAX)],
    2: [("range", 200000, 10**9)],
    3: [("range", M, U32_MAX)],
    4: [("range", M, 10**9)],
    5: [("range", M, (1 << 16) - 1)],
    6: [("few", M, 16)],
    7: [("equal", M, 0)],
    8: [("sorted", M, 0)],
    9: [("reversed", M, 0)],
    10: [("nearly", M, 1000)],
    11: [("mask", M, 0xFFFF0000)],
}


def main() -> None:
    seed = int(sys.argv[1])
    if seed not in CASES:
        raise SystemExit(f"seed {seed} は {len(CASES)} 未満にしてください")
    rows = CASES[seed]
    out = [str(len(rows))]
    # 配列ごとの乱数の種は、ケースの seed と行の番号から決める。
    out += [f"{dist} {n} {seed * 1_000_003 + i} {arg}" for i, (dist, n, arg) in enumerate(rows)]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
