#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""CSES 2088 (Knuth Division) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

seed 0 から 13 は本番のケース。例、角 (n = 1、n = 2、全部同じ、最大値)、小さいランダム、
制約いっぱいのランダム、遅くなりやすい形 (増える、値が 2 種類だけ、1 つだけ大きい) の順に並べる。
seed 1000 以上は、愚直な O(n^3) の区間 DP (brute.hpp) で解ける n <= 60 の入力で、
大きさと値を splitmix64 で選ぶ。
"""
import sys

MASK64 = (1 << 64) - 1


class SplitMix64:
    def __init__(self, seed: int) -> None:
        self.x = seed & MASK64

    def next(self) -> int:
        self.x = (self.x + 0x9E3779B97F4A7C15) & MASK64
        z = self.x
        z = ((z ^ (z >> 30)) * 0xBF58476D1CE4E5B9) & MASK64
        z = ((z ^ (z >> 27)) * 0x94D049BB133111EB) & MASK64
        return z ^ (z >> 31)

    def range(self, lo: int, hi: int) -> int:
        return lo + (self.next() * (hi - lo + 1) >> 64)

    def ints(self, n: int, lo: int, hi: int) -> list:
        return [self.range(lo, hi) for _ in range(n)]


def case(seed: int) -> tuple:
    r = SplitMix64(0x2088_5EED + seed)
    if seed == 0:
        return 5, [2, 7, 3, 2, 5]
    if seed == 1:
        return 1, [1000000000]
    if seed == 2:
        return 2, [1, 1]
    if seed == 3:
        return 10, [1] * 10
    if seed == 4:
        n = 5000
        return n, [1] * n
    if seed == 5:
        n = 5000
        return n, [1000000000] * n
    if seed == 6:
        n = 5000
        return n, r.ints(n, 1, 1000000000)
    if seed == 7:
        n = 5000
        return n, [i + 1 for i in range(n)]
    if seed == 8:
        n = 5000
        return n, [(1, 1000000000)[r.range(0, 1)] for _ in range(n)]
    if seed == 9:
        n = 4999
        return n, r.ints(n, 1, 1000000000)
    if seed == 10:
        n = 5000
        return n, [1000000000] + [1] * (n - 1)
    if seed == 11:
        return 2, [1000000000, 1000000000]
    if seed == 12:
        n = 5000
        return n, [1 if i % 2 == 0 else 1000000000 for i in range(n)]
    if seed == 13:
        n = 3000
        return n, r.ints(n, 1, 1000)
    raise SystemExit(f"no case: {seed}")


def small(seed: int) -> tuple:
    r = SplitMix64(seed)
    n = r.range(1, 60)
    return n, r.ints(n, 1, 1000000000)


def main() -> None:
    seed = int(sys.argv[1])
    n, xs = small(seed) if seed >= 1000 else case(seed)
    print(n)
    print(*xs)


if __name__ == "__main__":
    main()
