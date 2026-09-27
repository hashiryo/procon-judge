"""abc321-g (Electric Circuit) の入力を作る。N M と、赤い端子の部品 R_1 ... R_M、青い端子の部品 B_1 ... B_M を出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、N = 17 で M = 10^5 のもの。
部品の集合 S の中だけでつなげるのは、S の赤と青の数が同じときに限るので、その数が同じになる S の多さを変える。
赤と青を同じ多重集合にしたもの (どの S でも同じ)、ランダム (ほとんどの S で違う)、偏った分布、
1 か所だけずらしたものを入れる。R_i = B_i = i (M = N = 17) は置換の巡回の数なので、答えは調和数 H_17。
seed が 1000 以上なら、M! 通りのつなぎ方を全部試す愚直解で解ける M ≤ 7 の入力を出す (pj testdata crosscheck 用)。
2000 以上なら、愚直解でまだ解ける M = 8, 9 の入力を出す。
"""

import random
import sys

MAX_N = 17
MAX_M = 10**5

Case = tuple[int, list[int], list[int]]

SAMPLES = [
    (3, [1, 2], [3, 2]),
    (17, [1, 1, 1, 1, 1], [1, 1, 1, 1, 1]),
    (8, [2, 4, 7, 1, 7, 6, 1, 4, 8, 1], [5, 1, 5, 2, 5, 8, 4, 6, 1, 3]),
]
# 本番のケースのうち例のあとに並べるもの。
PLANS = [
    "one_part",  # N = 1、M = 1
    "one_part_max",  # N = 1、M = 10^5
    "two_parts",  # N = 2、M = 1 で赤と青が別の部品
    "single_cable",  # N = 17、M = 1 で同じ部品
    "single_cable_apart",  # N = 17、M = 1 で別の部品。答えは 16
    "permutation",  # R_i = B_i = i。答えは H_17
    "far_apart",  # 赤は全部 1、青は全部 17。答えは 16
    "random",
    "balanced",  # 赤と青が同じ多重集合
    "balanced_skewed",  # 同じ多重集合で、部品 k の端子は 2^-k ぐらいの割合
    "half_balanced",  # 部品 1 から 8 は赤と青が同じ数、ほかはランダム
    "off_by_one",  # 同じ多重集合から青を 1 個だけ別の部品へ動かす
    "two_heavy",  # 端子のほとんどが 2 個の部品に集まる
    "cyclic",  # R_i = i mod 17 + 1、B_i = (i + 1) mod 17 + 1
    "random_16",  # N = 16
    "tiny_m",  # N = 17、M = 2
    "random_mid",
    "random_mid",
]
COUNT = len(SAMPLES) + len(PLANS)


def balanced(rng: random.Random, n: int, m: int, weights: list[float] | None = None) -> Case:
    r = rng.choices(range(1, n + 1), weights=weights, k=m)
    b = r[:]
    rng.shuffle(b)
    return n, r, b


def uniform(rng: random.Random, n: int, m: int) -> Case:
    return n, [rng.randint(1, n) for _ in range(m)], [rng.randint(1, n) for _ in range(m)]


def plan_case(rng: random.Random, plan: str) -> Case:
    n, m = MAX_N, MAX_M
    if plan == "one_part":
        return 1, [1], [1]
    if plan == "one_part_max":
        return 1, [1] * m, [1] * m
    if plan == "two_parts":
        return 2, [1], [2]
    if plan == "single_cable":
        return n, [5], [5]
    if plan == "single_cable_apart":
        return n, [3], [9]
    if plan == "permutation":
        return n, list(range(1, n + 1)), list(range(1, n + 1))
    if plan == "far_apart":
        return n, [1] * m, [n] * m
    if plan == "random":
        return uniform(rng, n, m)
    if plan == "balanced":
        return balanced(rng, n, m)
    if plan == "balanced_skewed":
        return balanced(rng, n, m, [2.0**-k for k in range(n)])
    if plan == "half_balanced":
        _, r, b = balanced(rng, 8, m // 2)
        _, r2, b2 = uniform(rng, n - 8, m - m // 2)
        return n, r + [x + 8 for x in r2], b + [x + 8 for x in b2]
    if plan == "off_by_one":
        _, r, b = balanced(rng, n, m)
        i = rng.randrange(m)
        b[i] = b[i] % n + 1
        return n, r, b
    if plan == "two_heavy":
        r = [rng.choice([4, 11]) for _ in range(m)]
        b = r[:]
        rng.shuffle(b)
        # 軽い部品を少し混ぜる
        for i in rng.sample(range(m), 20):
            r[i] = b[i] = rng.randint(1, n)
        return n, r, b
    if plan == "cyclic":
        return n, [i % n + 1 for i in range(m)], [(i + 1) % n + 1 for i in range(m)]
    if plan == "random_16":
        return uniform(rng, 16, m)
    if plan == "tiny_m":
        return n, [rng.randint(1, n) for _ in range(2)], [rng.randint(1, n) for _ in range(2)]
    assert plan == "random_mid"
    k = rng.randint(1, n)
    return (balanced if rng.random() < 0.5 else uniform)(rng, k, rng.randint(1, m))


def small_case(rng: random.Random, max_n: int, min_m: int, max_m: int) -> Case:
    n, m = rng.randint(1, max_n), rng.randint(min_m, max_m)
    kind = rng.randrange(3)
    if kind == 0:
        return uniform(rng, n, m)
    if kind == 1:
        return balanced(rng, n, m)
    # 部品を 2 つか 3 つに絞る。
    parts = rng.sample(range(1, n + 1), min(n, rng.randint(1, 3)))
    return n, [rng.choice(parts) for _ in range(m)], [rng.choice(parts) for _ in range(m)]


def case_for(seed: int, rng: random.Random) -> Case:
    if seed >= 2000:
        return small_case(rng, MAX_N, 8, 9)
    if seed >= 1000:
        return small_case(rng, 6, 1, 7)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    return plan_case(rng, PLANS[seed - len(SAMPLES)])


def main() -> None:
    seed = int(sys.argv[1])
    n, r, b = case_for(seed, random.Random(seed))
    m = len(r)
    assert 1 <= n <= MAX_N and 1 <= m <= MAX_M and len(b) == m
    assert all(1 <= x <= n for x in r + b)
    print(n, m)
    print(*r)
    print(*b)


if __name__ == "__main__":
    main()
