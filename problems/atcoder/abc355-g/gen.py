#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""ABC 355 G (Baseball) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

N K、そのあとに P_1..P_N を出す。seed 0 から 13 は本番のケース。問題文の 4 例、ΣP を 1e5 に
使い切る形 (一様、全部同じ、中央に集中、交互)、K が 1・N・N-1 の角、遅くなりやすい形の順に並べる。
seed 1000 以上は、愚直解 (brute.cpp、O(k M^2)、M = ΣP) で解ける M <= 60 の入力。
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


MAX_N = 5 * 10**4
MAX_P = 10**5
MAX_SUM = 10**5


def spread(rng: SplitMix64, n: int, budget: int) -> list[int]:
    """n 軒に budget を配る (各 0..、合計 budget)。"""
    p = [0] * n
    for _ in range(budget):
        p[rng.range(0, n - 1)] += 1
    return p


def case(seed: int) -> tuple[int, int, list[int]]:
    rng = SplitMix64(seed * 0x9E3779B9 + 0x5A5A5)
    if seed == 0:  # 問題文の入力例 1
        return 5, 2, [1, 1, 1, 1, 1]
    if seed == 1:  # 入力例 2
        return 5, 1, [0, 0, 1, 0, 0]
    if seed == 2:  # 入力例 3
        return 1, 1, [100]
    if seed == 3:  # 入力例 4
        return 20, 7, [
            4262, 9522, 2426, 3823, 7364, 964, 2743, 2423, 1955, 5274,
            3684, 847, 363, 35, 278, 3220, 203, 2904, 6304, 1928,
        ]
    if seed == 4:
        n, k = MAX_N, 1
        return n, k, spread(rng, n, MAX_SUM)
    if seed == 5:
        n, k = MAX_N, MAX_N
        return n, k, [2] * n
    if seed == 6:
        n, k = MAX_N, MAX_N // 2
        return n, k, spread(rng, n, MAX_SUM)
    if seed == 7:
        n, k = MAX_N, MAX_N // 4
        return n, k, [2] * n
    if seed == 8:
        n, k = MAX_N, 2
        return n, k, spread(rng, n, MAX_SUM)
    if seed == 9:
        n, k = MAX_N, MAX_N // 5
        p = [0] * n
        p[n // 2] = MAX_SUM
        return n, k, p
    if seed == 10:
        n, k = 1000, 500
        return n, k, spread(rng, n, 1000)
    if seed == 11:
        n, k = MAX_N, MAX_N // 2
        return n, k, [4 if i % 2 == 0 else 0 for i in range(n)]
    if seed == 12:
        n, k = MAX_N, MAX_N - 1
        p = [0] * n
        for i in range(0, n, n // 50):
            p[i] = 2000
        return n, k, p
    if seed == 13:
        n, k = MAX_N, 30000
        p = [0] * n
        p[0], p[n // 3], p[n - 1] = 40000, 30000, 30000
        return n, k, p
    # seed 1000 以上: 愚直解で解ける小さい入力 (M = ΣP <= 60)
    n = rng.range(1, 30)
    k = rng.range(1, n)
    budget = rng.range(1, 60)
    if rng.range(0, 1) == 0:
        p = spread(rng, n, budget)
    else:
        p = [budget if i == rng.range(0, n - 1) else 0 for i in range(n)]
        if sum(p) == 0:
            p[0] = 1
    return n, k, p


def main() -> None:
    seed = int(sys.argv[1])
    n, k, p = case(seed)
    assert len(p) == n and 1 <= n <= MAX_N and 1 <= k <= n
    assert all(0 <= x <= MAX_P for x in p)
    assert 1 <= sum(p) <= MAX_SUM
    out = [f"{n} {k}", " ".join(map(str, p))]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
