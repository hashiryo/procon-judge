#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""TUPC 2022 K (Lebesgue Integral) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

N K、そのあとに A_1..A_N を出す。seed 0 から 13 は本番のケース。小さい例、n = 1、k = 1、k = n、
値が全部同じ、制約いっぱいのランダム、遅くなりやすい形 (増える、減る、2 種類だけ) の順に並べる。
seed 1000 以上は、愚直解 (brute.cpp、O(k n^2)) で解ける n <= 40 の入力。
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


MAX_N = 10**5
MAX_A = 10**5


def case(seed: int) -> tuple[int, int, list[int]]:
    rng = SplitMix64(seed * 0x9E3779B9 + 0x1234567)
    if seed == 0:  # 問題文の入力例 1
        return 4, 2, [3, 7, 4, 5]
    if seed == 1:  # 問題文の入力例 2
        return 19, 4, [
            3279, 97197, 36049, 32099, 29257, 18290, 96531, 13435, 88697,
            97081, 71483, 11396, 77398, 55303, 4166, 3906, 12281, 28658, 30496,
        ]
    if seed == 2:
        return 1, 1, [1]
    if seed == 3:
        return 1, MAX_N, [MAX_A]
    if seed == 4:
        n, k = 1000, 500
        return n, k, [7] * n
    if seed == 5:
        n, k = MAX_N, 1
        return n, k, [rng.range(1, MAX_A) for _ in range(n)]
    if seed == 6:
        n, k = MAX_N, MAX_N
        return n, k, [rng.range(1, MAX_A) for _ in range(n)]
    if seed == 7:
        n, k = MAX_N, 50000
        return n, k, [rng.range(1, MAX_A) for _ in range(n)]
    if seed == 8:
        n, k = MAX_N, 30000
        return n, k, [(i + 1) * MAX_A // n for i in range(n)]
    if seed == 9:
        n, k = MAX_N, 30000
        return n, k, [MAX_A - (i * (MAX_A - 1)) // n for i in range(n)]
    if seed == 10:
        n, k = MAX_N, 77777
        return n, k, [MAX_A] * n
    if seed == 11:
        n, k = MAX_N, 33333
        vals = [1, 2, 500, MAX_A]
        return n, k, [vals[rng.range(0, 3)] for _ in range(n)]
    if seed == 12:
        n, k = 1000, 1000
        return n, k, [rng.range(1, MAX_A) for _ in range(n)]
    if seed == 13:
        n, k = MAX_N, MAX_N - 1
        return n, k, [rng.range(1, MAX_A) for _ in range(n)]
    # seed 1000 以上: 愚直解で解ける小さい入力
    n = rng.range(1, 40)
    k = rng.range(1, 40)
    mode = rng.range(0, 2)
    if mode == 0:
        a = [rng.range(1, 10) for _ in range(n)]
    elif mode == 1:
        a = [rng.range(1, MAX_A) for _ in range(n)]
    else:
        a = [rng.range(1, 3) for _ in range(n)]
    return n, k, a


def main() -> None:
    seed = int(sys.argv[1])
    n, k, a = case(seed)
    assert len(a) == n and 1 <= n <= MAX_N and 1 <= k <= MAX_N
    assert all(1 <= x <= MAX_A for x in a)
    out = [f"{n} {k}", " ".join(map(str, a))]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
