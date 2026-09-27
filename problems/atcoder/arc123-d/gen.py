"""arc123-d (Inc, Dec - Decomposition) の入力を作る。N と A を出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、N = 1 と 2 の角のケース、N = 2 * 10^5 の
大きいもの。値は、全部 0、全部最大、全部最小、増える列、減る列、最小と最大を交互に並べたもの
(B と C が 10^13 ほどまで広がり、答えは 2 * 10^18 ほどになる)、ランダム、小さい値のランダム、
ランダムウォーク、のこぎり波、長い上りと下りをつないだ山を混ぜる。
seed が 1000 以上なら、B_i の値を全部試す DP (愚直解) で解ける、N と |A_i| が小さい入力を出す
(pj testdata crosscheck 用)。
"""

import random
import sys

MAX_N = 2 * 10**5
MAX_A = 10**8

SAMPLES = [
    [1, -2, 3],  # 例 1
    [5, 4, 3, 5],  # 例 2
    [-10],  # 例 3
]
FIXED = [
    [0],
    [MAX_A],
    [-MAX_A],
    [-MAX_A, MAX_A],
    [MAX_A, -MAX_A],
    [MAX_A, MAX_A],
]
# (N, 値の出し方)。本番のケースのうち FIXED のあとに並べる。
PLANS = [
    (MAX_N, "zero"),
    (MAX_N, "max"),
    (5000, "min"),
    (MAX_N, "increasing"),
    (MAX_N, "decreasing"),
    (MAX_N, "alternating"),  # 最小から始める
    (MAX_N, "alternating_max"),  # 最大から始める
    (MAX_N, "random"),
    (MAX_N, "small"),
    (MAX_N, "walk"),
    (MAX_N, "sawtooth"),
    (MAX_N, "mountains"),
    (None, "random"),  # N はランダム
    (None, "alternating"),
    (None, "mountains"),
]
PATTERNS = ["zero", "max", "min", "increasing", "decreasing", "alternating", "alternating_max",
            "random", "small", "walk", "sawtooth", "mountains"]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)


def values(rng: random.Random, n: int, how: str, bound: int) -> list[int]:
    """|A_i| <= bound の列を作る。"""
    if how == "zero":
        return [0] * n
    if how == "max":
        return [bound] * n
    if how == "min":
        return [-bound] * n
    if how in ("increasing", "decreasing"):
        a = sorted(rng.randint(-bound, bound) for _ in range(n))
        return a if how == "increasing" else a[::-1]
    if how in ("alternating", "alternating_max"):
        first = -bound if how == "alternating" else bound
        return [first if i % 2 == 0 else -first for i in range(n)]
    if how == "small":
        return [rng.randint(-min(bound, 3), min(bound, 3)) for _ in range(n)]
    if how == "walk":
        step, a, x = max(bound // 1000, 1), [], rng.randint(-bound, bound)
        for _ in range(n):
            a.append(x)
            x = min(max(x + rng.randint(-step, step), -bound), bound)
        return a
    if how == "sawtooth":
        # ゆっくり上がって一気に落ちる。周期はランダム。
        period = rng.randint(2, max(2, min(n, 1000)))
        return [-bound + (2 * bound) * (i % period) // period for i in range(n)]
    if how == "mountains":
        # 長い上りと下りを交互につなぐ。
        a, x, up = [], rng.randint(-bound, bound), rng.random() < 0.5
        while len(a) < n:
            run = rng.randint(1, max(1, n // 10))
            target = rng.randint(x, bound) if up else rng.randint(-bound, x)
            a += [x + (target - x) * k // run for k in range(1, run + 1)]
            x, up = target, not up
        return a[:n]
    return [rng.randint(-bound, bound) for _ in range(n)]


def case_for(seed: int, rng: random.Random) -> list[int]:
    if seed >= 1000:
        # 愚直解の DP が持つ値の幅は、およそ 2 * (N + 1) * 2 * bound。
        n = rng.randint(1, 20)
        bound = rng.choice([1, 3, 10, 100, 1000])
        return values(rng, n, rng.choice(PATTERNS), bound)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    n, how = PLANS[seed - len(FIXED)]
    return values(rng, n or rng.randint(2, 100), how, MAX_A)


def main() -> None:
    seed = int(sys.argv[1])
    a = case_for(seed, random.Random(seed))
    assert 1 <= len(a) <= MAX_N and all(-MAX_A <= x <= MAX_A for x in a)
    sys.stdout.write(f"{len(a)}\n{' '.join(map(str, a))}\n")


if __name__ == "__main__":
    main()
