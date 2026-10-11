"""abc281-e (Least Elements) の入力を作る。N M K と A を出す。

seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース、小さいランダム、N = 2 × 10^5 のケース。
大きいケースは、M と K の組み合わせ (窓が長くて K が真ん中、M = N、K = M、K = 1、窓が短い) と、A の出し方
(ランダム、全部 10^9 で答えが 2 × 10^14 になるもの、昇順、降順、0 から 2 だけで同じ値だらけのもの) を混ぜる。
seed が 1000 以上なら、窓ごとに並べ替える愚直解で解ける小さい入力を出す (pj testdata crosscheck 用)。
"""

import random
import sys

MAX_N = 2 * 10**5
MAX_A = 10**9

SAMPLES = [
    (4, 3, [3, 1, 4, 1, 5, 9]),
    (6, 3, [12, 2, 17, 11, 19, 8, 4, 3, 6, 20]),
]
FIXED = [
    (1, 1, [0]),
    (1, 1, [MAX_A]),
    (3, 3, [MAX_A, MAX_A, MAX_A]),
    (2, 1, [5, 5, 5, 5, 5]),
    (5, 2, [0, MAX_A, 0, MAX_A, 1]),
]
# 小さいランダム。(N, M, K, A の上限)
SMALL = [(10, 4, 2, 5), (1000, 100, 30, 100), (1000, 1000, 1000, 10**9)]
# 大きいケース。(M, K, A の出し方)
PLANS = [
    (MAX_N // 2, MAX_N // 4, "random"),
    (MAX_N, MAX_N // 2, "random"),
    (MAX_N // 2, MAX_N // 2, "random"),
    (MAX_N // 2, 1, "random"),
    (1000, 500, "random"),
    (2, 1, "random"),
    (MAX_N // 2, MAX_N // 4, "max"),
    (MAX_N // 2, MAX_N // 4, "asc"),
    (MAX_N // 2, MAX_N // 4, "desc"),
    (MAX_N // 2, MAX_N // 4, "ties"),
]
COUNT = len(SAMPLES) + len(FIXED) + len(SMALL) + len(PLANS)


def values(rng: random.Random, n: int, how: str, max_a: int = MAX_A) -> list[int]:
    if how == "random":
        return [rng.randint(0, max_a) for _ in range(n)]
    if how == "max":
        return [MAX_A] * n
    if how == "asc":
        return sorted(rng.randint(0, MAX_A) for _ in range(n))
    if how == "desc":
        return sorted((rng.randint(0, MAX_A) for _ in range(n)), reverse=True)
    return [rng.randint(0, 2) for _ in range(n)]  # ties


def small_case(rng: random.Random) -> tuple[int, int, list[int]]:
    n = rng.randint(1, 30) if rng.random() < 0.9 else rng.randint(31, 300)
    m = rng.randint(1, n)
    k = rng.randint(1, m)
    how = rng.choice(["small", "small", "large", "equal"])
    if how == "small":
        a = [rng.randint(0, 3) for _ in range(n)]
    elif how == "large":
        a = [rng.randint(0, MAX_A) for _ in range(n)]
    else:
        a = [rng.choice([0, MAX_A, rng.randint(0, MAX_A)])] * n
    return m, k, a


def case_for(seed: int, rng: random.Random) -> tuple[int, int, list[int]]:
    if seed >= 1000:
        return small_case(rng)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    seed -= len(FIXED)
    if seed < len(SMALL):
        n, m, k, max_a = SMALL[seed]
        return m, k, values(rng, n, "random", max_a)
    m, k, how = PLANS[seed - len(SMALL)]
    return m, k, values(rng, MAX_N, how)


def main() -> None:
    seed = int(sys.argv[1])
    m, k, a = case_for(seed, random.Random(seed))
    n = len(a)
    assert 1 <= k <= m <= n <= MAX_N and all(0 <= v <= MAX_A for v in a)
    sys.stdout.write(f"{n} {m} {k}\n" + " ".join(map(str, a)) + "\n")


if __name__ == "__main__":
    main()
