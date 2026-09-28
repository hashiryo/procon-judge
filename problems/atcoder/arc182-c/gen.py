"""arc182-c (Sum of Number of Divisors of Product) の入力を作る。N M を 1 行で出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、N = 10^18 で M が 2 から 16 のもの、
N の余りが気になる値。提出の M = 1 は別の分岐になる。sps::pow は指数 N + 1 が 6 以下かどうかで計算が
分かれるので、N = 4, 5, 6 を入れる。N + 1 や N が 998244353 の倍数のとき、N や N + 1 が 998244352 の倍数のとき
(M^N の指数を縮める所) も入れる。
seed が 1000 以上なら、長さ N までの列を全部たどって約数の個数を足す愚直解で解ける、sum M^k の小さい
入力を出す (pj testdata crosscheck 用)。
"""

import random
import sys

MAX_N = 10**18
P = 998244353
# 愚直解がたどる列の数 (M + M^2 + ... + M^N) の上限。
BRUTE_LIMIT = 2 * 10**7

SAMPLES = [
    (1, 7),
    (3, 11),
    (81131, 14),
]
FIXED = [
    (1, 1),
    (MAX_N, 1),
    (1, 16),  # 50
    (1, 2),
    (2, 2),  # 11
    (4, 16),
    (5, 16),  # sps::pow の指数 N + 1 = 6
    (6, 16),  # 指数 7
    (P - 1, 16),  # N + 1 が P の倍数
    (P, 16),  # N が P の倍数
    ((MAX_N + 1) // P * P - 1, 13),
    ((P - 1) * 10**9, 16),  # N が P - 1 の倍数
    ((P - 1) * 10**9 - 1, 12),  # N + 1 が P - 1 の倍数
    (2**59, 8),
]
RANDOM = 4


def small_case(rng: random.Random) -> tuple[int, int]:
    m = rng.randint(1, 16)
    total, n = 0, 0
    while True:
        total += m ** (n + 1)
        if total > BRUTE_LIMIT or n + 1 > 1000:  # M = 1 では愚直解の再帰が N 段になる
            break
        n += 1
    return rng.randint(1, max(1, n)) if rng.random() < 0.8 else max(1, n), m


def case_for(seed: int, rng: random.Random) -> tuple[int, int]:
    if seed >= 1000:
        return small_case(rng)
    fixed = SAMPLES + FIXED + [(MAX_N, m) for m in range(2, 17)]
    if seed < len(fixed):
        return fixed[seed]
    assert seed < len(fixed) + RANDOM
    return rng.randint(1, MAX_N), rng.randint(2, 16)


def main() -> None:
    seed = int(sys.argv[1])
    n, m = case_for(seed, random.Random(seed))
    assert 1 <= n <= MAX_N and 1 <= m <= 16
    print(n, m)


if __name__ == "__main__":
    main()
