"""abc239-h (Dice Product 2) の入力を作る。N M を 1 行で出す。

seed が 0 から count - 1 までは本番のケース。例の 5 つ、角のケース、M = 10^9 と M もランダムのケース。
角のケースは、N = 2 で M が 2 の累乗の前後 (答えは 2 × (2 の累乗の回数) になる)、M = 1
(答えは N / (N - 1))、N > M、N = M と N = M ± 1、M が平方数の前後 (DirichletSeries の区切りの K, L は
M の平方根と 3 分の 2 乗から決まる)、M = 10^9 で N がいろいろ。
提出の計算量は N によらず M で決まるので、M = 10^9 を 10 ケース入れる。
seed が 1000 以上なら、2 以上の目の並びで積が M 以下のものを長さごとに全部数える愚直解で解ける
M ≤ 3000 の入力を出す (pj testdata crosscheck 用)。2000 以上なら、愚直解でまだ解ける M ≤ 10^5 の入力を出す。
"""

import random
import sys

MAX = 10**9

SAMPLES = [
    (2, 1),
    (2, 39),
    (3, 2),
    (2392, 39239),
    (MAX, MAX),
]
FIXED = [
    (2, MAX),  # 2 が 30 回出るまで。答えは 60
    (2, 2**29),
    (2, 2**29 - 1),
    (3, MAX),
    (MAX, 1),  # 1 以外が出るまで。答えは N / (N - 1)
    (MAX, 2),
    (MAX, 10**5),  # N > M
    (1001, 1000),
    (1000, 1000),
    (999, 1000),
    (MAX - 1, MAX),
    (MAX, MAX - 1),
    (31623, 31622**2),  # M = 31622^2 は 10^9 以下で最大の平方数
    (31622, 31622**2 - 1),
    (10**6, MAX),
    (1000, MAX),
    (31623, MAX),
    (2, 3),
    (3, 3),
]
RANDOM_MAX_M = 3  # M = 10^9 で N がランダム
RANDOM = 5  # N も M もランダム
COUNT = len(SAMPLES) + len(FIXED) + RANDOM_MAX_M + RANDOM


def small_case(rng: random.Random, limit: int) -> tuple[int, int]:
    """M ≤ limit。N は M より小さいものも大きいものも、2 と 3 も混ぜる。"""
    m = rng.choice([rng.randint(1, limit), limit, rng.randint(1, 60), 2 ** rng.randint(0, limit.bit_length() - 1)])
    n = rng.choice([2, 3, rng.randint(2, 10), rng.randint(2, max(2, m)), m + 1, rng.randint(2, MAX)])
    return max(2, n), m


def case_for(seed: int, rng: random.Random) -> tuple[int, int]:
    if seed >= 1000:
        return small_case(rng, 10**5 if seed >= 2000 else 3000)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    seed -= len(FIXED)
    if seed < RANDOM_MAX_M:
        return rng.randint(2, MAX), MAX
    return rng.randint(2, MAX), rng.randint(1, MAX)


def main() -> None:
    seed = int(sys.argv[1])
    n, m = case_for(seed, random.Random(seed))
    assert 2 <= n <= MAX and 1 <= m <= MAX
    print(n, m)


if __name__ == "__main__":
    main()
