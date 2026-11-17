#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""半環の部分集合畳み込みと exp の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

1 行目に t n、2 行目に f_0 ... f_{2^n - 1}、t が 0、1、4 なら 3 行目に g_0 ... g_{2^n - 1}。t は base.cpp の説明のとおり。
値は、t = 0 が 0 以上 10^8 以下、t = 2 が 0 以上 5 × 10^7 以下 (塊が 18 個でも int の INF に届かない)、t = 1 と 3 が絶対値 10^12
以下で、どれも 16 個に 1 個を「無い」にする。t = 4 は int の全域。exp の f[0] は 0。
seed 0 から 4 は t = seed で、畳み込みは n = 16、exp は n = 18。5 から 11 は小さい n、12 と 13 は n = 17。
乱数は Python の random の版による違いを避けるため splitmix64 で作る。
"""
import sys

MASK64 = (1 << 64) - 1
NONE_MAX = -(10**18)


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
    0: (0, 16),
    1: (1, 16),
    2: (2, 18),
    3: (3, 18),
    4: (4, 16),
    5: (0, 3),
    6: (1, 2),
    7: (2, 4),
    8: (3, 1),
    9: (4, 0),
    10: (2, 0),
    11: (3, 5),
    12: (0, 17),
    13: (3, 17),
}


def value(r: SplitMix64, t: int) -> int:
    if t == 4:
        return r.below(2**32) - 2**31
    if r.below(16) == 0:
        return -1 if t in (0, 2) else NONE_MAX
    if t == 0:
        return r.below(10**8 + 1)
    if t == 2:
        return r.below(5 * 10**7 + 1)
    return r.below(2 * 10**12 + 1) - 10**12


def main() -> None:
    seed = int(sys.argv[1])
    t, n = CASES[seed]
    r = SplitMix64(seed * 1000003 + 20261117)
    f = [value(r, t) for _ in range(1 << n)]
    if t in (2, 3):
        f[0] = 0
    out = [f"{t} {n}", " ".join(map(str, f))]
    if t in (0, 1, 4):
        out.append(" ".join(str(value(r, t)) for _ in range(1 << n)))
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
