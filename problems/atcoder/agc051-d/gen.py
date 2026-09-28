"""agc051-d (C4) の入力を作る。a b c d を 1 行で出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、大きい値のランダム。
4 つの偶奇がそろわないと答えは 0 (例 2)。そろっていれば、提出は周回数 k (-min から min まで 2 おき) ごとに
BEST 定理で数えるので、min(a, b, c, d) が大きいほど遅い。角のケースは、全部 1 (答えは 2)、
全部 5 × 10^5 と全部 499999、1 つだけ偶奇が違う (答えは 0)、min が 1 か 2 で他が大きい
(ある辺を片方向にしか通らない周回数が出る)、1 辺だけ大きい、など。
seed が 1000 以上なら、今いる頂点と各辺の残りの回数を状態にしてメモ化で数える愚直解で解ける
各値 12 以下の入力を出す (pj testdata crosscheck 用)。2000 以上なら、愚直解でまだ解ける各値 36 以下の入力を出す。
"""

import random
import sys

MAX = 5 * 10**5

SAMPLES = [
    (2, 2, 2, 2),
    (1, 2, 3, 4),
    (470000, 480000, 490000, 500000),
]
FIXED = [
    (1, 1, 1, 1),  # S T U V S とその逆
    (1, 1, 1, 3),
    (2, 4, 6, 8),
    (3, 5, 7, 9),
    (MAX, MAX, MAX, MAX),
    (MAX - 1, MAX - 1, MAX - 1, MAX - 1),
    (MAX, MAX - 1, MAX, MAX),  # 1 つだけ偶奇が違う
    (1, MAX, MAX, MAX),
    (1, MAX - 1, MAX - 1, MAX - 1),  # min = 1 で k = ±1 だけ
    (2, MAX, MAX, MAX),
    (MAX, 2, MAX, 2),
    (MAX, MAX, 2, 2),
    (2, 2, 2, MAX),  # 1 辺だけ大きい
    (1, 1, 1, MAX - 1),
    (MAX - 2, MAX, MAX - 2, MAX),
    (MAX, MAX, MAX, MAX - 2),
]
RANDOM_EVEN = 3  # 全部偶数の大きい値
RANDOM_ODD = 3  # 全部奇数の大きい値
RANDOM_ANY = 2  # 偶奇もランダム (たいてい 0)
COUNT = len(SAMPLES) + len(FIXED) + RANDOM_EVEN + RANDOM_ODD + RANDOM_ANY


def with_parity(rng: random.Random, lo: int, hi: int, parity: int) -> int:
    """lo 以上 hi 以下で偶奇が parity の値。"""
    x = rng.randint(lo, hi)
    if x % 2 != parity:
        x = x + 1 if x < hi else x - 1
    return x


def small_case(rng: random.Random, limit: int) -> tuple[int, int, int, int]:
    """各値 limit 以下。7 割は偶奇をそろえ、値の片寄りも混ぜる。"""
    if rng.random() < 0.3:
        return tuple(rng.randint(1, limit) for _ in range(4))
    parity = rng.randrange(2)
    hi = rng.choice([limit, rng.randint(2, limit)])
    return tuple(with_parity(rng, 1, hi, parity) if rng.random() < 0.8 else with_parity(rng, 1, 3, parity) for _ in range(4))


def case_for(seed: int, rng: random.Random) -> tuple[int, int, int, int]:
    if seed >= 1000:
        return small_case(rng, 36 if seed >= 2000 else 12)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    seed -= len(FIXED)
    if seed < RANDOM_EVEN + RANDOM_ODD:
        parity = 0 if seed < RANDOM_EVEN else 1
        return tuple(with_parity(rng, MAX // 2, MAX, parity) for _ in range(4))
    return tuple(rng.randint(1, MAX) for _ in range(4))


def main() -> None:
    seed = int(sys.argv[1])
    values = case_for(seed, random.Random(seed))
    assert len(values) == 4 and all(1 <= v <= MAX for v in values)
    print(*values)


if __name__ == "__main__":
    main()
