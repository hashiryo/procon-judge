"""abc330-f (Minimize Bounding Square) の入力を作る。N K と N 個の点 X_i Y_i を出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、N = 2 × 10^5 と 5 × 10^4 のいろいろな点。
提出は一辺 m を二分探索し、区分線形凸関数 (永続) を平行移動して足した最小値と K を比べる。
K は、ランダム、0 (答えは x と y の幅の大きい方)、4 × 10^14、ちょうど一辺 m0 で間に合う費用と、その 1 つ手前を入れる。
一辺 m の費用は、座標を並べて i 番目に小さい値と i 番目に大きい値の組ごとに max(0, 差 - m) を足したもの (x と y の和)。
点は、ランダム、全部同じ、2 つの塊 (0 の近くと 10^9 の近く)、狭い範囲 (同じ座標だらけ)、直線の上、座標が 0 か 10^9、片方の座標が一定。
seed が 1000 以上なら、一辺を二分探索して左端の候補を全部試す愚直解で解ける小さい入力を出す (pj testdata crosscheck 用)。
2000 以上なら、愚直解でまだ解ける N = 300 までの入力を出す。
"""

import random
import sys

MAX_N = 2 * 10**5
MID_N = 5 * 10**4
MAX_C = 10**9
MAX_K = 4 * 10**14

SAMPLES = [
    (5, [(2, 0), (5, 2), (0, 3), (3, 2), (3, 4), (1, 5)]),
    (MAX_K, [(MAX_C, MAX_C)] * 4),
    (
        998244353,
        [
            (489733278, 189351894), (861289363, 30208889), (450668761, 133103889), (306319121, 739571083),
            (409648209, 922270934), (930832199, 304946211), (358683490, 923133355), (369972904, 539399938),
            (915030547, 735320146), (386219602, 277971612),
        ],
    ),
]
FIXED = [
    (0, [(0, 0)]),
    (0, [(0, 0), (MAX_C, MAX_C)]),  # 答えは 10^9 (二分探索の上端のまま)
    (MAX_K, [(0, 0), (MAX_C, MAX_C)]),
    (1, [(0, 5), (3, 0)]),
]
# (N, 点の置き方, K の決め方)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    (MAX_N, "random", "random"),
    (MAX_N, "random", "zero"),
    (MAX_N, "random", "exact"),
    (MAX_N, "random", "exact_minus"),
    (MAX_N, "clusters", "random_small"),
    (MAX_N, "narrow", "exact"),
    (MID_N, "same", "zero"),
    (MID_N, "random", "max"),
    (MID_N, "line", "random_small"),
    (MID_N, "corners", "exact"),
    (MID_N, "flat", "exact_minus"),
    (MID_N, "narrow_wide", "random_small"),
    (MID_N, "random", "exact_small"),
]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)


def cost(pts: list[tuple[int, int]], m: int) -> int:
    """一辺 m の正方形に全部の点を入れる最小の移動回数。"""
    total = 0
    for axis in (0, 1):
        v = sorted(p[axis] for p in pts)
        n = len(v)
        total += sum(max(0, v[n - 1 - i] - v[i] - m) for i in range(n // 2))
    return total


def make_points(how: str, n: int, rng: random.Random) -> list[tuple[int, int]]:
    if how == "random":
        return [(rng.randint(0, MAX_C), rng.randint(0, MAX_C)) for _ in range(n)]
    if how == "same":
        p = (rng.randint(0, MAX_C), rng.randint(0, MAX_C))
        return [p] * n
    if how == "clusters":
        return [
            (rng.randint(0, 1000), rng.randint(0, 1000)) if rng.random() < 0.5
            else (MAX_C - rng.randint(0, 1000), MAX_C - rng.randint(0, 1000))
            for _ in range(n)
        ]
    if how == "narrow":
        return [(rng.randint(0, 20), rng.randint(0, 20)) for _ in range(n)]
    if how == "line":
        return [(t, t) for t in (rng.randint(0, MAX_C) for _ in range(n))]
    if how == "corners":
        return [(rng.choice([0, MAX_C]), rng.choice([0, MAX_C])) for _ in range(n)]
    if how == "flat":
        y = rng.randint(0, MAX_C)
        return [(rng.randint(0, MAX_C), y) for _ in range(n)]
    assert how == "narrow_wide"
    return [(rng.randint(0, 10), rng.randint(0, MAX_C)) for _ in range(n)]


def make_k(how: str, pts: list[tuple[int, int]], rng: random.Random) -> int:
    if how == "random":
        return rng.randint(0, MAX_K)
    if how == "zero":
        return 0
    if how == "max":
        return MAX_K
    if how == "random_small":
        return rng.randint(0, max(0, cost(pts, 0) // 3))
    # exact: 一辺 m0 でちょうど間に合う K。exact_minus はその 1 つ手前 (m0 では足りない)。
    xs, ys = [p[0] for p in pts], [p[1] for p in pts]
    width = max(max(xs) - min(xs), max(ys) - min(ys))
    m0 = rng.randint(1, max(1, width - 1)) if how != "exact_small" else rng.randint(1, 1000)
    k = cost(pts, m0)
    if how == "exact_minus":
        k -= 1
    return max(0, min(MAX_K, k))


def small_case(rng: random.Random, medium: bool) -> tuple[int, list[tuple[int, int]]]:
    n = rng.randint(9, 300) if medium else rng.randint(1, 8)
    c = rng.choice([0, 1, 3, 10, 60, 1000, MAX_C])
    pts = [(rng.randint(0, c), rng.randint(0, c)) for _ in range(n)]
    if rng.random() < 0.2:
        pts = [(0, 0) if rng.random() < 0.5 else (c, c) for _ in range(n)]
    total = cost(pts, 0)
    k = rng.choice([0, rng.randint(0, total), total, max(0, total - 1), rng.randint(0, MAX_K)])
    return k, pts


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 1000:
        k, pts = small_case(rng, seed >= 2000)
    elif seed < len(SAMPLES):
        k, pts = SAMPLES[seed]
    elif seed < len(SAMPLES) + len(FIXED):
        k, pts = FIXED[seed - len(SAMPLES)]
    else:
        n, how_p, how_k = PLANS[seed - len(SAMPLES) - len(FIXED)]
        pts = make_points(how_p, n, rng)
        k = make_k(how_k, pts, rng)
    n = len(pts)
    assert 1 <= n <= MAX_N and 0 <= k <= MAX_K
    assert all(0 <= x <= MAX_C and 0 <= y <= MAX_C for x, y in pts)
    sys.stdout.write("\n".join([f"{n} {k}"] + [f"{x} {y}" for x, y in pts]) + "\n")


if __name__ == "__main__":
    main()
