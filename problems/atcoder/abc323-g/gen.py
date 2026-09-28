"""abc323-g (Inversion of Tree) の入力を作る。N と順列 P_1 ... P_N を出す。
答えは K = 0, ..., N - 1 ごとに、辺 (u, v) (u < v) のうち P_u > P_v のものがちょうど K 本の全域木の数。

seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース、N = 500 などのケース。
角のケースは、N = 2 と N = 3 の順列、愚直解で数えられる N = 8 の順列。大きいケースの P は、ランダム、
昇順 (反転が無く、答えは N^(N-2), 0, ..., 0)、降順 (全部が反転で、答えは 0, ..., 0, N^(N-2))、昇順の隣を
1 組だけ入れ替えたもの、昇順を 5 組入れ替えたもの、降順を 3 組入れ替えたもの、1 つずらしたもの
(2, 3, ..., N, 1)、後ろ半分と前半分を入れ替えたもの (反転の辺が完全 2 部グラフになる)、小と大を交互に
並べたもの。昇順に近いと、反転の辺だけのラプラシアン (x の係数) が退化していて、提出の
det_of_first_degree_poly_mat は掃き出しの途中で列を入れ替える側に何度も入る。
seed が 1000 以上なら、Prüfer 列で全域木を全部たどる愚直解で解ける小さい順列を出す (pj testdata crosscheck 用)。
"""

import random
import sys

MAX_N = 500

SAMPLES = [
    [1, 3, 2],
    [3, 1, 4, 10, 8, 6, 9, 2, 7, 5],
]
FIXED = [
    [1, 2],
    [2, 1],
    [1, 2, 3],
    [3, 2, 1],
    [2, 1, 4, 3],
    [5, 1, 8, 3, 7, 2, 6, 4],
    [1, 2, 3, 4, 5, 6, 8, 7],
]
# (N, P の作り方)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    (MAX_N, "random"),
    (MAX_N, "random"),
    (MAX_N, "sorted"),
    (MAX_N, "reversed"),
    (MAX_N, "one_swap"),
    (MAX_N, "few_swaps"),
    (MAX_N, "near_reversed"),
    (MAX_N, "rotated"),
    (MAX_N, "halves"),
    (MAX_N, "zigzag"),
    (MAX_N - 1, "random"),
    (100, "random"),
    (30, "few_swaps"),
]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)
HOWS = ["random", "sorted", "reversed", "one_swap", "few_swaps", "near_reversed", "rotated", "halves", "zigzag"]


def perm(rng: random.Random, n: int, how: str) -> list[int]:
    if how == "random":
        p = list(range(1, n + 1))
        rng.shuffle(p)
        return p
    if how in ("sorted", "one_swap", "few_swaps"):
        p = list(range(1, n + 1))
        if how == "one_swap":
            i = rng.randrange(n - 1)
            p[i], p[i + 1] = p[i + 1], p[i]
        elif how == "few_swaps":
            for _ in range(5):
                i, j = rng.randrange(n), rng.randrange(n)
                p[i], p[j] = p[j], p[i]
        return p
    if how in ("reversed", "near_reversed"):
        p = list(range(n, 0, -1))
        if how == "near_reversed":
            for _ in range(3):
                i, j = rng.randrange(n), rng.randrange(n)
                p[i], p[j] = p[j], p[i]
        return p
    if how == "rotated":
        return list(range(2, n + 1)) + [1]
    if how == "halves":
        h = n // 2
        return list(range(h + 1, n + 1)) + list(range(1, h + 1))
    assert how == "zigzag"
    lo, hi, p = 1, n, []
    while lo <= hi:
        p.append(lo)
        lo += 1
        if lo <= hi:
            p.append(hi)
            hi -= 1
    return p


def case_for(seed: int, rng: random.Random) -> list[int]:
    if seed >= 1000:
        # Prüfer 列を全部たどれる大きさ (N^(N-2) 本)。
        return perm(rng, rng.randint(2, 8), rng.choice(HOWS))
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    return perm(rng, *PLANS[seed - len(FIXED)])


def main() -> None:
    seed = int(sys.argv[1])
    p = case_for(seed, random.Random(seed))
    n = len(p)
    assert 2 <= n <= MAX_N and sorted(p) == list(range(1, n + 1))
    sys.stdout.write(f"{n}\n" + " ".join(map(str, p)) + "\n")


if __name__ == "__main__":
    main()
