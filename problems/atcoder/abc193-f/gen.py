#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""ABC 193 F (Zebraness) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

seed 0 から 2 は問題文の例、3 と 4 は N = 1 の角のケース、5 から 13 は N = 100 前後の大きいもの (全部 ?、? が無い、? の割合が
1 割、5 割、9 割、市松模様に逆らう色を 1 割だけ決めたもの、1 行おきに色を決めたもの、縁だけ B に決めたもの、N = 99)。
seed 1000 以上は、? の塗り方を全部試す愚直解 (brute.cpp) で解ける N <= 4 の入力。
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
    0: "2\nBB\nBW\n",
    1: "3\nBBB\nBBB\nW?W\n",
    2: "5\n?????\n?????\n?????\n?????\n?????\n",
}


def fmt(g: list[list[str]]) -> str:
    return f"{len(g)}\n" + "".join("".join(row) + "\n" for row in g)


def rand_grid(r: SplitMix64, n: int, q_pct: int) -> list[list[str]]:
    return [["?" if r.range(1, 100) <= q_pct else "BW"[r.range(0, 1)] for _ in range(n)] for _ in range(n)]


def case(seed: int) -> str:
    if seed in SAMPLES:
        return SAMPLES[seed]
    r = SplitMix64(seed * 998244353 + 11)
    if seed == 3:
        return "1\n?\n"
    if seed == 4:
        return "1\nW\n"
    n = 100
    if seed == 5:
        return fmt([["?"] * n for _ in range(n)])
    if seed == 6:
        return fmt(rand_grid(r, n, 0))
    if seed == 7:
        return fmt(rand_grid(r, n, 10))
    if seed == 8:
        return fmt(rand_grid(r, n, 50))
    if seed == 9:
        return fmt(rand_grid(r, n, 90))
    if seed == 10:  # 市松模様と逆の色を 1 割のマスに決める
        return fmt([["BW"[(i + j + 1) & 1] if r.range(1, 10) == 1 else "?" for j in range(n)] for i in range(n)])
    if seed == 11:  # 偶数番の行を交互に B と W で決め、奇数番の行は ?
        return fmt([["BW"[(i >> 1) & 1] if i % 2 == 0 else "?" for _ in range(n)] for i in range(n)])
    if seed == 12:  # 縁だけ B
        return fmt([["B" if i in (0, n - 1) or j in (0, n - 1) else "?" for j in range(n)] for i in range(n)])
    if seed == 13:
        return fmt(rand_grid(r, 99, 70))
    raise SystemExit(f"seed {seed} のケースはありません")


def small_case(seed: int) -> str:
    r = SplitMix64(seed * 1000003 + 5)
    n = r.range(1, 4)
    return fmt(rand_grid(r, n, r.range(0, 100)))


def main() -> None:
    seed = int(sys.argv[1])
    print(small_case(seed) if seed >= 1000 else case(seed), end="")


if __name__ == "__main__":
    main()
