#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""法 998244353 固定のテトレーションの入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

1 行目に n。続く n 行が a b (a ↑↑ b mod 998244353 を答える)。

seed 1000 以上は愚直解と突き合わせる小さい入力 (a <= 10、b <= 3 と、2 の塔だけ b <= 5)。
乱数は Python の random の版による違いを避けるため splitmix64 で作る。
"""
import sys

MASK64 = (1 << 64) - 1
MOD = 998244353
CASES = {0: 47, 1: 1000, 2: 50000, 3: 200000, 4: 500000}


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


def main() -> None:
    seed = int(sys.argv[1])
    if seed >= 1000:
        # 愚直解と突き合わせる小さい入力。a <= 10、b <= 3。2 の塔だけ b を 5 まで伸ばす。
        out = []
        for a in range(11):
            for b in range(4):
                out.append(f"{a} {b}")
        out += ["2 4", "2 5", "1 1000000000", "0 2", "0 3", "998244353 1", "998244354 1"]
        print(len(out))
        print("\n".join(out))
        return
    rng = SplitMix64(seed * 1_000_003 + 42)
    n = CASES[seed]
    out = [str(n)]
    if seed == 0:
        # 端と小さい値を先に並べる。
        edge = []
        for a in (0, 1, 2, 3, 4, 5, MOD - 1, MOD, MOD + 1, MOD * 2, 10**9, 10**9 - 1, 998244352):
            for b in (0, 1, 2, 3, 4, 5):
                edge.append(f"{a} {b}")
        out += edge[:47]
        n = len(out) - 1
        out[0] = str(n)
        print("\n".join(out))
        return
    for _ in range(n):
        a = rng.below(10**9 + 1)
        # b は小さい値 (0..5) を 1 割ほど混ぜる。残りは 6..10^9。
        b = rng.below(6) if rng.below(10) == 0 else 6 + rng.below(10**9 - 5)
        out.append(f"{a} {b}")
    print("\n".join(out))


if __name__ == "__main__":
    main()
