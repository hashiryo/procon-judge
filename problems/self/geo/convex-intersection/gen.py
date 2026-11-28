"""self-geo-convex-intersection の入力を作る。

n、n 個の点 (反時計回りの凸多角形)、m、m 個の点を出す。2 つの凸多角形の共通部分の面積を答える。
seed が 0 から count - 1 までが本番。seed が 1000 以上なら、n,m <= 10 の小さい入力を出す
(pj testdata crosscheck で brute.cpp と突き合わせる)。

参照実装は小さい方ではない多角形 (a) を b の辺で 1 本ずつ切るので、費用は (辺の本数) * (切るたびの
多角形の頂点数) になる。頂点が増えるケースは辺の本数を 3000 までにする。
"""

import math
import random
import sys

MAXAB = 3000


def hull(pts):
    pts = sorted(set(pts))
    if len(pts) <= 2:
        return pts

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


def random_polygon(rng, v, rmin, rmax, dx=0, dy=0):
    while True:
        pts = []
        for _ in range(v):
            t = rng.random() * 2 * math.pi
            r = rng.uniform(rmin, rmax)
            pts.append((round(r * math.cos(t)) + dx, round(r * math.sin(t)) + dy))
        h = hull(pts)
        if len(h) >= 3:
            return h


def regular(rng, v, r, dx=0, dy=0):
    t0 = rng.random() * 2 * math.pi
    pts = [(round(r * math.cos(t0 + 2 * math.pi * k / v)) + dx, round(r * math.sin(t0 + 2 * math.pi * k / v)) + dy) for k in range(v)]
    return hull(pts)


def add_collinear(rng, poly):
    """辺の上に点を足す (凸包を作り直すと消える)。"""
    out = []
    n = len(poly)
    for i in range(n):
        x1, y1 = poly[i]
        x2, y2 = poly[(i + 1) % n]
        out.append((x1, y1))
        if rng.random() < 0.3:
            t = rng.uniform(0.2, 0.8)
            out.append((round(x1 + t * (x2 - x1)), round(y1 + t * (y2 - y1))))
    return out


def plan_square_overlap(rng):
    a = [(0, 0), (10, 0), (10, 10), (0, 10)]
    b = regular(rng, 4, 7, 6, 6)
    return a, b


def plan_disjoint(rng):
    a = regular(rng, 5, 1000, 0, 0)
    b = regular(rng, 6, 1000, 5000, 0)
    return a, b


def plan_inside(rng):
    a = regular(rng, 7, 5000, 0, 0)
    b = regular(rng, 4, 1000, 100, 100)
    return a, b


def plan_identical(rng):
    a = regular(rng, 6, 3000, 0, 0)
    return a, a


def plan_touch(rng):
    a = [(0, 0), (10, 0), (10, 10), (0, 10)]
    b = [(10, 0), (20, 0), (20, 10), (10, 10)]
    return a, b


def plan_triangle(rng):
    a = [(0, 0), (10**4, 0), (0, 10**4)]
    b = regular(rng, 4, 4000, 3000, 3000)
    return a, b


def plan_thin(rng):
    a = [(-10**4, 0), (10**4, 1), (10**4, 3), (-10**4, 2)]
    b = [(-10**4, 1), (10**4, -1), (10**4, 2), (-10**4, 4)]
    return a, b


def plan_random_small(rng):
    a = random_polygon(rng, rng.randint(3, 10), 3000, 8000)
    b = random_polygon(rng, rng.randint(3, 10), 3000, 8000, rng.randint(-4000, 4000), rng.randint(-4000, 4000))
    return a, b


def plan_random_500(rng):
    a = random_polygon(rng, 500, 4000, 8000)
    b = random_polygon(rng, 500, 4000, 8000, rng.randint(-3000, 3000), rng.randint(-3000, 3000))
    return a, b


def plan_random_3000(rng):
    a = regular(rng, 3000, 8000)
    b = regular(rng, 3000, 7000, 2000, 1000)
    return a, b


def plan_regular_2000(rng):
    a = regular(rng, 2000, 8000)
    b = random_polygon(rng, 2000, 1000, 7000, 3000, 0)
    return a, b


def plan_collinear(rng):
    a = add_collinear(rng, regular(rng, 6, 6000))
    b = add_collinear(rng, regular(rng, 5, 5000, 2000, 2000))
    return a, b


def plan_near_parallel(rng):
    a = [(0, 0), (10**4, 10), (10**4, 20), (0, 15)]
    b = [(0, 5), (10**4, 12), (10**4, 18), (0, 10)]
    return a, b


PLANS = [
    plan_square_overlap,
    plan_disjoint,
    plan_inside,
    plan_identical,
    plan_touch,
    plan_triangle,
    plan_thin,
    plan_random_small,
    plan_random_500,
    plan_random_3000,
    plan_regular_2000,
    plan_collinear,
    plan_near_parallel,
]
COUNT = len(PLANS)


def small(rng):
    a = random_polygon(rng, rng.randint(3, 10), 10, 100)
    b = random_polygon(rng, rng.randint(3, 10), 10, 100, rng.randint(-100, 100), rng.randint(-100, 100))
    return a, b


def main():
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    a, b = small(rng) if seed >= 1000 else PLANS[seed % COUNT](rng)
    out = [str(len(a))] + [f"{x} {y}" for x, y in a] + [str(len(b))] + [f"{x} {y}" for x, y in b]
    sys.stdout.write("\n".join(out) + "\n")


main()
