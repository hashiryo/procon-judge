"""atcoder-abc181-c (Collinearity) の入力を作る。

N と N 個の点を出す。3 点が同一直線上にあるかを Yes/No で答える。
seed が 0 から count - 1 までが本番 (例、角のケース、ランダム、同一直線にするケース)。
seed が 1000 以上なら、N <= 8 の小さい入力を出す (pj testdata crosscheck 用)。
"""

import random
import sys

LIM = 1000

SAMPLES = [
    [(0, 1), (0, 2), (0, 3), (1, 1)],
    [
        (5, 5), (0, 1), (2, 5), (8, 0), (2, 1), (0, 0), (3, 6),
        (8, 6), (5, 9), (7, 9), (3, 4), (9, 2), (9, 8), (7, 2),
    ],
    [(8, 2), (2, 3), (1, 3), (3, 7), (1, 0), (8, 8), (5, 6), (9, 7), (0, 1)],
]


def collinear(p, q, r):
    return (q[0] - p[0]) * (r[1] - p[1]) - (q[1] - p[1]) * (r[0] - p[0]) == 0


def general_position(rng, n, lim):
    pts = []
    while len(pts) < n:
        p = (rng.randint(-lim, lim), rng.randint(-lim, lim))
        if p in pts:
            continue
        ok = True
        for i in range(len(pts)):
            for j in range(i):
                if collinear(pts[i], pts[j], p):
                    ok = False
                    break
            if not ok:
                break
        if ok:
            pts.append(p)
    return pts


def with_collinear(rng, n, lim):
    # 同一直線の 3 点を先に入れ、残りはその 3 点と一直線にならないように足す。
    x = rng.randint(-lim // 2, lim // 2)
    ys = rng.sample(range(-lim, lim + 1), 3)
    pts = [(x, y) for y in ys]
    while len(pts) < n:
        p = (rng.randint(-lim, lim), rng.randint(-lim, lim))
        if p in pts:
            continue
        ok = True
        for i in range(len(pts)):
            for j in range(i):
                if collinear(pts[i], pts[j], p):
                    ok = False
                    break
            if not ok:
                break
        if ok:
            pts.append(p)
    return pts


def plan_sample(rng, i):
    return SAMPLES[i]


def plan_corner(rng):
    return [(0, 0), (1, 1), (2, 2)]  # 同一直線


def plan_corner_no(rng):
    return [(0, 0), (1, 0), (0, 1)]


def plan_vertical(_rng):
    return [(LIM, y) for y in range(-50, 50)]


def plan_random_small(rng):
    return [(rng.randint(-20, 20), rng.randint(-20, 20)) for _ in range(10)]


def plan_max_no(rng):
    return general_position(rng, 100, LIM)


def plan_max_yes(rng):
    return with_collinear(rng, 100, LIM)


PLANS = [
    lambda rng: plan_sample(rng, 0),
    lambda rng: plan_sample(rng, 1),
    lambda rng: plan_sample(rng, 2),
    plan_corner,
    plan_corner_no,
    plan_vertical,
    plan_random_small,
    plan_max_no,
    plan_max_yes,
]
COUNT = len(PLANS)


def small(rng):
    n = rng.randint(3, 8)
    pts = set()
    while len(pts) < n:
        pts.add((rng.randint(-5, 5), rng.randint(-5, 5)))
    return list(pts)


def main():
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    pts = small(rng) if seed >= 1000 else PLANS[seed % COUNT](rng)
    # 点は相異なる (問題の制約)。
    seen = set()
    out = []
    for p in pts:
        if p in seen:
            continue
        seen.add(p)
        out.append(p)
    sys.stdout.write(str(len(out)) + "\n" + "\n".join(f"{x} {y}" for x, y in out) + "\n")


main()
