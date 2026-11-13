#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""ABC 247 G (Dream Team) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

seed 0 と 1 は問題文の例、2 から 4 は角のケース (N = 1、大学が 1 つだけ、全員が同じ大学と分野)、5 から 11 は N = 3 * 10^4 の
ランダムと遅くなりやすい形 (強さの種類が少ない、全員が同じ強さ、分野が 5 つだけ、大学と分野が同じ番号の人だけ、強さが
大学と分野の差で決まる、大学と分野の組が 150 * 150 のすべてにある)。seed 1000 以上は、部分集合を全部試す愚直解
(brute.cpp) で解ける N <= 10 の入力。
"""
import sys

MASK64 = (1 << 64) - 1
NMAX, AMAX, CMAX = 30000, 150, 10**9


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


def fmt(people: list[tuple[int, int, int]]) -> str:
    return f"{len(people)}\n" + "".join(f"{a} {b} {c}\n" for a, b, c in people)


SAMPLES = {
    0: "3\n1 1 100\n1 20 10\n2 1 1\n",
    1: (
        "10\n1 4 142135623\n2 6 457513110\n3 1 622776601\n5 1 961524227\n2 2 360679774\n"
        "2 4 494897427\n3 7 416573867\n5 2 915026221\n1 7 320508075\n5 3 851648071\n"
    ),
}


def case(seed: int) -> str:
    if seed in SAMPLES:
        return SAMPLES[seed]
    r = SplitMix64(seed * 998244353 + 7)
    if seed == 2:
        return fmt([(r.range(1, AMAX), r.range(1, AMAX), r.range(1, CMAX))])
    if seed == 3:
        return fmt([(77, r.range(1, AMAX), r.range(1, CMAX)) for _ in range(NMAX)])
    if seed == 4:
        return fmt([(150, 150, r.range(1, CMAX)) for _ in range(NMAX)])
    if seed == 5:
        return fmt([(r.range(1, AMAX), r.range(1, AMAX), r.range(1, CMAX)) for _ in range(NMAX)])
    if seed == 6:
        return fmt([(r.range(1, AMAX), r.range(1, AMAX), r.range(1, 10)) for _ in range(NMAX)])
    if seed == 7:
        return fmt([(r.range(1, AMAX), r.range(1, AMAX), CMAX) for _ in range(NMAX)])
    if seed == 8:
        return fmt([(r.range(1, AMAX), r.range(1, 5), r.range(1, CMAX)) for _ in range(NMAX)])
    if seed == 9:
        ps = [(a, a, r.range(1, CMAX)) for a in range(1, AMAX + 1)]
        while len(ps) < NMAX:
            a = r.range(1, AMAX)
            ps.append((a, a, r.range(1, CMAX)))
        return fmt(ps)
    if seed == 10:
        ps = []
        for _ in range(NMAX):
            a, b = r.range(1, AMAX), r.range(1, AMAX)
            ps.append((a, b, CMAX - abs(a - b) * 6_000_000 - r.range(0, 1000)))
        return fmt(ps)
    if seed == 11:
        ps = [(a, b, r.range(1, CMAX)) for a in range(1, AMAX + 1) for b in range(1, AMAX + 1)]
        while len(ps) < NMAX:
            ps.append((r.range(1, AMAX), r.range(1, AMAX), r.range(1, CMAX)))
        return fmt(ps)
    raise SystemExit(f"seed {seed} は 0 から 11 か、1000 以上にしてください")


def small_case(seed: int) -> str:
    r = SplitMix64(seed * 1000003 + 3)
    n = r.range(1, 10)
    am, bm, cm = r.range(1, 4), r.range(1, 4), r.range(1, 20)
    return fmt([(r.range(1, am), r.range(1, bm), r.range(1, cm)) for _ in range(n)])


def main() -> None:
    seed = int(sys.argv[1])
    print(small_case(seed) if seed >= 1000 else case(seed), end="")


if __name__ == "__main__":
    main()
