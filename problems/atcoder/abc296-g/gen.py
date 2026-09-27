"""abc296-g (Polygon and Points) の入力を作る。N と凸多角形の頂点 (反時計回り)、Q と Q 個の点を出す。答えは点ごとの IN、OUT、ON。

seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース、大きいケース。
角のケースは、座標が ±10^9 の三角形と正方形 (外積が 8 × 10^18 近くになる)、左右の辺が垂直な多角形
(頂点の並びの始まりを変えたもの)、細い三角形。
大きい多角形は、辺のベクトルを角度の順に並べる Valtr の方法で作る頂点 2 × 10^5 のもの、それを 2 倍に広げて
辺の上に格子点を作ったもの (頂点 10^5)、向きの違う原始ベクトルを全部並べて広げたもの (頂点 5 × 10^4)、
Valtr の多角形に左右の垂直な辺 (と上下の水平な辺) を足したもの、小さい多角形を大きく広げたもの (頂点 28)、
ランダムな三角形、座標が ±10^9 の正方形。頂点 2 × 10^5 で点が 1 つだけのものも入れる。
点は、頂点、辺の上の格子点 (ON)、辺を延ばした直線の上で辺の外の点 (OUT)、辺のすぐ近くの点、頂点 3 つの重心、
外接する長方形の中のランダム、全体のランダム、左端と右端の頂点と同じ x の点、を混ぜる。
入力が大きい (N = Q = 2 × 10^5 で 8 MB) ので、N と Q が両方とも最大なのは 1 ケースにする。
seed が 1000 以上なら、辺ごとに外積の符号を見る愚直解で解ける小さい入力を出す (pj testdata crosscheck 用)。
2000 以上なら、同じ愚直解でまだ解ける N, Q <= 300 の入力を出す。
"""

import functools
import math
import random
import sys

MAX_N = 2 * 10**5
MAX_Q = 2 * 10**5
LIM = 10**9

Point = tuple[int, int]

SAMPLES = [
    ([(0, 4), (-2, 2), (-1, 0), (3, 1)], [(-1, 3), (0, 2), (2, 0)]),
    ([(0, 0), (1, 0), (0, 1)], [(0, 0), (1, 0), (0, 1)]),
]
BIG_TRIANGLE = [(-LIM, -LIM), (LIM, -LIM), (-LIM, LIM)]
BIG_SQUARE = [(-LIM, -LIM), (LIM, -LIM), (LIM, LIM), (-LIM, LIM)]
HEXAGON = [(0, 0), (5, -3), (10, 0), (10, 4), (5, 7), (0, 4)]  # 左右の辺が垂直
THIN = [(-LIM, -LIM), (LIM, LIM - 1), (LIM - 1, LIM)]
FIXED = [
    (BIG_TRIANGLE, BIG_TRIANGLE + [(0, 0), (1, 0), (-1, 0), (0, 1), (-LIM, 0), (0, -LIM), (LIM, LIM), (LIM - 1, -LIM + 1),
                                   (-LIM + 1, LIM - 1), (-LIM + 1, LIM - 2), (LIM, LIM - 1)]),
    (BIG_SQUARE, BIG_SQUARE + [(0, 0), (LIM, 0), (-LIM, 0), (0, LIM), (0, -LIM), (LIM - 1, LIM - 1), (LIM, LIM - 1)]),
    (HEXAGON, HEXAGON + [(0, 2), (0, 5), (0, -1), (10, 2), (10, 5), (10, -1), (5, 0), (5, -3), (5, -4), (5, 7), (5, 8),
                         (1, 0), (9, 5), (11, 2), (-1, 2), (5, 2), (4, -2), (3, 6)]),
    (HEXAGON[3:] + HEXAGON[:3], [(0, 2), (0, 5), (10, 2), (10, 5), (10, 4), (0, 0), (-1, 4), (11, 0)]),
    (HEXAGON[5:] + HEXAGON[:5], [(0, 2), (0, 5), (10, 2), (10, 5), (10, 0), (0, 4), (5, 3)]),
    (THIN, THIN + [(0, 0), (1, 1), (0, 1), (1, 0), (LIM - 1, LIM - 1), (-LIM + 1, -LIM + 1), (LIM, LIM), (-LIM, -LIM + 1)]),
]
# (多角形の作り方, N の目安, Q, 点の出し方)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    ("valtr", MAX_N, MAX_Q, "mixed"),
    ("valtr_x2", 100000, 50000, "mixed"),
    ("primitive", 0, 50000, "mixed"),
    ("triangle", 3, 100000, "mixed"),
    ("square", 4, 50000, "mixed"),
    ("valtr_vertical", 10000, 50000, "mixed"),
    ("valtr_x2_axis", 50000, 20000, "boundary"),
    ("valtr", 1000, 1000, "mixed"),
    ("valtr_small", 30, 20000, "mixed"),
    ("valtr", MAX_N, 1, "mixed"),
]
KINDS = ["vertex", "edge", "extension", "near", "inside", "box", "far", "vertical"]


def cross(o: Point, a: Point, b: Point) -> int:
    return (a[0] - o[0]) * (b[1] - o[1]) - (a[1] - o[1]) * (b[0] - o[0])


def strict_hull(pts: list[Point]) -> list[Point]:
    """反時計回りの凸包。辺の途中の点は入れない。"""
    pts = sorted(set(pts))
    if len(pts) < 3:
        return pts
    lower: list[Point] = []
    upper: list[Point] = []
    for p in pts:
        while len(lower) >= 2 and cross(lower[-2], lower[-1], p) <= 0:
            lower.pop()
        lower.append(p)
    for p in reversed(pts):
        while len(upper) >= 2 and cross(upper[-2], upper[-1], p) <= 0:
            upper.pop()
        upper.append(p)
    return lower[:-1] + upper[:-1]


def by_angle(u: Point, v: Point) -> int:
    hu = 0 if u[1] > 0 or (u[1] == 0 and u[0] > 0) else 1
    hv = 0 if v[1] > 0 or (v[1] == 0 and v[0] > 0) else 1
    if hu != hv:
        return hu - hv
    c = u[0] * v[1] - u[1] * v[0]
    return -1 if c > 0 else 1 if c < 0 else 0


def from_vectors(vecs: list[Point]) -> list[Point]:
    """和が 0 のベクトルを角度の順に並べて繋ぐ。同じ向きのものは 1 本にまとめる。"""
    vecs = sorted(vecs, key=functools.cmp_to_key(by_angle))
    merged: list[Point] = []
    for v in vecs:
        if merged and by_angle(merged[-1], v) == 0:
            merged[-1] = (merged[-1][0] + v[0], merged[-1][1] + v[1])
        else:
            merged.append(v)
    out, x, y = [], 0, 0
    for dx, dy in merged:
        out.append((x, y))
        x, y = x + dx, y + dy
    assert (x, y) == (0, 0)
    return out


def valtr(rng: random.Random, n: int, r: int) -> list[Point]:
    """[0, r] の 2 乗に入る、頂点が n 以下のランダムな狭義凸多角形 (Valtr の方法)。"""

    def components() -> list[int]:
        vals = sorted(rng.sample(range(r + 1), n))
        lo, hi = vals[0], vals[-1]
        a = b = lo
        out = []
        for v in vals[1:-1]:
            if rng.random() < 0.5:
                out.append(v - a)
                a = v
            else:
                out.append(b - v)
                b = v
        return out + [hi - a, b - hi]

    xs, ys = components(), components()
    rng.shuffle(ys)
    return from_vectors(list(zip(xs, ys)))


def primitive(r: int) -> list[Point]:
    """max(|dx|, |dy|) <= r の原始ベクトルを全部、角度の順に繋いだ多角形 (中心対称)。"""
    return from_vectors([
        (dx, dy) for dx in range(-r, r + 1) for dy in range(-r, r + 1) if math.gcd(dx, dy) == 1
    ])


def fit(poly: list[Point], rng: random.Random, scale: int = 1) -> list[Point]:
    """scale 倍して [-10^9, 10^9] の 2 乗に収まる所へランダムに動かし、頂点の並びの始まりを混ぜる。"""
    poly = [(x * scale, y * scale) for x, y in poly]
    x0, x1 = min(p[0] for p in poly), max(p[0] for p in poly)
    y0, y1 = min(p[1] for p in poly), max(p[1] for p in poly)
    assert x1 - x0 <= 2 * LIM and y1 - y0 <= 2 * LIM
    sx = rng.randint(-LIM - x0, LIM - x1)
    sy = rng.randint(-LIM - y0, LIM - y1)
    poly = [(x + sx, y + sy) for x, y in poly]
    k = rng.randrange(len(poly))
    return poly[k:] + poly[:k]


def polygon(rng: random.Random, how: str, n: int) -> list[Point]:
    if how == "valtr":
        return fit(valtr(rng, n, 2 * LIM), rng)
    if how == "valtr_small":
        return fit(valtr(rng, n, 100), rng, scale=rng.randint(1, 10**7))
    if how == "valtr_x2":
        return fit(valtr(rng, n, LIM), rng, scale=2)
    if how in ("valtr_vertical", "valtr_x2_axis"):
        # Valtr の辺に、垂直な辺 (と valtr_x2_axis では水平な辺も) を左右 (上下) に 1 本ずつ足す。
        base = valtr(rng, n, LIM // 2)
        vecs = [(b[0] - a[0], b[1] - a[1]) for a, b in zip(base, base[1:] + base[:1])]
        h = rng.randint(1, LIM // 4)
        vecs += [(0, h), (0, -h)]
        if how == "valtr_x2_axis":
            w = rng.randint(1, LIM // 4)
            vecs += [(w, 0), (-w, 0)]
        return fit(from_vectors(vecs), rng, scale=2 if how == "valtr_x2_axis" else 1)
    if how == "primitive":
        poly = primitive(143)
        span = max(max(p[0] for p in poly) - min(p[0] for p in poly), max(p[1] for p in poly) - min(p[1] for p in poly))
        return fit(poly, rng, scale=2 * LIM // span)
    if how == "triangle":
        hull = strict_hull([(rng.randint(-LIM, LIM), rng.randint(-LIM, LIM)) for _ in range(3)])
        return fit(hull if len(hull) == 3 else BIG_TRIANGLE, rng)
    assert how == "square"
    return BIG_SQUARE


def clamp(p: Point) -> Point:
    return (max(-LIM, min(LIM, p[0])), max(-LIM, min(LIM, p[1])))


def bounding_box(poly: list[Point]) -> tuple[int, int, int, int, list[int], list[int]]:
    """外接する長方形と、左端と右端の頂点の y の一覧。"""
    x0, x1 = min(p[0] for p in poly), max(p[0] for p in poly)
    y0, y1 = min(p[1] for p in poly), max(p[1] for p in poly)
    return x0, x1, y0, y1, [p[1] for p in poly if p[0] == x0], [p[1] for p in poly if p[0] == x1]


def query(rng: random.Random, poly: list[Point], box, kind: str) -> Point:
    n = len(poly)
    i = rng.randrange(n)
    a, b = poly[i], poly[(i + 1) % n]
    dx, dy = b[0] - a[0], b[1] - a[1]
    g = math.gcd(dx, dy)
    x0, x1, y0, y1, left, right = box
    if kind == "vertex":
        return a
    if kind == "edge":
        if g < 2:
            return a
        t = rng.randint(1, g - 1)
        return (a[0] + dx // g * t, a[1] + dy // g * t)
    if kind == "extension":
        # 辺を延ばした直線の上で、辺の外にある格子点。範囲の外に出るなら頂点にする。
        t = rng.choice([1, 2, rng.randint(1, 100)])
        p = (b[0] + dx // g * t, b[1] + dy // g * t) if rng.random() < 0.5 else (a[0] - dx // g * t, a[1] - dy // g * t)
        return p if clamp(p) == p else a
    if kind == "near":
        s = rng.random()
        return clamp((a[0] + round(dx * s) + rng.randint(-2, 2), a[1] + round(dy * s) + rng.randint(-2, 2)))
    if kind == "inside":
        p, q, r = (poly[rng.randrange(n)] for _ in range(3))
        return ((p[0] + q[0] + r[0]) // 3, (p[1] + q[1] + r[1]) // 3)
    if kind == "box":
        return (rng.randint(x0, x1), rng.randint(y0, y1))
    if kind == "far":
        return (rng.randint(-LIM, LIM), rng.randint(-LIM, LIM))
    assert kind == "vertical"
    x, ys = (x0, left) if rng.random() < 0.5 else (x1, right)
    y = rng.choice([rng.randint(y0, y1), min(ys), max(ys), min(ys) - 1, max(ys) + 1, (min(ys) + max(ys)) // 2])
    return clamp((x, y))


def queries(rng: random.Random, poly: list[Point], q: int, how: str) -> list[Point]:
    kinds = ["vertex", "edge", "edge", "extension", "near", "vertical"] if how == "boundary" else KINDS
    box = bounding_box(poly)
    return [query(rng, poly, box, rng.choice(kinds)) for _ in range(q)]


def small_case(rng: random.Random, seed: int) -> tuple[list[Point], list[Point]]:
    if seed >= 2000:
        n = rng.randint(3, 300)
        poly = fit(valtr(rng, n, rng.choice([n + 10, 1000, 2 * LIM])), rng) if rng.random() < 0.7 else []
        if len(poly) < 3:
            poly = fit(valtr(rng, max(n, 3), 2 * LIM), rng)
        return poly, queries(rng, poly, rng.randint(1, 300), rng.choice(["mixed", "boundary"]))
    while True:
        r = rng.choice([3, 6, 20, LIM])
        pts = [(rng.randint(-r, r), rng.randint(-r, r)) for _ in range(rng.randint(3, 12))]
        poly = strict_hull(pts)
        if len(poly) >= 3:
            break
    k = rng.randrange(len(poly))
    poly = poly[k:] + poly[:k]
    return poly, queries(rng, poly, rng.randint(1, 30), rng.choice(["mixed", "boundary"]))


def case_for(seed: int, rng: random.Random) -> tuple[list[Point], list[Point]]:
    if seed >= 1000:
        return small_case(rng, seed)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    how, n, q, qhow = PLANS[seed - len(FIXED)]
    poly = polygon(rng, how, n)
    return poly, queries(rng, poly, q, qhow)


def main() -> None:
    seed = int(sys.argv[1])
    poly, qs = case_for(seed, random.Random(seed))
    n = len(poly)
    assert 3 <= n <= MAX_N and 1 <= len(qs) <= MAX_Q
    assert all(-LIM <= v <= LIM for p in poly + qs for v in p)
    # 狭義凸で反時計回りか確かめる (どの 3 頂点も左に曲がり、1 周で 1 回だけ回る)。
    assert all(cross(poly[i - 2], poly[i - 1], poly[i]) > 0 for i in range(n))
    assert sum(by_angle((poly[i][0] - poly[i - 1][0], poly[i][1] - poly[i - 1][1]),
                        (poly[(i + 1) % n][0] - poly[i][0], poly[(i + 1) % n][1] - poly[i][1])) > 0 for i in range(n)) == 1
    out = [str(n)] + [f"{x} {y}" for x, y in poly] + [str(len(qs))] + [f"{x} {y}" for x, y in qs]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
