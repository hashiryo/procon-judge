"""self-geo-half-plane-intersection の入力を作る。

N と N 本の半平面 a_i x + b_i y + c_i >= 0 を出す。どのケースも |x|,|y| <= 10^4 の箱の 4 本を含み、
共通部分は箱の中 (有界)。面積は 0 のこともある (空、点、線分になる)。
seed が 0 から count - 1 までが本番。seed が 1000 以上なら、N <= 12 の小さい入力を出す
(pj testdata crosscheck で brute.cpp と突き合わせる)。

参照実装は箱 (±2*10^4) から始めて 1 本ずつ切るので、共通部分が箱の外に出るケースは作らない。
参照実装の費用は切るたびの多角形の頂点数の和なので、頂点が増えるケースは N を小さくする
(N * 頂点数 が 2*10^7 以下になるようにする)。
"""

import math
import random
import sys

B = 10**4
MAXC = 10**9


def box():
    return [(1, 0, B), (-1, 0, B), (0, 1, B), (0, -1, B)]


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


def poly_halfplanes(pts):
    """反時計回りの凸多角形の各辺の内側 (左) の半平面を返す。"""
    out = []
    n = len(pts)
    for i in range(n):
        x1, y1 = pts[i]
        x2, y2 = pts[(i + 1) % n]
        dx, dy = x2 - x1, y2 - y1
        # cross((dx,dy), x-(x1,y1)) >= 0  <=>  -dy x + dx y + dy x1 - dx y1 >= 0
        out.append((-dy, dx, dy * x1 - dx * y1))
    return out


def regular(rng, v, r):
    """半径 r の正 v 角形 (integer に丸めて凸包を取る)。"""
    t0 = rng.random() * 2 * math.pi
    pts = [(round(r * math.cos(t0 + 2 * math.pi * k / v)), round(r * math.sin(t0 + 2 * math.pi * k / v))) for k in range(v)]
    return hull(pts)


def random_polygon(rng, v, rmin, rmax):
    """円盤のランダムな点の凸包。頂点が 3 未満ならやり直す。"""
    while True:
        pts = []
        for _ in range(v):
            t = rng.random() * 2 * math.pi
            r = rng.uniform(rmin, rmax)
            pts.append((round(r * math.cos(t)), round(r * math.sin(t))))
        h = hull(pts)
        if len(h) >= 3:
            return h


def redundant(rng, pts, count, ab=10**4):
    """pts をすべて含む半平面を count 本作る。"""
    out = []
    for _ in range(count):
        while True:
            a = rng.randint(-ab, ab)
            b = rng.randint(-ab, ab)
            if a or b:
                break
        m = min(a * x + b * y for x, y in pts)
        c = -m + rng.randint(1, 10**6)
        out.append((a, b, min(c, MAXC)))
    return out


def plan_box(_rng):
    return box()


def plan_triangle(rng):
    pts = random_polygon(rng, 3, 2 * 10**3, 6 * 10**3)
    return box() + poly_halfplanes(pts)


def plan_empty(_rng):
    return box() + [(1, 0, -1), (-1, 0, 0)]


def plan_point(_rng):
    return box() + [(1, 0, 0), (-1, 0, 0), (0, 1, 0), (0, -1, 0)]


def plan_segment(_rng):
    return box() + [(1, 0, 0), (-1, 0, 0)]


def plan_duplicates(rng):
    return box() + [(1, 0, 0)] * 500 + [(a, b, c) for a, b, c in redundant(rng, [(0, 0), (1, 1), (-1, 1)], 200)]


def plan_huge(_rng):
    return box() + [(10**4, 10**4, MAXC), (-10**4, 10**4, MAXC), (10**4, -10**4, MAXC), (0, 10**4, MAXC - 1)]


def plan_near_parallel(rng):
    pts = random_polygon(rng, 5, 2 * 10**3, 5 * 10**3)
    out = box() + poly_halfplanes(pts)
    for k in range(200):
        a = 10**4
        b = k * 50
        m = min(a * x + b * y for x, y in pts)
        out.append((a, b, -m + rng.randint(1, 10**6)))
    return out


def plan_random_2e5(rng):
    pts = random_polygon(rng, 8, 2 * 10**3, 6 * 10**3)
    core = box() + poly_halfplanes(pts)
    return core + redundant(rng, pts, 200000 - len(core))


def plan_random_1000(rng):
    pts = random_polygon(rng, 8, 2 * 10**3, 6 * 10**3)
    core = box() + poly_halfplanes(pts)
    return core + redundant(rng, pts, 1000 - len(core))


def plan_regular_2000(rng):
    pts = regular(rng, 2000, 8 * 10**3)
    return box() + poly_halfplanes(pts)


def redundant_outside_circle(rng, r, count, ab=10**4):
    """半径 r の円の外側の半平面を count 本。多角形の頂点を全部見るより速い。"""
    out = []
    for _ in range(count):
        while True:
            a = rng.randint(-ab, ab)
            b = rng.randint(-ab, ab)
            if a or b:
                break
        c = (math.isqrt(a * a + b * b) + 1) * r + rng.randint(1, 10**6)
        out.append((a, b, min(c, MAXC)))
    return out


def plan_regular_2000_20000(rng):
    pts = regular(rng, 2000, 8 * 10**3)
    core = box() + poly_halfplanes(pts)
    return core + redundant_outside_circle(rng, 8 * 10**3, 20000 - len(core))


def plan_regular_500(rng):
    pts = regular(rng, 500, 8 * 10**3)
    return box() + poly_halfplanes(pts)


def plan_poly_100_1e5(rng):
    pts = regular(rng, 100, 6 * 10**3)
    core = box() + poly_halfplanes(pts)
    return core + redundant(rng, pts, 100000 - len(core))


def plan_origin_2e5(rng):
    out = box()
    for _ in range(200000 - len(out)):
        while True:
            a = rng.randint(-10**4, 10**4)
            b = rng.randint(-10**4, 10**4)
            if a or b:
                break
        out.append((a, b, rng.randint(0, 10**8)))
    return out


def plan_chopped_2e5(rng):
    out = box()
    for _ in range(200000 - len(out)):
        while True:
            a = rng.randint(-10**4, 10**4)
            b = rng.randint(-10**4, 10**4)
            if a or b:
                break
        out.append((a, b, rng.randint(-MAXC, MAXC)))
    return out


def plan_mix_1e4(rng):
    out = box()
    for _ in range(10000 - len(out)):
        while True:
            a = rng.randint(-100, 100)
            b = rng.randint(-100, 100)
            if a or b:
                break
        out.append((a, b, rng.randint(-10**6, 10**6)))
    return out


def plan_many_duplicates_2e5(rng):
    base = [(1, 0, 0), (-1, 0, 3000), (0, 1, 0), (0, -1, 5000), (1, 1, 100)]
    out = box()
    for k in range(200000 - len(out)):
        out.append(base[k % len(base)])
    return out


def plan_tiny_2e5(rng):
    pts = random_polygon(rng, 3, 2 * 10**3, 4 * 10**3)
    core = box() + poly_halfplanes(pts)
    return core + redundant(rng, pts, 200000 - len(core))


PLANS = [
    plan_box,
    plan_triangle,
    plan_empty,
    plan_point,
    plan_segment,
    plan_duplicates,
    plan_huge,
    plan_near_parallel,
    plan_random_2e5,
    plan_random_1000,
    plan_regular_2000,
    plan_regular_2000_20000,
    plan_regular_500,
    plan_poly_100_1e5,
    plan_origin_2e5,
    plan_chopped_2e5,
    plan_mix_1e4,
    plan_many_duplicates_2e5,
    plan_tiny_2e5,
]
COUNT = len(PLANS)


def small(rng):
    """crosscheck 用の小さい入力 (N <= 12)。"""
    out = box()
    for _ in range(rng.randint(0, 8)):
        while True:
            a = rng.randint(-10, 10)
            b = rng.randint(-10, 10)
            if a or b:
                break
        out.append((a, b, rng.randint(-10**6, 10**6)))
    return out


def main():
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    lines = small(rng) if seed >= 1000 else PLANS[seed % COUNT](rng)
    out = [str(len(lines))]
    out += [f"{a} {b} {c}" for a, b, c in lines]
    sys.stdout.write("\n".join(out) + "\n")


main()
