"""atcoder-abc251-g (Intersection of Polygons) の入力を作る。

N 個の点 (反時計回りの凸多角形 P)、M 個の平行移動、Q 個のクエリ点を出し、
各クエリ点が M 個の平行移動した P のすべてに含まれるかを Yes/No で答える。
seed が 0 から count - 1 までが本番 (例、角、ランダム、最大)。
seed が 1000 以上なら、小さいランダムを出す (pj testdata crosscheck 用)。
"""

import math
import random
import sys

LIM = 10**8

SAMPLES = [
    (
        [(-2, -3), (0, -2), (1, 0), (0, 2), (-2, 1)],
        [(0, 1), (1, 0)],
        [(0, 0), (1, 0), (0, 1), (1, 1), (-1, -1), (-1, -2)],
    ),
]


def hull(pts):
    pts = sorted(set(pts))

    def cross(o, a, b):
        return (a[0] - o[0]) * (b[1] - o[1]) - (a[1] - o[1]) * (b[0] - o[0])

    lo = []
    for p in pts:
        while len(lo) >= 2 and cross(lo[-2], lo[-1], p) <= 0:
            lo.pop()
        lo.append(p)
    up = []
    for p in reversed(pts):
        while len(up) >= 2 and cross(up[-2], up[-1], p) <= 0:
            up.pop()
        up.append(p)
    return lo[:-1] + up[:-1]


def regular(rng, n, r, dx=0, dy=0):
    t0 = rng.random() * 2 * math.pi
    pts = [(round(r * math.cos(t0 + 2 * math.pi * k / n)) + dx, round(r * math.sin(t0 + 2 * math.pi * k / n)) + dy) for k in range(n)]
    return hull(pts)


def random_convex(rng, n, r, dx=0, dy=0):
    while True:
        pts = []
        for _ in range(n * 3):
            t = rng.random() * 2 * math.pi
            rr = rng.uniform(r * 0.5, r)
            pts.append((round(rr * math.cos(t)) + dx, round(rr * math.sin(t)) + dy))
        h = hull(pts)
        if 3 <= len(h) <= n:
            return h


def random_points(rng, n, lim):
    return [(rng.randint(-lim, lim), rng.randint(-lim, lim)) for _ in range(n)]


def main():
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 1000:
        p = random_convex(rng, 5, 30)
        m = random_points(rng, rng.randint(1, 5), 50)
        q = random_points(rng, rng.randint(1, 10), 60)
    else:
        plans = [
            lambda: SAMPLES[0],
            lambda: (regular(rng, 3, 1000), [(0, 0)], random_points(rng, 20, 3000)),
            lambda: (regular(rng, 4, 10**8), [(LIM, LIM)], random_points(rng, 10, LIM)),
            lambda: (regular(rng, 50, 10**8), random_points(rng, 2000, LIM), random_points(rng, 2000, LIM)),
            lambda: (regular(rng, 5, 1000), [(10**6, 0)], random_points(rng, 50, 10**6)),
            lambda: (random_convex(rng, 50, 10**8), random_points(rng, 200000, LIM), random_points(rng, 200000, LIM)),
            lambda: (regular(rng, 3, 10**8), [(LIM, -LIM), (-LIM, LIM)], random_points(rng, 100, LIM)),
            lambda: (random_convex(rng, 10, 1000), random_points(rng, 200000, 2000), random_points(rng, 200000, 2000)),
            lambda: (regular(rng, 50, 1000), [(i, -i) for i in range(50)], random_points(rng, 100, 2000)),
            lambda: (random_convex(rng, 50, 10**8), random_points(rng, 200000, 10**4), random_points(rng, 200000, 10**4)),
        ]
        p, m, q = plans[seed % len(plans)]()
    out = [str(len(p))]
    out += [f"{x} {y}" for x, y in p]
    out.append(str(len(m)))
    out += [f"{x} {y}" for x, y in m]
    out.append(str(len(q)))
    out += [f"{x} {y}" for x, y in q]
    sys.stdout.write("\n".join(out) + "\n")


main()
