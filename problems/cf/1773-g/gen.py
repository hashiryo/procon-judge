#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""Codeforces 1773G (Game of Questions) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

1 行目に n と m、続く n 行に長さ m の 0 と 1 の列 (1 が正解)。seed 0 から 2 は問題文の例。seed 3 から 11 は、n = 2e5 で m = 17 の
正解の確率 1/2 (Genie が必ず正解のものと、そうでないもの)、正解の確率 0.9、0.97、質問の種類が 40 だけ (n = 2e5)、
m = 2、m = 14 で確率 0.8、n = 1 で m = 17、質問ごとの難しさと人ごとの強さで正解を決める入れ子に近いもの。
seed 1000 以上は m = 2 から 5、n <= 7 の小さい入力 (愚直解 brute.hpp 用)。
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


def biased(rng: SplitMix64, n: int, m: int, per_mille: int, genie_ok: bool) -> list[int]:
    out = []
    for _ in range(n):
        mask = 0
        for j in range(m):
            if rng.next() % 1000 < per_mille:
                mask |= 1 << j
        if genie_ok:
            mask |= 1
        out.append(mask)
    return out


def main() -> None:
    seed = int(sys.argv[1])
    rng = SplitMix64(seed * 1000003 + 53)
    if seed == 0:
        print("1 5\n11010")
        return
    if seed == 1:
        print("3 3\n011\n101\n110")
        return
    if seed == 2:
        print("6 4\n1011\n0110\n1111\n0110\n0000\n1101")
        return
    if seed >= 1000:
        m = 2 + rng.next() % 4
        n = 1 + rng.next() % 7
        masks = biased(rng, n, m, 300 + rng.next() % 500, rng.next() % 2 == 0)
    elif seed == 3:
        n, m = 200000, 17
        masks = biased(rng, n, m, 500, True)
    elif seed == 4:
        n, m = 200000, 17
        masks = biased(rng, n, m, 500, False)
    elif seed == 5:
        n, m = 200000, 17
        masks = biased(rng, n, m, 900, False)
    elif seed == 6:
        n, m = 200000, 17
        masks = biased(rng, n, m, 970, False)
    elif seed == 7:
        n, m = 200000, 17
        pool = biased(rng, 40, m, 700, False)
        masks = [pool[rng.next() % 40] for _ in range(n)]
    elif seed == 8:
        n, m = 200000, 2
        masks = biased(rng, n, m, 500, False)
    elif seed == 9:
        n, m = 200000, 14
        masks = biased(rng, n, m, 800, False)
    elif seed == 10:
        n, m = 1, 17
        masks = biased(rng, n, m, 500, True)
    else:
        n, m = 200000, 17
        power = [rng.next() % 1000000 for _ in range(m)]
        masks = []
        for _ in range(n):
            d = rng.next() % 1000000
            mask = 0
            for j in range(m):
                if power[j] + rng.next() % 300000 >= d:
                    mask |= 1 << j
            masks.append(mask)
    print(n, m)
    print("\n".join("".join("1" if mk >> j & 1 else "0" for j in range(m)) for mk in masks))


if __name__ == "__main__":
    main()
