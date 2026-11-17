#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""dp-u (Grouping) の入力を作る。N と、対称で対角が 0 の N × N の行列 a を出す (1 <= N <= 16、|a_{i,j}| <= 10^9)。

seed が 0 から count - 1 までは本番のケース。例の 4 つ、N = 1、N = 16 のいろいろな a (ランダム、全部 10^9 で 1 つの塊が最良、
全部 -10^9 で 1 羽ずつが最良、-10 から 10 の小さい値、負が多いランダム)、N = 15 と N = 8 のランダム。
seed が 1000 以上なら、集合の分割を全部見る愚直解 (brute.cpp) で解ける N = 1 から 8 の入力を出す (pj testdata crosscheck 用)。
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
    [[0, 10, 20], [10, 0, -100], [20, -100, 0]],
    [[0, -10], [-10, 0]],
    [[0, MAX_A, MAX_A, MAX_A], [MAX_A, 0, MAX_A, MAX_A], [MAX_A, MAX_A, 0, -1], [MAX_A, MAX_A, -1, 0]],
    None,  # 例 4 は下の SAMPLE4 で与える
]
SAMPLE4 = """0 5 -4 -5 -8 -4 7 2 -4 0 7 0 2 -3 7 7
5 0 8 -9 3 5 2 -7 2 -7 0 -1 -4 1 -1 9
-4 8 0 -9 8 9 3 1 4 9 6 6 -6 1 8 9
-5 -9 -9 0 -7 6 4 -1 9 -3 -5 0 1 2 -4 1
-8 3 8 -7 0 -5 -9 9 1 -9 -6 -3 -8 3 4 3
-4 5 9 6 -5 0 -6 1 -2 2 0 -5 -2 3 1 2
7 2 3 4 -9 -6 0 -2 -2 -9 -3 9 -2 9 2 -5
2 -7 1 -1 9 1 -2 0 -6 0 -6 6 4 -1 -7 8
-4 2 4 9 1 -2 -2 -6 0 8 -6 -2 -4 8 7 7
0 -7 9 -3 -9 2 -9 0 8 0 0 1 -3 3 -6 -6
7 0 6 -5 -6 0 -3 -6 -6 0 0 5 7 -1 -5 3
0 -1 6 0 -3 -5 9 6 -2 1 5 0 -2 7 -8 0
2 -4 -6 1 -8 -2 -2 4 -4 -3 7 -2 0 -9 7 1
-3 1 1 2 3 3 9 -1 8 3 -1 7 -9 0 -6 -8
7 -1 8 -4 4 1 2 -7 7 -6 -5 -8 7 -6 0 -9
7 9 9 1 3 2 -5 8 7 -6 3 0 1 -8 -9 0"""
# seed 4 から 11 の (N, a の形)
PLANS = {
    4: (1, "random"),
    5: (16, "random"),
    6: (16, "all_max"),
    7: (16, "all_min"),
    8: (16, "small"),
    9: (16, "negative"),
    10: (15, "random"),
    11: (8, "random"),
}


def matrix(r: SplitMix64, n: int, how: str) -> list[list[int]]:
    a = [[0] * n for _ in range(n)]
    for i in range(n):
        for j in range(i + 1, n):
            if how == "all_max":
                v = MAX_A
            elif how == "all_min":
                v = -MAX_A
            elif how == "small":
                v = r.below(21) - 10
            elif how == "negative":
                v = r.below(2 * MAX_A + 1) - MAX_A
                if r.below(4):
                    v = -abs(v)
            else:
                v = r.below(2 * MAX_A + 1) - MAX_A
            a[i][j] = a[j][i] = v
    return a


def main() -> None:
    seed = int(sys.argv[1])
    r = SplitMix64(seed * 1000003 + 21)
    if seed == 3:
        a = [list(map(int, line.split())) for line in SAMPLE4.splitlines()]
    elif seed < 3:
        a = SAMPLES[seed]
    elif seed >= 1000:
        n = 1 + r.below(8)
        a = matrix(r, n, ["random", "small", "negative"][r.below(3)])
    else:
        n, how = PLANS[seed]
        a = matrix(r, n, how)
    sys.stdout.write(f"{len(a)}\n" + "".join(" ".join(map(str, row)) + "\n" for row in a))


if __name__ == "__main__":
    main()
