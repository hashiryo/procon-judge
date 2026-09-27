"""abc172-d (Sum of Divisors) の入力を作る。N を 1 行で出す。答えは K f(K) の K = 1 から N までの和。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、ランダム (10^7 に近いものと、平方数の近くのもの)。
角のケースは、N = 1、N が素数や約数の多い数や 2 のべきのとき、平方数と k(k+1) の前後
(商を列挙する境目の sqrt(N) の前後) のもの。
seed が 1000 以上なら、約数ごとに倍数へ足していく愚直解で解ける入力を出す (pj testdata crosscheck 用)。
N は 10^6 までにする。
"""

import random
import sys

MAX = 10**7
SMALL_MAX = 10**6

FIXED = [
    4,  # 例 1
    100,  # 例 2
    MAX,  # 例 3
    1,
    2,
    3,
    6,
    MAX - 1,
    9999991,  # 10^7 以下で最大の素数
    8648640,  # 10^7 以下で約数が最も多い数の 1 つ (448 個)
    2**23,
    3162**2,  # 10^7 以下で最大の平方数
    3162**2 - 1,
    3161 * 3162,
    3161 * 3162 - 1,
]
RANDOM = 6


def near_square(rng: random.Random, lo: int, hi: int) -> int:
    """k^2 や k(k+1) の近くの N を選ぶ。"""
    while True:
        k = rng.randint(1, 3162)
        n = rng.choice([k * k, k * (k + 1)]) + rng.randint(-1, 1)
        if lo <= n <= hi:
            return n


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 1000:
        kind = rng.randrange(4)
        if kind == 0:
            n = rng.randint(1, 50)
        elif kind == 1:
            n = rng.randint(1, 10**4)
        elif kind == 2:
            n = near_square(rng, 1, SMALL_MAX)
        else:
            n = rng.randint(1, SMALL_MAX)
    elif seed < len(FIXED):
        n = FIXED[seed]
    else:
        n = rng.randint(MAX - 10**6, MAX) if seed % 2 else near_square(rng, MAX // 2, MAX)
    assert 1 <= n <= MAX
    print(n)


if __name__ == "__main__":
    main()
