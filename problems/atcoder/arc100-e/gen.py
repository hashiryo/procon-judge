#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""arc100-e (Or Plus Max) の入力を作る。N と A_0 ... A_{2^N - 1} を出す (1 <= N <= 18、1 <= A_i <= 10^9)。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、N = 1、N = 18 のいろいろな A (ランダム、全部 10^9、増える列、
減る列、1 から 10 の値で同じ値が多いもの、最大が添字 0 で 2 番目が最後の添字)、N = 17 と N = 2 のランダム。
seed が 1000 以上なら、愚直解 (brute.cpp、4^N) で解ける N = 1 から 8 の入力を出す (pj testdata crosscheck 用)。
乱数は Python の random の版による違いを避けるため splitmix64 で作る。
"""
import sys

MASK64 = (1 << 64) - 1
MAX_A = 10**9


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


SAMPLES = [
    (2, [1, 2, 3, 1]),
    (3, [10, 71, 84, 33, 6, 47, 23, 25]),
    (4, [75, 26, 45, 72, 81, 47, 97, 97, 2, 2, 25, 82, 84, 17, 56, 32]),
]
# seed 3 から 11 の (N, A の形)
PLANS = {
    3: (1, "two"),
    4: (18, "random"),
    5: (18, "all_max"),
    6: (18, "increasing"),
    7: (18, "decreasing"),
    8: (18, "small"),
    9: (18, "ends"),
    10: (17, "random"),
    11: (2, "random"),
}


def values(r: SplitMix64, n: int, how: str) -> list[int]:
    m = 1 << n
    if how == "two":
        return [1, MAX_A]
    if how == "all_max":
        return [MAX_A] * m
    if how == "increasing":
        return [i + 1 for i in range(m)]
    if how == "decreasing":
        return [m - i for i in range(m)]
    if how == "small":
        return [1 + r.below(10) for _ in range(m)]
    a = [1 + r.below(MAX_A) for _ in range(m)]
    if how == "ends":
        a[0], a[m - 1] = MAX_A, MAX_A - 1
    return a


def main() -> None:
    seed = int(sys.argv[1])
    r = SplitMix64(seed * 1000003 + 100)
    if seed < len(SAMPLES):
        n, a = SAMPLES[seed]
    elif seed >= 1000:
        n = 1 + r.below(8)
        top = [10, 100, MAX_A][r.below(3)]
        a = [1 + r.below(top) for _ in range(1 << n)]
    else:
        n, how = PLANS[seed]
        a = values(r, n, how)
    sys.stdout.write(f"{n}\n" + " ".join(map(str, a)) + "\n")


if __name__ == "__main__":
    main()
