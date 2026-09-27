"""past202303-m (Put Away) の入力を作る。N M、A_1 ... A_N、B_1 ... B_M を出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、N と M が 2 × 10^5 の列。
列は、両方ランダム (途中で入らなくなる)、箱が荷物の 2 倍あるランダム (全部入る)、荷物をまとめた和を
箱の大きさにしたもの (全部の箱がちょうど埋まる)、全部 10^9、全部同じ大きさで箱が 1 つ足りないもの
(最後の荷物が入らない)、最初の荷物が入らないもの、箱が 1 つ、荷物が 1 つで入るのが最後の箱だけ、
箱がだんだん大きくなるもの、小さい値だけのもの。データを 30 MB ほどに収めるため、すぐ答えが決まるものは
小さい値にする。
seed が 1000 以上なら、荷物ごとに左の箱から順に空きを調べる愚直解で解ける小さい入力を出す
(pj testdata crosscheck 用)。2000 以上なら、愚直解でまだ解ける N, M = 3000 までの入力を出す。
"""

import random
import sys

MAX_NM = 2 * 10**5
MAX_V = 10**9

SAMPLES = [
    ([4, 7, 3], [10, 4, 8]),
    ([4, 7, 3], [10, 4]),
    ([5, 4, 3, 2, 1], [1, 2, 3, 4, 5]),
]
FIXED = [
    ([1], [1]),
    ([2], [1]),  # 最初の荷物が入らない
    ([MAX_V], [MAX_V]),
    ([MAX_V, 1], [MAX_V]),  # 最後の荷物が入らない
    ([1, 1, 1], [MAX_V]),
    ([3, 3, 3], [1, 2, 3, 4, 5, 6]),
    ([5, 5, 2, 2, 1], [6, 6]),
]
# (N, M, 値の出し方)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    (MAX_NM, MAX_NM, "random"),
    (MAX_NM // 2, MAX_NM, "random"),
    (MAX_NM, MAX_NM, "exact"),
    (MAX_NM, MAX_NM, "all_max"),
    (MAX_NM, MAX_NM - 1, "one_short"),
    (MAX_NM, MAX_NM, "first_fails"),
    (MAX_NM, 1, "one_box"),
    (1, MAX_NM, "last_box"),
    (MAX_NM, MAX_NM, "increasing"),
    (MAX_NM, MAX_NM, "small"),
]


def plan_values(rng: random.Random, n: int, m: int, how: str) -> tuple[list[int], list[int]]:
    if how == "random":
        return [rng.randint(1, MAX_V) for _ in range(n)], [rng.randint(1, MAX_V) for _ in range(m)]
    if how == "exact":
        # 荷物を 1 から 3 個ずつまとめ、その和を箱の大きさにする。余った箱は大きさをランダムにする。
        a = [rng.randint(1, MAX_V // 3) for _ in range(n)]
        b = []
        i = 0
        while i < n:
            k = min(n - i, rng.randint(1, 3))
            b.append(sum(a[i:i + k]))
            i += k
        return a, (b + [rng.randint(1, MAX_V) for _ in range(m - len(b))])[:m]
    if how == "all_max":
        return [MAX_V] * n, [MAX_V] * m
    if how == "one_short":
        return [7] * n, [7] * m
    if how == "first_fails":
        return [MAX_V] + [rng.randint(1, 1000) for _ in range(n - 1)], [rng.randint(1, 1000) for _ in range(m)]
    if how == "one_box":
        return [rng.randint(1, 5000) for _ in range(n)], [MAX_V]
    if how == "last_box":
        return [MAX_V], [rng.randint(1, 1000) for _ in range(m - 1)] + [MAX_V]
    if how == "increasing":
        # 箱が右ほど大きいので、大きな荷物は右の方まで探しに行く。
        return [rng.randint(1, MAX_V) for _ in range(n)], sorted(rng.randint(1, MAX_V) for _ in range(m))
    assert how == "small"
    return [rng.randint(1, 10) for _ in range(n)], [rng.randint(1, 10) for _ in range(m)]


def small_case(rng: random.Random, medium: bool) -> tuple[list[int], list[int]]:
    """愚直解で解ける大きさ。値は、狭い範囲、ランダム、端の値を混ぜる。"""
    limit = 3000 if medium else 10
    n, m = rng.randint(1, limit), rng.randint(1, limit)
    how = rng.choice(["tiny", "random", "edge", "exact"])
    if how == "exact":
        return plan_values(rng, n, m, "exact")
    if how == "tiny":
        hi = rng.randint(1, 10)
        return [rng.randint(1, hi) for _ in range(n)], [rng.randint(1, hi) for _ in range(m)]
    if how == "edge":
        pick = [1, 2, MAX_V - 1, MAX_V]
        return [rng.choice(pick) for _ in range(n)], [rng.choice(pick) for _ in range(m)]
    return [rng.randint(1, MAX_V) for _ in range(n)], [rng.randint(1, MAX_V) for _ in range(m)]


def case_for(seed: int, rng: random.Random) -> tuple[list[int], list[int]]:
    if seed >= 1000:
        return small_case(rng, seed >= 2000)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    n, m, how = PLANS[seed - len(FIXED)]
    return plan_values(rng, n, m, how)


def main() -> None:
    seed = int(sys.argv[1])
    a, b = case_for(seed, random.Random(seed))
    n, m = len(a), len(b)
    assert 1 <= n <= MAX_NM and 1 <= m <= MAX_NM and all(1 <= v <= MAX_V for v in a + b)
    sys.stdout.write(f"{n} {m}\n{' '.join(map(str, a))}\n{' '.join(map(str, b))}\n")


if __name__ == "__main__":
    main()
