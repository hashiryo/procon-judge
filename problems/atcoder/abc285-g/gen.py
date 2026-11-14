#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""ABC 285 G (Tatami) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

seed 0 から 2 は問題文の例、3 から 5 は 1 マスと 2 マスの角のケース、6 から 15 は H, W が 300 前後の大きいもの。大きいものは、
全部 ?、全部 2 (300 × 300 と、マスの数が奇数の 299 × 299)、全部 2 の中に色の違う 1 を 2 つだけ遠くに置いたもの (Yes) と色の
同じ 1 を 2 つ置いたもの (No)、先にでたらめな敷き方を作ってから文字を決めたもの (2 つ。どれも Yes)、文字を全部決めた敷き方の 1 を
1 つだけ 2 に変えたもの (その連結成分の 2 が奇数個になるので No)、でたらめな文字の 2 つ。seed 1000 以上は、敷き方を全部試す
愚直解 (brute.cpp) で解ける H, W <= 5 の入力。
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


SAMPLES = {
    0: "3 4\n2221\n?1??\n2?21\n",
    1: "3 4\n2?21\n??1?\n2?21\n",
    2: "5 5\n11111\n11111\n11211\n11111\n11111\n",
}


def fmt(g: list[list[str]]) -> str:
    return f"{len(g)} {len(g[0])}\n" + "".join("".join(row) + "\n" for row in g)


def rand_grid(r: SplitMix64, h: int, w: int, p1: int, p2: int) -> list[list[str]]:
    """1 を p1 %、2 を p2 %、残りを ? にする。"""

    def one() -> str:
        x = r.range(1, 100)
        return "1" if x <= p1 else "2" if x <= p1 + p2 else "?"

    return [[one() for _ in range(w)] for _ in range(h)]


def tiled_grid(r: SplitMix64, h: int, w: int, pdom: int, p2: int, p1: int) -> list[list[str]]:
    """上の行から順に、まだ覆っていないマスに pdom % で 1 × 2 を (右か下へ) 置き、ほかは 1 × 1 にした敷き方を作る。1 × 2 の
    マスは p2 % で 2、1 × 1 のマスは p1 % で 1 にし、残りを ? にする。この敷き方が条件を満たすので、答えは Yes になる。"""
    kind = [[0] * w for _ in range(h)]  # 0: まだ、1: 1 × 1、2: 1 × 2
    for i in range(h):
        for j in range(w):
            if kind[i][j]:
                continue
            opts = []
            if j + 1 < w and not kind[i][j + 1]:
                opts.append((i, j + 1))
            if i + 1 < h:
                opts.append((i + 1, j))
            if opts and r.range(1, 100) <= pdom:
                a, b = opts[r.range(0, len(opts) - 1)]
                kind[i][j] = kind[a][b] = 2
            else:
                kind[i][j] = 1
    g = [["?"] * w for _ in range(h)]
    for i in range(h):
        for j in range(w):
            if kind[i][j] == 2 and r.range(1, 100) <= p2:
                g[i][j] = "2"
            if kind[i][j] == 1 and r.range(1, 100) <= p1:
                g[i][j] = "1"
    return g


def case(seed: int) -> str:
    if seed in SAMPLES:
        return SAMPLES[seed]
    r = SplitMix64(seed * 998244353 + 7)
    if seed == 3:
        return "1 1\n2\n"
    if seed == 4:
        return "1 1\n?\n"
    if seed == 5:
        return "1 2\n22\n"
    n = 300
    if seed == 6:
        return fmt([["?"] * n for _ in range(n)])
    if seed == 7:
        return fmt([["2"] * n for _ in range(n)])
    if seed == 8:
        return fmt([["2"] * 299 for _ in range(299)])
    if seed == 9:  # 色の違う 1 を 2 つ。残りは 1 × 2 で敷き詰められる (Yes)
        g = [["2"] * n for _ in range(n)]
        g[0][0] = g[n - 1][0] = "1"  # (0, 0) と (n - 1, 0) は n が偶数なので色が違う
        return fmt(g)
    if seed == 10:  # 色の同じ 1 を 2 つ (No)
        g = [["2"] * n for _ in range(n)]
        g[0][0] = g[n - 1][n - 1] = "1"
        return fmt(g)
    if seed == 11:
        return fmt(tiled_grid(r, n, n, 95, 100, 100))
    if seed == 12:
        return fmt(tiled_grid(r, n, n, 70, 90, 50))
    if seed == 13:  # 文字を全部決めた敷き方の 1 を 1 つだけ 2 に変える (No)
        g = tiled_grid(r, n, n, 80, 100, 100)
        ones = [(i, j) for i in range(n) for j in range(n) if g[i][j] == "1"]
        i, j = ones[r.range(0, len(ones) - 1)]
        g[i][j] = "2"
        return fmt(g)
    if seed == 14:
        return fmt(rand_grid(r, n, n, 20, 70))
    if seed == 15:
        return fmt(rand_grid(r, 300, 299, 2, 40))
    raise SystemExit(f"seed {seed} のケースはありません")


def small_case(seed: int) -> str:
    r = SplitMix64(seed * 1000003 + 5)
    h, w = r.range(1, 5), r.range(1, 5)
    if r.range(0, 1):
        return fmt(tiled_grid(r, h, w, r.range(0, 100), r.range(0, 100), r.range(0, 100)))
    p1 = r.range(0, 40)
    return fmt(rand_grid(r, h, w, p1, r.range(0, 100 - p1)))


def main() -> None:
    seed = int(sys.argv[1])
    print(small_case(seed) if seed >= 1000 else case(seed), end="")


if __name__ == "__main__":
    main()
