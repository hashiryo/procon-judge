"""abc179-c (A x B + C) の入力を作る。N を 1 行で出す。答えは A B < N となる (A, B) の数。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、ランダム (10^6 に近いものと、平方数の近くのもの)。
角のケースは、N - 1 が 1 や素数や約数の多い数のときと、平方数の近く (商を列挙する境目の
sqrt(N - 1) の前後) のもの。
seed が 1000 以上なら、(A, B) を全部数える愚直解で解ける入力を出す (pj testdata crosscheck 用)。
愚直解は N = 10^6 でも速いので、大きさは小さいものから制約いっぱいまで混ぜる。
"""

import random
import sys

MAX = 10**6

FIXED = [
    3,  # 例 1
    100,  # 例 2
    MAX,  # 例 3
    2,  # N - 1 = 1。答えは 1
    4,
    5,  # N - 1 = 4 は平方数
    7,
    MAX - 1,
    999984,  # N - 1 = 999983 は 10^6 未満で最大の素数
    720721,  # N - 1 = 720720 は約数が多い
    524289,  # N - 1 = 2^19
    998002,  # N - 1 = 999^2
    998001,  # N - 1 = 999^2 - 1
    999001,  # N - 1 = 999 * 1000
    999000,  # N - 1 = 999 * 1000 - 1
    1000,
]
RANDOM = 6


def near_square(rng: random.Random, lo: int, hi: int) -> int:
    """k^2 や k(k+1) の近くの N - 1 を選んで N を返す。"""
    while True:
        k = rng.randint(1, 1000)
        m = rng.choice([k * k, k * (k + 1)]) + rng.randint(-1, 1)
        if lo <= m + 1 <= hi:
            return m + 1


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 1000:
        kind = rng.randrange(4)
        if kind == 0:
            n = rng.randint(2, 50)
        elif kind == 1:
            n = rng.randint(2, 10**4)
        elif kind == 2:
            n = near_square(rng, 2, MAX)
        else:
            n = rng.randint(2, MAX)
    elif seed < len(FIXED):
        n = FIXED[seed]
    else:
        n = rng.randint(MAX - 10**5, MAX) if seed % 2 else near_square(rng, MAX // 2, MAX)
    assert 2 <= n <= MAX
    print(n)


if __name__ == "__main__":
    main()
