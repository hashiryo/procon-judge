#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""洛谷 P9131 ([USACO23FEB] Problem Setting P) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

1 行目に N と M、続く M 行に長さ N の文字列 (E か H)。seed 0 と 1 は問題文の例 1 と 2。seed 2 から 11 は N = 1e5 前後で、評価の作り方を変える:
M = 20 の一様な乱数 (hard の人の集合 S がほとんど重ならず c_S が小さい)、問題ごとの難しさ x と人ごとの閾値で決める (S が入れ子に近く、
雑音の大きさを変える)、S の種類が 40 だけ (c_S が大きい)、M = 16 の乱数、N = 1、M = 1、全部 E、全部 H、半分は入れ子で半分は乱数。
seed 1000 以上は M = 1 から 6、N <= 8 の小さい入力 (愚直解 brute.hpp 用)。
乱数は Python の random の版による違いを避けるため splitmix64 で作る。
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


def latent(rng: SplitMix64, n: int, m: int, sigma: int) -> list[int]:
    """問題ごとの難しさ x (0 から 10^6) と人ごとの閾値で、各問題の hard の人の集合を返す。"""
    th = [rng.next() % 1000000 for _ in range(m)]
    out = []
    for _ in range(n):
        x = rng.next() % 1000000
        mask = 0
        for j in range(m):
            if x + rng.next() % (sigma + 1) >= th[j]:
                mask |= 1 << j
        out.append(mask)
    return out


def main() -> None:
    seed = int(sys.argv[1])
    rng = SplitMix64(seed * 1000003 + 5)
    if seed == 0:
        print("3 1\nEHE")
        return
    if seed == 1:
        print("10 6\nEHEEEHHEEH\nEHHHEEHHHE\nEHEHEHEEHH\nHEHEEEHEEE\nHHEEHEEEHE\nEHHEEEEEHE")
        return
    if seed >= 1000:
        m = 1 + rng.next() % 6
        n = 1 + rng.next() % 8
        masks = [rng.next() % (1 << m) for _ in range(n)]
    elif seed == 2:
        n, m = 100000, 20
        masks = [rng.next() % (1 << m) for _ in range(n)]
    elif seed == 3:
        n, m = 100000, 20
        masks = latent(rng, n, m, 30000)
    elif seed == 4:
        n, m = 100000, 20
        pool = latent(rng, 40, m, 100000)
        masks = [pool[rng.next() % 40] for _ in range(n)]
    elif seed == 5:
        n, m = 100000, 16
        masks = [rng.next() % (1 << m) for _ in range(n)]
    elif seed == 6:
        n, m = 100000, 20
        masks = latent(rng, n, m, 300000)
    elif seed == 7:
        n, m = 1, 20
        masks = [rng.next() % (1 << m)]
    elif seed == 8:
        n, m = 100000, 1
        masks = [rng.next() % 2 for _ in range(n)]
    elif seed == 9:
        n, m = 100000, 20
        masks = [0] * n
    elif seed == 10:
        n, m = 100000, 20
        masks = [(1 << m) - 1] * n
    else:
        n, m = 50000, 20
        masks = latent(rng, n // 2, m, 20000) + [rng.next() % (1 << m) for _ in range(n - n // 2)]
    print(n, m)
    for j in range(m):
        print("".join("H" if mk >> j & 1 else "E" for mk in masks))


if __name__ == "__main__":
    main()
