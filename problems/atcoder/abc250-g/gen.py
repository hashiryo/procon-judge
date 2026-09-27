"""abc250-g (Stonks) の入力を作る。N と P_1 ... P_N を出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、N = 2 × 10^5 の列。
列は、ランダム、増える列 (前半で買って後半で売る)、減る列 (答えは 0)、1 と 10^9 の交互
(答えが 10^14 近くになる)、小さい値だけ (同じ値だらけ)、ランダムウォーク、谷と山の形、
10^9 の近くに固まったもの。
seed が 1000 以上なら、持っている株の数を状態にする O(N^2) の DP で解ける小さい入力を出す
(pj testdata crosscheck 用)。
"""

import random
import sys

MAX_N = 2 * 10**5
MAX_P = 10**9

SAMPLES = [
    [2, 5, 4, 3, 7, 1, 8, 6],
    [10000, 1000, 100, 10, 1],
    [300, 1, 4000, 1, 50000, 900000000, 20, 600000, 50000, 300, 50000, 80000000, 900000000, 7000000, 900000000],
]
FIXED = [
    [1],
    [MAX_P],
    [1, MAX_P],
    [MAX_P, 1],
    [5, 5, 5, 5],
    [1, 2, 3],  # 1 日に 1 つしか売り買いできないので、答えは 2
]
# (列の出し方, N)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    ("random", MAX_N),
    ("increasing", MAX_N),
    ("decreasing", MAX_N),
    ("alternate", MAX_N),
    ("small", MAX_N),
    ("walk", MAX_N),
    ("valley", MAX_N),
    ("mountain", MAX_N),
    ("near_max", MAX_N),
    ("random", 1000),
    ("small", 1000),
]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)


def prices(rng: random.Random, how: str, n: int) -> list[int]:
    if how == "random":
        return [rng.randint(1, MAX_P) for _ in range(n)]
    if how == "increasing":
        return sorted(rng.randint(1, MAX_P) for _ in range(n))
    if how == "decreasing":
        return sorted((rng.randint(1, MAX_P) for _ in range(n)), reverse=True)
    if how == "alternate":
        return [1 if i % 2 == 0 else MAX_P for i in range(n)]
    if how == "small":
        return [rng.randint(1, 5) for _ in range(n)]
    if how == "walk":
        out, p = [], MAX_P // 2
        for _ in range(n):
            p = max(1, min(MAX_P, p + rng.randint(-1000, 1000)))
            out.append(p)
        return out
    if how in ("valley", "mountain"):
        half = sorted(rng.randint(1, MAX_P) for _ in range(n))
        left, right = half[0::2], half[1::2]
        if how == "valley":
            return left[::-1] + right
        return left + right[::-1]
    assert how == "near_max"
    return [MAX_P - rng.randint(0, 1000) for _ in range(n)]


def small_case(rng: random.Random) -> list[int]:
    """愚直解の DP が解ける大きさ。値の出し方は本番と同じものから選ぶ。"""
    n = rng.randint(1, 12) if rng.random() < 0.3 else rng.randint(1, 300)
    how = rng.choice([
        "random", "increasing", "decreasing", "alternate", "small", "walk", "valley", "mountain", "near_max",
    ])
    return prices(rng, how, n)


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 1000:
        p = small_case(rng)
    elif seed < len(SAMPLES):
        p = SAMPLES[seed]
    elif seed < len(SAMPLES) + len(FIXED):
        p = FIXED[seed - len(SAMPLES)]
    else:
        how, n = PLANS[seed - len(SAMPLES) - len(FIXED)]
        p = prices(rng, how, n)
    assert 1 <= len(p) <= MAX_N and all(1 <= v <= MAX_P for v in p)
    sys.stdout.write(f"{len(p)}\n{' '.join(map(str, p))}\n")


if __name__ == "__main__":
    main()
