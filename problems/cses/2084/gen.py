#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""CSES 2084 (Monster Game I) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

seed 0 から 13 は本番のケース。例、角 (n = 1、最小、全部同じ、最大)、小さいランダム、
制約いっぱいのランダム、遅くなりやすい形 (s が狭義単調増加、f が一定、値が 3 種類だけ) の順に並べる。
seed 1000 以上は、愚直な O(n^2) の DP (brute.hpp) で解ける n <= 60 の入力で、
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


def nondec(r: SplitMix64, n: int, lo: int, hi: int) -> list:
    return sorted(r.ints(n, lo, hi))


def noninc(r: SplitMix64, n: int, hi: int) -> list:
    """1 <= f_n <= ... <= f_1 <= hi の列。"""
    return sorted(r.ints(n, 1, hi), reverse=True)


def case(seed: int) -> tuple:
    r = SplitMix64(0x2084_5EED + seed)
    if seed == 0:
        return 5, 100, [20, 30, 30, 50, 90], [90, 60, 20, 20, 10]
    if seed == 1:
        return 1, 1, [1], [1]
    if seed == 2:
        return 8, 20, nondec(r, 8, 1, 20), noninc(r, 8, 20)
    if seed == 3:
        return 10, 1000000, [5] * 10, [1] * 10
    if seed == 4:
        return 10, 1, [1] * 10, [1] * 10
    if seed == 5:
        n = 200000
        return n, 1000000, nondec(r, n, 1, 1000000), noninc(r, n, 1000000)
    if seed == 6:
        n = 200000
        return n, 1000000, [1000000] * n, [1] * n
    if seed == 7:
        return 1, 1000000, [1000000], [1]
    if seed == 8:
        n = 200000
        return n, 1000000, [i + 1 for i in range(n)], [1000000 - i for i in range(n)]
    if seed == 9:
        n = 200000
        return n, 1000000, nondec(r, n, 1, 1000000), [1000000] * n
    if seed == 10:
        n = 200000
        return n, 777, nondec(r, n, 1, 1000000), noninc(r, n, 777)
    if seed == 11:
        n = 1000
        return n, 100000, nondec(r, n, 1, 1000000), noninc(r, n, 100000)
    if seed == 12:
        n = 200000
        f = [(1, 500000, 1000000)[r.range(0, 2)] for _ in range(n)]
        f.sort(reverse=True)
        return n, 1000000, nondec(r, n, 1, 1000000), f
    if seed == 13:
        n = 200000
        return n, 1000000, [1000000] * n, [1000000] * n
    raise SystemExit(f"no case: {seed}")


def small(seed: int) -> tuple:
    r = SplitMix64(seed)
    n = r.range(1, 60)
    x = r.range(1, 1000000)
    return n, x, nondec(r, n, 1, 1000000), noninc(r, n, x)


def main() -> None:
    seed = int(sys.argv[1])
    n, x, s, f = small(seed) if seed >= 1000 else case(seed)
    print(n, x)
    print(*s)
    print(*f)


if __name__ == "__main__":
    main()
