#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""ABC 259 G (Grid Card Game) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

seed 0 と 1 は問題文の例、2 から 4 は H = W = 1 の角のケース (正、負、0)、5 から 13 は H = W = 100 前後 (乱択、全部が正、
全部が負、負のマスが 1 割、負のマスが 9 割、符号が市松模様、絶対値が最大、0 が多く値の小さいもの、H = 100 で W = 1)。
seed 1000 以上は、行と列の選び方を全部試す愚直解 (brute.cpp) で解ける H, W <= 6 の入力。
"""
import sys

MASK64 = (1 << 64) - 1
AMAX = 10**9


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


SAMPLES = {
    0: "2 3\n-9 5 1\n6 -2 4\n",
    1: '15 20\n-14 74 -48 38 -51 43 5 37 -39 -29 80 -44 -55 59 17 89 -37 -68 38 -16\n14 31 43 -73 49 -7 -65 13 -40 -45 36 88 -54 -43 99 87 -94 57 -22 31\n-85 67 -46 23 95 68 55 17 -56 51 -38 64 32 -19 65 -62 76 66 -53 -16\n35 -78 -41 35 -51 -85 24 -22 45 -53 82 -30 39 19 -52 -3 -11 -67 -33 71\n-75 45 -80 -42 -31 94 59 -58 39 -26 -94 -60 98 -1 21 25 0 -86 37 4\n-41 66 -53 -55 55 98 23 33 -3 -27 7 -53 -64 68 -33 -8 -99 -15 50 40\n66 53 -65 5 -49 81 45 1 33 19 0 20 -46 -82 14 -15 -13 -65 68 -65\n50 -66 63 -71 84 51 -91 45 100 76 -7 -55 45 -72 18 40 -42 73 69 -36\n59 -65 -30 89 -10 43 7 72 93 -70 23 86 81 16 25 -63 73 16 34 -62\n22 -88 27 -69 82 -54 -92 32 -72 -95 28 -25 28 -55 97 87 91 17 21 -95\n62 39 -65 -16 -84 51 62 -44 -60 -70 8 69 -7 74 79 -12 62 -86 6 -60\n-72 -6 -79 -28 39 -42 -80 -17 -95 -28 -66 66 36 86 -68 91 -23 70 58 2\n-19 -20 77 0 65 -94 -30 76 55 57 -8 59 -43 -6 -15 -83 8 29 16 34\n79 40 86 -92 88 -70 -94 -21 50 -3 -42 -35 -79 91 96 -87 -93 -6 46 27\n-94 -49 71 37 91 47 97 1 21 32 -100 -4 -78 -47 -36 -84 -61 86 -51 -9\n',
}


def fmt(a: list[list[int]]) -> str:
    return f"{len(a)} {len(a[0])}\n" + "".join(" ".join(map(str, row)) + "\n" for row in a)


def grid(r: SplitMix64, h: int, w: int, neg_pct: int, m: int = AMAX) -> list[list[int]]:
    return [[-r.range(1, m) if r.range(1, 100) <= neg_pct else r.range(0, m) for _ in range(w)] for _ in range(h)]


def case(seed: int) -> str:
    if seed in SAMPLES:
        return SAMPLES[seed]
    r = SplitMix64(seed * 998244353 + 17)
    n = 100
    if seed == 2:
        return fmt([[AMAX]])
    if seed == 3:
        return fmt([[-AMAX]])
    if seed == 4:
        return fmt([[0]])
    if seed == 5:
        return fmt(grid(r, n, n, 50))
    if seed == 6:
        return fmt(grid(r, n, n, 0))
    if seed == 7:
        return fmt(grid(r, n, n, 100))
    if seed == 8:
        return fmt(grid(r, n, n, 10))
    if seed == 9:
        return fmt(grid(r, n, n, 90))
    if seed == 10:
        return fmt([[(AMAX - r.range(0, 1000)) * (1 if (i + j) % 2 == 0 else -1) for j in range(n)] for i in range(n)])
    if seed == 11:
        return fmt([[AMAX if r.range(0, 3) else -AMAX for _ in range(n)] for _ in range(n)])
    if seed == 12:
        return fmt([[0 if r.range(0, 2) else r.range(-3, 3) for _ in range(n)] for _ in range(n)])
    if seed == 13:
        return fmt(grid(r, n, 1, 40))
    raise SystemExit(f"seed {seed} のケースはありません")


def small_case(seed: int) -> str:
    r = SplitMix64(seed * 1000003 + 9)
    h, w = r.range(1, 6), r.range(1, 6)
    m = [3, 100, AMAX][r.range(0, 2)]
    return fmt(grid(r, h, w, r.range(0, 100), m))


def main() -> None:
    seed = int(sys.argv[1])
    print(small_case(seed) if seed >= 1000 else case(seed), end="")


if __name__ == "__main__":
    main()
