#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""ゼータ変換・メビウス変換・アダマール変換の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

1 行目に t n、2 行目に a_0 ... a_{2^n - 1}。t は base.cpp の説明のとおりで、値の範囲は t で決まる。
t = 0 は int の全域 (端の値も混ぜる)、t = 1 は絶対値 10^18 以下、t = 2 は 2^63 未満、t = 3 から 5 は 998244353 未満。
seed 0 から 5 は n = 20 で t = seed、6 から 11 は小さい n、12 と 13 は n = 21 と 22 (t = 0 と 3)。
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

    def below(self, n: int) -> int:
        return (self.next() * n) >> 64


# seed -> (t, n)
CASES = {
    0: (0, 20),
    1: (1, 20),
    2: (2, 20),
    3: (3, 20),
    4: (4, 20),
    5: (5, 20),
    6: (0, 0),
    7: (1, 1),
    8: (2, 2),
    9: (3, 3),
    10: (4, 4),
    11: (5, 5),
    12: (0, 21),
    13: (3, 22),
}


def value(r: SplitMix64, t: int) -> int:
    if t == 0:
        k = r.below(16)
        if k == 0:
            return 2**31 - 1
        if k == 1:
            return -(2**31)
        return r.below(2**32) - 2**31
    if t == 1:
        return r.below(2 * 10**18 + 1) - 10**18
    if t == 2:
        return r.below(2**63)
    return r.below(998244353)


def main() -> None:
    seed = int(sys.argv[1])
    t, n = CASES[seed]
    r = SplitMix64(seed * 1000003 + 20261117)
    a = [value(r, t) for _ in range(1 << n)]
    sys.stdout.write(f"{t} {n}\n" + " ".join(map(str, a)) + "\n")


if __name__ == "__main__":
    main()
