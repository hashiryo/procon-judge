#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""法 998244353 固定の nCr の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

1 行目が N Q。続く Q 行が n r (0 <= r <= n <= N < 998244353)。
表の構築を測れるよう、n = N の問いを先頭に置く。

seed 1000 以上は愚直解と突き合わせる小さい入力 (N <= 200)。
乱数は Python の random の版による違いを避けるため splitmix64 で作る。
"""
import sys

MASK64 = (1 << 64) - 1
CASES = {0: (10**7, 5), 1: (10**7, 10**6), 2: (1000, 1000), 3: (10**6, 10**6), 4: (5 * 10**6, 5 * 10**5)}


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
        # 愚直解と突き合わせる小さい入力。
        rng = SplitMix64(seed * 1_000_003 + 42)
        N = 1 + rng.below(200)
        q = 1 + rng.below(50)
        out = [f"{N} {q}"]
        out.append(f"{N} {rng.below(N + 1)}")
        for _ in range(q - 1):
            n = rng.below(N + 1)
            out.append(f"{n} {rng.below(n + 1)}")
        print("\n".join(out))
        return
    N, q = CASES[seed]
    rng = SplitMix64(seed * 1_000_003 + 42)
    out = [f"{N} {q}", f"{N} {rng.below(N + 1)}"]
    for i in range(q - 1):
        # 1 割は r を端 (0 か n) に寄せる。
        n = rng.below(N + 1)
        t = rng.below(10)
        r = 0 if t == 0 else n if t == 1 else rng.below(n + 1)
        out.append(f"{n} {r}")
    print("\n".join(out))


if __name__ == "__main__":
    main()
