"""atcoder-abc266-c (Convex Quadrilateral) の入力を作る。

反時計回りの四角形の 4 頂点を出し、凸かを Yes/No で答える。3 点が一直線に並ばないことは保証する。
seed が 0 から count - 1 までが本番 (例、角、凸、凹、ランダム)。
seed が 1000 以上なら、小さいランダムな四角形を出す (pj testdata crosscheck 用)。
"""

import math
import random
import sys

LIM = 1000

SAMPLES = [
    [(0, 0), (1, 0), (1, 1), (0, 1)],
    [(0, 0), (1, 1), (-1, 0), (1, -1)],
]


def collinear(p, q, r):
    return (q[0] - p[0]) * (r[1] - p[1]) - (q[1] - p[1]) * (r[0] - p[0]) == 0


def no_three_collinear(pts):
    n = len(pts)
    for i in range(n):
        for j in range(i):
            for k in range(j):
                if collinear(pts[i], pts[j], pts[k]):
                    return False
    return True


def angular_order(pts):
    """重心のまわりの角度の順 (反時計回り)。単純な多角形になる。"""
    cx = sum(x for x, y in pts) / len(pts)
    cy = sum(y for x, y in pts) / len(pts)
    return sorted(pts, key=lambda p: math.atan2(p[1] - cy, p[0] - cx))


def random_quad(rng, lim):
    while True:
        pts = set()
        while len(pts) < 4:
            pts.add((rng.randint(-lim, lim), rng.randint(-lim, lim)))
        pts = list(pts)
        if not no_three_collinear(pts):
            continue
        return angular_order(pts)


def convex_quad(rng, lim):
    while True:
        pts = set()
        while len(pts) < 4:
            pts.add((rng.randint(-lim, lim), rng.randint(-lim, lim)))
        pts = list(pts)
        if not no_three_collinear(pts):
            continue
        h = hull(pts)
        if len(h) == 4:
            return h


def concave_quad(rng, lim):
    while True:
        quad = random_quad(rng, lim)
        # 頂点のどれかが他の 3 点の三角形の内側なら凹。
        for i in range(4):
            tri = [quad[j] for j in range(4) if j != i]
            if inside_triangle(tri, quad[i]):
                return quad


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


def inside_triangle(tri, p):
    def cross(o, a, b):
        return (a[0] - o[0]) * (b[1] - o[1]) - (a[1] - o[1]) * (b[0] - o[0])

    s = [cross(tri[i], tri[(i + 1) % 3], p) for i in range(3)]
    return all(v > 0 for v in s) or all(v < 0 for v in s)


def plan_square(_rng):
    return [(0, 0), (10, 0), (10, 10), (0, 10)]


def plan_rotated(_rng):
    return [(0, 0), (2, 1), (1, 3), (-1, 2)]


def plan_thin(_rng):
    return [(0, 0), (1000, 1), (1000, 2), (0, 1)]


def plan_sample(_rng):
    return SAMPLES[0]


def plan_sample_no(_rng):
    return SAMPLES[1]


def plan_concave(rng):
    return concave_quad(rng, LIM)


def plan_convex(rng):
    return convex_quad(rng, LIM)


def plan_random(rng):
    return random_quad(rng, LIM)


PLANS = [
    plan_sample,
    plan_sample_no,
    plan_square,
    plan_rotated,
    plan_thin,
    plan_concave,
    plan_convex,
    plan_random,
]
COUNT = len(PLANS)


def main():
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    pts = random_quad(rng, 10) if seed >= 1000 else PLANS[seed % COUNT](rng)
    sys.stdout.write("\n".join(f"{x} {y}" for x, y in pts) + "\n")


main()
