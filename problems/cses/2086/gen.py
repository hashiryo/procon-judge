#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""CSES 2086 (Subarray Squares) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

seed 0 から 13 は本番のケース。小さいランダム、角 (n = 1、k = 1、k = n、全部同じ、0 だけ、最大値)、
制約いっぱいのランダム、遅くなりやすい形 (値が増える、減る、3 種類だけ) の順に並べる。
seed 1000 以上は、愚直な O(k n^2) の DP (brute.cpp) で解ける n <= 40 の入力で、
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
    r = SplitMix64(0x2086_5EED + seed)
    if seed == 0:
        return 5, 2, [3, 1, 4, 1, 5]
    if seed == 1:
        return 1, 1, [1]
    if seed == 2:
        return 8, 3, r.ints(8, 1, 20)
    if seed == 3:
        return 10, 10, [100000] * 10
    if seed == 4:
        return 12, 4, [0] * 12
    if seed == 5:
        return 2, 2, [100000, 100000]
    if seed == 6:
        return 3000, 1, r.ints(3000, 1, 100000)
    if seed == 7:
        return 3000, 2, r.ints(3000, 1, 100000)
    if seed == 8:
        return 3000, 1500, [1] * 3000
    if seed == 9:
        return 3000, 1500, [100000] * 3000
    if seed == 10:
        return 3000, 1500, r.ints(3000, 1, 100000)
    if seed == 11:
        return 3000, 3000, r.ints(3000, 1, 100000)
    if seed == 12:
        return 3000, 1000, [33 * i + 1 for i in range(3000)]
    if seed == 13:
        return 3000, 777, [(1, 2, 100000)[r.range(0, 2)] for _ in range(3000)]
    raise SystemExit(f"no case: {seed}")


def small(seed: int) -> tuple:
    r = SplitMix64(seed)
    n = r.range(1, 40)
    k = r.range(1, n)
    return n, k, r.ints(n, 0, 20)


def main() -> None:
    seed = int(sys.argv[1])
    n, k, xs = small(seed) if seed >= 1000 else case(seed)
    print(n, k)
    print(*xs)


if __name__ == "__main__":
    main()
