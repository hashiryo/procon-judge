#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""arc136-d (Without Carry) の入力を作る。N と A_1 ... A_N を出す (2 <= N <= 10^6、0 <= A_i <= 10^6 - 1)。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、N = 2、N = 10^6 のいろいろな A (ランダム、全部 0、全部 999999、
どの桁も 4 以下、100 未満、どの桁も 0 か 9)、N = 999999 のランダム、N = 10^6 で 2 種類の値だけ。
seed が 1000 以上なら、組を全部見る愚直解 (brute.cpp) で解ける N = 2 から 300 の入力を出す (pj testdata crosscheck 用)。
乱数は Python の random の版による違いを避けるため splitmix64 で作る。
"""
import sys

MASK64 = (1 << 64) - 1
M = 10**6
MAX_N = 10**6


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
    [4, 8, 12, 90],
    [313923, 246114, 271842, 371982, 284858, 10674, 532090, 593483, 185123, 364245,
     665161, 241644, 604914, 645577, 410849, 387586, 732231, 952593, 249651, 36908],
    [1, 1, 1, 1, 1],
]
# seed 3 から 11 の (N, A の形)
PLANS = {
    3: (2, "random"),
    4: (MAX_N, "random"),
    5: (MAX_N, "zero"),
    6: (MAX_N, "nines"),
    7: (MAX_N, "low_digits"),
    8: (MAX_N, "small"),
    9: (MAX_N, "zero_nine"),
    10: (MAX_N - 1, "random"),
    11: (MAX_N, "two_values"),
}


def digits(r: SplitMix64, hi: int) -> int:
    v = 0
    for _ in range(6):
        v = v * 10 + r.below(hi + 1)
    return v


def values(r: SplitMix64, n: int, how: str) -> list[int]:
    if how == "zero":
        return [0] * n
    if how == "nines":
        return [M - 1] * n
    if how == "low_digits":
        return [digits(r, 4) for _ in range(n)]
    if how == "small":
        return [r.below(100) for _ in range(n)]
    if how == "zero_nine":
        return [int("".join("9" if r.below(2) else "0" for _ in range(6))) for _ in range(n)]
    if how == "two_values":
        a, b = r.below(M), r.below(M)
        return [a if r.below(2) else b for _ in range(n)]
    return [r.below(M) for _ in range(n)]


def main() -> None:
    seed = int(sys.argv[1])
    r = SplitMix64(seed * 1000003 + 136)
    if seed < len(SAMPLES):
        a = SAMPLES[seed]
    elif seed >= 1000:
        n = 2 + r.below(299)
        how = ["random", "low_digits", "small", "zero_nine", "two_values"][r.below(5)]
        a = values(r, n, how)
    else:
        n, how = PLANS[seed]
        a = values(r, n, how)
    sys.stdout.write(f"{len(a)}\n" + " ".join(map(str, a)) + "\n")


if __name__ == "__main__":
    main()
