"""qoj-1846 (Nimber Sequence) の入力を作る。K m、a_1 ... a_{K-1}、b_1 ... b_5、c_1 ... c_5 を出す (値は 2^32 未満の nimber)。

漸化式は a_n = (⊕ a_{n-i} ⊗ b_i) ⊕ (⊕ a_{n-K+i} ⊗ c_i) (n >= K) で、答えは a_m。
seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース (K = 6 で b と c の項が重なる、m = 1、
m = K - 1 で漸化式を使わない、m = K、b も c も 0、b_1 だけ 1 で定数列、c だけ、値が 0 と 2^32 - 1 だけ)、
K = 10^5 で m = 10^18 や 2^59 - 1 (10^18 以下で bit が全部立つので、x 倍が最も多い) のケース。
seed が 1000 以上なら、漸化式を順に計算する愚直解で解ける m <= 3 × 10^5 の入力を出す (pj testdata crosscheck 用)。
"""

import random
import sys

MAX_K = 10**5
MAX_M = 10**18
TOP = 2**32 - 1

SAMPLES = [
    (6, 10**18, [1, 2, 3, 4, 5], [1, 0, 0, 0, 0], [0, 0, 0, 0, 0]),
    (6, 10, [1, 2, 3, 4, 5], [6, 7, 8, 9, 10], [11, 12, 13, 14, 15]),
    (11, 123, [849, 674, 223, 677, 243, 657, 979, 583, 643, 845], [979, 282, 313, 567, 433], [122, 443, 132, 554, 132]),
]


def values(rng: random.Random, n: int, how: str) -> list[int]:
    if how == "random":
        return [rng.randint(0, TOP) for _ in range(n)]
    if how == "small":
        return [rng.randint(0, 3) for _ in range(n)]
    if how == "extreme":
        return [rng.choice([0, TOP]) for _ in range(n)]
    assert how == "zero"
    return [0] * n


# (K, m, a の作り方, b の作り方, c の作り方)
PLANS = [
    (6, MAX_M, "random", "random", "random"),
    (6, 1, "random", "random", "random"),
    (MAX_K, MAX_K - 1, "random", "random", "random"),
    (MAX_K, MAX_K, "random", "random", "random"),
    (MAX_K, MAX_M, "random", "zero", "zero"),
    (MAX_K, MAX_M, "random", "one", "zero"),
    (MAX_K, MAX_M, "random", "zero", "random"),
    (MAX_K, MAX_M, "extreme", "extreme", "extreme"),
    (MAX_K, MAX_M, "random", "random", "random"),
    (MAX_K, 2**59 - 1, "random", "random", "random"),
    (MAX_K, None, "random", "random", "random"),
    (7, 2**59 - 1, "small", "small", "small"),
]
COUNT = len(SAMPLES) + len(PLANS)


def coefficients(rng: random.Random, how: str) -> list[int]:
    if how == "one":
        return [1, 0, 0, 0, 0]
    return values(rng, 5, how)


def case_for(seed: int) -> tuple[int, int, list[int], list[int], list[int]]:
    rng = random.Random(seed)
    if seed >= 1000:
        k = rng.randint(6, rng.choice([10, 100, 2000]))
        m = rng.randint(1, rng.choice([k + 10, 3 * 10**5]))
        how = rng.choice(["random", "small", "extreme"])
        b, c = coefficients(rng, rng.choice(["random", "zero", "one", how])), coefficients(rng, rng.choice(["random", "zero", how]))
        return k, m, values(rng, k - 1, how), b, c
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    k, m, ha, hb, hc = PLANS[seed - len(SAMPLES)]
    if m is None:
        m = rng.randint(MAX_M // 2, MAX_M)
    return k, m, values(rng, k - 1, ha), coefficients(rng, hb), coefficients(rng, hc)


def main() -> None:
    seed = int(sys.argv[1])
    k, m, a, b, c = case_for(seed)
    assert 6 <= k <= MAX_K and 1 <= m <= MAX_M and len(a) == k - 1 and len(b) == len(c) == 5
    assert all(0 <= x <= TOP for x in a + b + c)
    sys.stdout.write(f"{k} {m}\n{' '.join(map(str, a))}\n{' '.join(map(str, b))}\n{' '.join(map(str, c))}\n")


if __name__ == "__main__":
    main()
