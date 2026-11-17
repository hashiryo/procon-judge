#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""k 進の桁ごとのゼータ変換とメビウス変換の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

1 行目に t n、2 行目に a_0 ... a_{k^n - 1}。t と k は base.cpp の説明のとおり。値は、t = 0 が 10^9 未満、t = 1 が int の全域、
t = 2 が 998244353 未満、t = 3 が 2^63 未満、t = 4 が絶対値 10^18 以下。
seed 0 から 4 は t = seed で大きい n (10^6、3^13、4^10、5^8、7^7)、5 から 11 は小さい n。
乱数は Python の random の版による違いを避けるため splitmix64 で作る。
"""
import sys

MASK64 = (1 << 64) - 1
K = [10, 3, 4, 5, 7]


class SplitMix64:
    def __init__(self, seed: int) -> None:
        self.x = seed & MASK64

    def next(self) -> int:
        self.x = (self.x + 0x9E3779B97F4A7C15) & MASK64
        z = self.x
        z = ((z ^ (z >> 30)) * 0xBF58476D1CE4E5B9) & MASK64
        z = ((z ^ (z >> 27)) * 0x94D049BB133111EB) & MASK64
        return z ^ (z >> 31)

    def below(self, n: int) -> int:
        return (self.next() * n) >> 64


# seed -> (t, n)
CASES = {
    0: (0, 6),
    1: (1, 13),
    2: (2, 10),
    3: (3, 8),
    4: (4, 7),
    5: (0, 2),
    6: (1, 0),
    7: (2, 3),
    8: (3, 1),
    9: (4, 2),
    10: (1, 4),
    11: (0, 3),
}


def value(r: SplitMix64, t: int) -> int:
    if t == 0:
        return r.below(10**9)
    if t == 1:
        return r.below(2**32) - 2**31
    if t == 2:
        return r.below(998244353)
    if t == 3:
        return r.below(2**63)
    return r.below(2 * 10**18 + 1) - 10**18


def main() -> None:
    seed = int(sys.argv[1])
    t, n = CASES[seed]
    r = SplitMix64(seed * 1000003 + 20261117)
    a = [value(r, t) for _ in range(K[t] ** n)]
    sys.stdout.write(f"{t} {n}\n" + " ".join(map(str, a)) + "\n")


if __name__ == "__main__":
    main()
