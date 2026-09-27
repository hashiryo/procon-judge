"""abc244-h (Linear Maximization) の入力を作る。Q と、Q 行の X Y A B を出す。点は全部違う。

seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース、Q = 2000 の中くらいのケース、
大きいケースの順に並べる。
角のケースは、Q = 1、答えが ±2 * 10^18 に届くもの、(A, B) = (0, 0)、B = 0 (lib.cpp は B = 0 を
別に扱う)、一直線に並んだ点、X が同じ点など。
大きいケースは、全部の点が凸包に載る凸多角形 (直線の集合が小さくならないので遅い) をランダムな順と
角度の順で入れるもの、外へ広がる渦巻き (新しい点が古い凸包の点を消していく)、ランダム、狭い箱の中の点、
X が 3 通りしかない点 (傾きの同じ直線)、四隅に寄った点。凸多角形の辺に垂直な (A, B) では、辺の両端が
同点になる。データが大きくなりすぎないよう、凸多角形とランダム以外は Q を 2 * 10^4 から 10^5 にする。
seed が 1000 以上なら、質問ごとに全部の点を見る愚直解で解ける小さい入力を出す (pj testdata crosscheck 用)。
2000 以上なら、同じ愚直解ですぐ解ける Q = 3000 までの入力を出す。愚直解は Q = 2 * 10^5 でも数秒で
終わるので、本番のケースもそのまま突き合わせられる。
"""

import math
import random
import sys

MAX_Q = 2 * 10**5
MAX_V = 10**9

SAMPLES = [
    [(1, 0, -1, -1), (0, 1, 2, 0), (-1, 0, 1, 1), (0, -1, 1, -2)],
    [
        (-1, 4, -8, -2), (9, -9, -7, 7), (4, 1, 6, 7), (-4, -1, -4, -5), (-9, 3, -2, -6),
        (-1, 0, -8, 5), (-8, -5, 0, 0), (8, 3, 0, -4), (2, -5, 2, 5),
    ],
]
M = MAX_V
FIXED = [
    [(0, 0, 0, 0)],
    [(M, M, M, M)],  # 2 * 10^18
    [(-M, -M, M, M)],  # -2 * 10^18
    [(M, -M, -M, M), (-M, M, -M, M)],
    [(M, M, M, 0), (-M, M, -M, 0), (-M, -M, 0, M), (M, -M, 0, -M), (0, 0, M, -M), (1, 2, 0, 0)],
    [(i, i, 1, -1) for i in range(-5, 6)] + [(6, 6, 1, 1), (7, 7, -1, 0)],  # 一直線
    [(5, y, a, b) for y, a, b in [(3, 2, 0), (-7, -1, 0), (10, 0, 1), (0, 0, -1), (100, 3, 5), (-100, 3, -5)]],
]


def primitive_vectors(count: int) -> list[tuple[int, int]]:
    """上半平面 (角度が 0 以上 π 未満) の原始ベクトルを、短い順に count 個。"""
    r = int(math.sqrt(count)) + 10
    vecs = [(a, b) for a in range(-r, r + 1) for b in range(r + 1)
            if (b > 0 or a > 0) and math.gcd(a, b) == 1]
    vecs.sort(key=lambda v: (v[0] ** 2 + v[1] ** 2, math.atan2(v[1], v[0])))
    assert len(vecs) >= count
    return vecs[:count]


def convex_polygon(n: int) -> tuple[list[tuple[int, int]], list[tuple[int, int]]]:
    """頂点が n 個 (偶数) の凸多角形。向きの違う原始ベクトルとその逆向きを角度の順に足していく。
    頂点を反時計回りの順で返し、あわせて各辺のベクトルも返す。"""
    half = primitive_vectors(n // 2)
    edges = sorted(half + [(-a, -b) for a, b in half], key=lambda v: math.atan2(v[1], v[0]))
    x = y = 0
    points = []
    for a, b in edges:
        points.append((x, y))
        x, y = x + a, y + b
    assert (x, y) == (0, 0)
    cx = (max(p[0] for p in points) + min(p[0] for p in points)) // 2
    cy = (max(p[1] for p in points) + min(p[1] for p in points)) // 2
    return [(px - cx, py - cy) for px, py in points], edges


def distinct_points(rng: random.Random, n: int, pick) -> list[tuple[int, int]]:
    seen, out = set(), []
    while len(out) < n:
        p = pick()
        if p not in seen:
            seen.add(p)
            out.append(p)
    return out


def query(rng: random.Random, bound: int) -> tuple[int, int]:
    return rng.randint(-bound, bound), rng.randint(-bound, bound)


def join(points: list[tuple[int, int]], queries: list[tuple[int, int]]) -> list[tuple[int, int, int, int]]:
    return [(x, y, a, b) for (x, y), (a, b) in zip(points, queries)]


def convex_case(rng: random.Random, n: int, order: str) -> list[tuple[int, int, int, int]]:
    points, edges = convex_polygon(n)
    queries = []
    for _ in range(n):
        if rng.random() < 0.2:
            # 辺に垂直な向き。辺の両端がどちらも入っていれば同点になる。
            a, b = rng.choice(edges)
            k = rng.randint(1, 1000)
            queries.append((k * b, -k * a))
        else:
            queries.append(query(rng, 10**6))
    if order == "random":
        rng.shuffle(points)
    return join(points, queries)


def spiral_case(rng: random.Random, n: int) -> list[tuple[int, int, int, int]]:
    """半径を少しずつ大きくしながら、ランダムな角度に点を置く。新しい点は前の点をほぼ全部囲む。"""
    def pick_at(i: int) -> tuple[int, int]:
        r = (i + 1) / n * (MAX_V - 10)
        t = rng.random() * 2 * math.pi
        return round(r * math.cos(t)), round(r * math.sin(t))

    seen, points = set(), []
    for i in range(n):
        p = pick_at(i)
        while p in seen:
            p = pick_at(i)
        seen.add(p)
        points.append(p)
    return join(points, [query(rng, MAX_V) for _ in range(n)])


def big_case(rng: random.Random, shape: str) -> list[tuple[int, int, int, int]]:
    if shape == "convex_random":
        return convex_case(rng, MAX_Q, "random")
    if shape == "convex_sorted":
        return convex_case(rng, 5 * 10**4, "sorted")
    if shape == "spiral":
        return spiral_case(rng, 5 * 10**4)
    if shape == "random":
        n = 5 * 10**4
        points = distinct_points(rng, n, lambda: query(rng, MAX_V))
        return join(points, [query(rng, MAX_V) for _ in range(n)])
    if shape == "box":
        n = 10**5
        points = distinct_points(rng, n, lambda: query(rng, 400))
        return join(points, [query(rng, 1000) for _ in range(n)])
    if shape == "three_x":
        n = 2 * 10**4
        points = distinct_points(rng, n, lambda: (rng.choice([-MAX_V, 0, MAX_V]), rng.randint(-MAX_V, MAX_V)))
        queries = [(rng.randint(-MAX_V, MAX_V), rng.choice([0, 1, -1, rng.randint(-MAX_V, MAX_V)])) for _ in range(n)]
        return join(points, queries)
    assert shape == "corners"
    n = 2 * 10**4

    def near_corner() -> tuple[int, int]:
        return rng.choice([-1, 1]) * (MAX_V - rng.randrange(1000)), rng.choice([-1, 1]) * (MAX_V - rng.randrange(1000))

    points = distinct_points(rng, n, near_corner)
    return join(points, [near_corner() for _ in range(n)])


BIG = ["convex_random", "convex_sorted", "spiral", "random", "box", "three_x", "corners"]


def medium_case(rng: random.Random, shape: str, n: int) -> list[tuple[int, int, int, int]]:
    if shape == "box":
        points = distinct_points(rng, n, lambda: query(rng, 30))
        return join(points, [query(rng, 3) for _ in range(n)])
    if shape == "b_zero":
        points = distinct_points(rng, n, lambda: query(rng, MAX_V))
        return join(points, [(rng.randint(-MAX_V, MAX_V), 0) for _ in range(n)])
    if shape == "a_zero":
        points = distinct_points(rng, n, lambda: query(rng, MAX_V))
        return join(points, [(0, rng.randint(-MAX_V, MAX_V)) for _ in range(n)])
    if shape == "convex":
        return convex_case(rng, n - n % 2, rng.choice(["random", "sorted"]))
    if shape == "spiral":
        return spiral_case(rng, n)
    assert shape == "random"
    points = distinct_points(rng, n, lambda: query(rng, MAX_V))
    return join(points, [query(rng, MAX_V) for _ in range(n)])


MEDIUM = ["box", "b_zero", "a_zero", "convex"]
COUNT = len(SAMPLES) + len(FIXED) + len(MEDIUM) + len(BIG)


def small_case(rng: random.Random, n: int) -> list[tuple[int, int, int, int]]:
    """狭い箱の値と ±10^9 の近くの値を混ぜ、同点や 0 の向きを起こしやすくする。"""
    def value(bound_small: int) -> int:
        return rng.choice([
            rng.randint(-bound_small, bound_small),
            rng.choice([-1, 1]) * (MAX_V - rng.randrange(3)),
            rng.randint(-MAX_V, MAX_V),
        ])

    box = rng.choice([2, 5, 1000])
    points = distinct_points(rng, n, lambda: (value(box), value(box)))
    queries = [(value(3), value(3)) for _ in range(n)]
    return join(points, queries)


def case_for(seed: int, rng: random.Random) -> list[tuple[int, int, int, int]]:
    if seed >= 2000:
        return medium_case(rng, rng.choice(["box", "b_zero", "a_zero", "convex", "spiral", "random"]),
                           rng.randint(100, 3000))
    if seed >= 1000:
        return small_case(rng, rng.randint(1, 60))
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    seed -= len(FIXED)
    if seed < len(MEDIUM):
        return medium_case(rng, MEDIUM[seed], 2000)
    return big_case(rng, BIG[seed - len(MEDIUM)])


def main() -> None:
    seed = int(sys.argv[1])
    rows = case_for(seed, random.Random(seed))
    assert 1 <= len(rows) <= MAX_Q
    assert all(abs(v) <= MAX_V for row in rows for v in row)
    assert len({(x, y) for x, y, _, _ in rows}) == len(rows)
    sys.stdout.write("\n".join([str(len(rows))] + [f"{x} {y} {a} {b}" for x, y, a, b in rows]) + "\n")


if __name__ == "__main__":
    main()
