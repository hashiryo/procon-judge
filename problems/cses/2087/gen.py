#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""CSES 2087 (Houses and Schools) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

seed 0 から 13 は本番のケース。例、角 (n = 1、k = 1、k = n、最大の子の数)、小さいランダム、
制約いっぱいのランダム、遅くなりやすい形 (全部同じ、増える、2 種類だけ) の順に並べる。
seed 1000 以上は、愚直な O(k n^2) の DP (brute.hpp) で解ける n <= 40 の入力で、
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
    r = SplitMix64(0x2087_5EED + seed)
    if seed == 0:
        return 6, 2, [2, 7, 1, 4, 6, 4]
    if seed == 1:
        return 1, 1, [1]
    if seed == 2:
        return 2, 1, [1, 1]
    if seed == 3:
        return 2, 2, [1000000000, 1]
    if seed == 4:
        return 10, 1, r.ints(10, 1, 20)
    if seed == 5:
        return 10, 10, r.ints(10, 1, 20)
    if seed == 6:
        n = 3000
        return n, 1, r.ints(n, 1, 1000000000)
    if seed == 7:
        n = 3000
        return n, 2, r.ints(n, 1, 1000000000)
    if seed == 8:
        n = 3000
        return n, 1500, r.ints(n, 1, 1000000000)
    if seed == 9:
        n = 3000
        return n, n, r.ints(n, 1, 1000000000)
    if seed == 10:
        n = 3000
        return n, 1, [1000000000] * n
    if seed == 11:
        n = 3000
        return n, 1500, [1] * n
    if seed == 12:
        n = 3000
        return n, 1000, [i * 333333 + 1 for i in range(n)]
    if seed == 13:
        n = 3000
        return n, 500, [(1, 1000000000)[r.range(0, 1)] for _ in range(n)]
    raise SystemExit(f"no case: {seed}")


def small(seed: int) -> tuple:
    r = SplitMix64(seed)
    n = r.range(1, 40)
    k = r.range(1, n)
    return n, k, r.ints(n, 1, 1000000000)


def main() -> None:
    seed = int(sys.argv[1])
    n, k, c = small(seed) if seed >= 1000 else case(seed)
    print(n, k)
    print(*c)


if __name__ == "__main__":
    main()
