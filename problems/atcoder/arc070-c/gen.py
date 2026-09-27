"""arc070-c (NarrowRectangles) の入力を作る。N と、N 個の区間 l r を出す。

seed が 0 から count - 1 までは本番のケース。例の 5 つ、角のケース、小さいランダム、N = 10^5 のケース。
大きいケースは、ランダム、幅 1 の区間を左右の端に交互に置くもの (答えが 5 × 10^13 くらいになる)、全部同じ区間、
幅 1 の区間を散らしたもの、右へずれていく階段と左へずれていく階段、幅の広い区間、幅がばらばらのもの、
座標が 400 以下のもの、少しずつずれていくものである。
seed が 1000 以上なら、左端の置き場所ごとに最小の費用を持つ DP の愚直解で解ける、座標の小さい入力を出す (pj testdata crosscheck 用)。
"""

import random
import sys

MAX_N = 10**5
MAX_V = 10**9

SAMPLES = [
    [(1, 3), (5, 7), (1, 3)],
    [(2, 5), (4, 6), (1, 4)],
    [(999999999, 1000000000), (1, 2), (314, 315), (500000, 500001), (999999999, 1000000000)],
    [(123456, 789012), (123, 456), (12, 345678901), (123456, 789012), (1, 23)],
    [(1, 400)],
]
FIXED = [
    [(1, MAX_V)],
    [(1, 2), (MAX_V - 1, MAX_V)],
    [(1, 3), (3, 5)],  # 端が触れていればつながっている
    [(1, 3), (4, 6)],
    [(5, 6), (1, 2), (5, 6)],
    [(1, MAX_V), (1, 2), (MAX_V - 1, MAX_V)],
]
# 小さいランダム。(N, 座標の上限)
SMALL = [(5, 10), (100, 1000), (400, 400)]
PLANS = [
    "random",
    "alternate",
    "same",
    "narrow",
    "stairs_up",
    "stairs_down",
    "wide",
    "mixed",
    "small",
    "walk",
]
COUNT = len(SAMPLES) + len(FIXED) + len(SMALL) + len(PLANS)


def interval(rng: random.Random, width: int, top: int) -> tuple[int, int]:
    """幅 width の区間を、座標が 1 以上 top 以下に収まるようにランダムに置く。"""
    width = max(1, min(width, top - 1))
    left = rng.randint(1, top - width)
    return left, left + width


def random_intervals(rng: random.Random, n: int, top: int) -> list[tuple[int, int]]:
    out = []
    for _ in range(n):
        left, right = rng.sample(range(1, top + 1), 2)
        out.append((min(left, right), max(left, right)))
    return out


def plan_case(rng: random.Random, how: str) -> list[tuple[int, int]]:
    n = MAX_N
    if how == "random":
        return random_intervals(rng, n, MAX_V)
    if how == "alternate":
        return [(1, 2) if i % 2 == 0 else (MAX_V - 1, MAX_V) for i in range(n)]
    if how == "same":
        return [interval(rng, rng.randint(1, MAX_V - 1), MAX_V)] * n
    if how == "narrow":
        return [interval(rng, 1, MAX_V) for _ in range(n)]
    if how in ("stairs_up", "stairs_down"):
        out = [(1 + i * 10**4, 2 + i * 10**4) for i in range(n)]
        return out if how == "stairs_up" else out[::-1]
    if how == "wide":
        return [interval(rng, rng.randint(4 * 10**8, MAX_V - 1), MAX_V) for _ in range(n)]
    if how == "mixed":
        return [interval(rng, int(10 ** rng.uniform(0, 9)), MAX_V) for _ in range(n)]
    if how == "small":
        return random_intervals(rng, n, 400)
    # walk: 左端が少しずつずれ、幅もばらばら。
    out, left = [], MAX_V // 2
    for _ in range(n):
        left = min(MAX_V - 10**4, max(1, left + rng.randint(-10**4, 10**4)))
        out.append((left, left + rng.randint(1, 10**4)))
    return out


def small_case(rng: random.Random) -> list[tuple[int, int]]:
    """愚直解の DP が座標ごとに回せる大きさ。"""
    n = rng.randint(1, 12) if rng.random() < 0.9 else rng.randint(13, 150)
    top = rng.choice([10, 30, 60, 150])
    how = rng.choice(["random", "narrow", "narrow", "alternate", "stairs"])
    if how == "random":
        return random_intervals(rng, n, top)
    if how == "narrow":
        return [interval(rng, rng.randint(1, 2), top) for _ in range(n)]
    if how == "alternate":
        return [(1, 2) if i % 2 == 0 else (top - 1, top) for i in range(n)]
    step = rng.randint(2, 4)
    out = [interval(rng, 1, top)]
    for _ in range(n - 1):
        left = min(top - 1, max(1, out[-1][0] + rng.choice([-step, step])))
        out.append((left, left + 1))
    return out


def case_for(seed: int, rng: random.Random) -> list[tuple[int, int]]:
    if seed >= 1000:
        return small_case(rng)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    seed -= len(FIXED)
    if seed < len(SMALL):
        n, top = SMALL[seed]
        return random_intervals(rng, n, top)
    return plan_case(rng, PLANS[seed - len(SMALL)])


def main() -> None:
    seed = int(sys.argv[1])
    rects = case_for(seed, random.Random(seed))
    assert 1 <= len(rects) <= MAX_N and all(1 <= left < right <= MAX_V for left, right in rects)
    out = [str(len(rects))] + [f"{left} {right}" for left, right in rects]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
