#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""洛谷 P13844 (【模板】集合幂级数 ln（非素数模数）) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

1 行目に n、2 行目に 2^n 個の値 ([0, 2^64) の整数)。f[∅] = 1。seed 0 から 19 は本番と同じく n = seed + 1 で、値は一様な乱数。seed 20 と 21 は n = 20 で、全部 2^64 - 1 にしたものと、
0 と 1 だけのもの。seed 1000 以上は n = 1 から 6 の小さい入力。
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


def main() -> None:
    seed = int(sys.argv[1])
    rng = SplitMix64(seed * 1000003 + 43)
    if seed >= 1000:
        n = 1 + rng.next() % 6
        mode = 0
    elif seed < 20:
        n, mode = seed + 1, 0
    else:
        n, mode = 20, seed - 19
    N = 1 << n
    if mode == 1:
        a = [MASK64] * N
    elif mode == 2:
        a = [rng.next() & 1 for _ in range(N)]
    else:
        a = [rng.next() for _ in range(N)]
    a[0] = 1
    print(n)
    print(" ".join(map(str, a)))


if __name__ == "__main__":
    main()
