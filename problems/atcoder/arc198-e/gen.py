#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""AtCoder ARC198 E (Monotone OR) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

1 行目に N と M、2 行目に 2^N 未満の相異なる M 個の整数 (小さい順)。seed 0 から 2 は問題文の例 1 から 3。seed 3 から 13 は
N = 24 の乱数 (M = 2e5 が 2 つ、1000、1)、s = {0}、0 から 199999、2^24 - 200000 から 2^24 - 1、popcount の小さい順の 2e5 個、
N = 17 の全部の値、N = 20 の乱数、N = 1 の {0, 1}。seed 1000 以上は N = 1 から 5 の小さい入力 (愚直解 brute.hpp 用)。
乱数は Python の random の版による違いを避けるため splitmix64 で作る。
"""
import sys

MASK64 = (1 << 64) - 1

SAMPLE3 = [673802, 709603, 941436, 987977, 1288854, 1448514, 1890649, 2031791, 2194398, 3531579, 3540682, 4352378, 4676427,
           5094869, 5243789, 6064976, 6412917, 7164733, 8403938, 9123034, 10396333, 10558625, 10726446, 12263566, 12421464,
           12503511, 12676340, 14032527, 14268967, 14669703, 15823827, 16285412]


class SplitMix64:
    def __init__(self, seed: int) -> None:
        self.x = seed & MASK64

    def next(self) -> int:
        self.x = (self.x + 0x9E3779B97F4A7C15) & MASK64
        z = self.x
        z = ((z ^ (z >> 30)) * 0xBF58476D1CE4E5B9) & MASK64
        z = ((z ^ (z >> 27)) * 0x94D049BB133111EB) & MASK64
        return z ^ (z >> 31)


def distinct(rng: SplitMix64, n: int, m: int) -> list[int]:
    seen: set[int] = set()
    while len(seen) < m:
        seen.add(rng.next() % (1 << n))
    return sorted(seen)


def main() -> None:
    seed = int(sys.argv[1])
    rng = SplitMix64(seed * 1000003 + 77)
    if seed == 0:
        n, s = 2, [1, 2]
    elif seed == 1:
        n, s = 5, [3, 5, 8, 9, 11, 16, 17, 23, 27, 28]
    elif seed == 2:
        n, s = 24, SAMPLE3
    elif seed in (3, 4):
        n = 24
        s = distinct(rng, n, 200000)
    elif seed == 5:
        n = 24
        s = distinct(rng, n, 1000)
    elif seed == 6:
        n = 24
        s = distinct(rng, n, 1)
    elif seed == 7:
        n, s = 24, [0]
    elif seed == 8:
        n, s = 24, list(range(200000))
    elif seed == 9:
        n, s = 24, list(range((1 << 24) - 200000, 1 << 24))
    elif seed == 10:
        n = 24
        s = sorted(sorted(range(1 << n), key=lambda v: (bin(v).count("1"), v))[:200000])
    elif seed == 11:
        n = 17
        s = list(range(1 << n))
    elif seed == 12:
        n = 20
        s = distinct(rng, n, 200000)
    elif seed == 13:
        n, s = 1, [0, 1]
    else:
        n = 1 + rng.next() % 5
        m = 1 + rng.next() % min(1 << n, 10)
        s = distinct(rng, n, m)
    print(n, len(s))
    print(" ".join(map(str, s)))


if __name__ == "__main__":
    main()
