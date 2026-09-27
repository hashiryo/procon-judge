"""past202004-n (Building Construction) の入力を作る。N Q、N 個の正方形 xmin ymin D C、Q 個の点 A B を出す。

答えは点ごとの、その点を含む (辺の上も含む) 正方形の C の和。
seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、N = 5 × 10^4 と Q = 10^5 のケース。
角のケースは、点が正方形の角や辺の上とその 1 つ外にあるもの、座標が ±10^9 の端のもの、C が全部 0 のもの、
C = 10^9 の同じ正方形が 5 × 10^4 個重なって答えが 5 × 10^13 になるもの。
大きいケースは、座標が全体に散ったもの、座標が狭い範囲に固まって辺の上の点と同じ点だらけのもの、
点が全部同じもの、点が 1 本の縦線に並んだもの、点が格子に並んだもの、正方形の角と辺を点に合わせたもの、
一辺が 10^9 の正方形ばかりのもの、点の雲を横切る中くらいの正方形ばかりのもの (KDTree は、正方形の辺が
横切る節をたどるので遅い) である。座標が長くて入力が大きくなるので、大きさの要らないもの (C が 0、答えが
5 × 10^13、点が全部同じ、縦線、一辺が 10^9) は N か Q を減らしてある。
seed が 1000 以上なら、点ごとに正方形を全部見る愚直解で解ける小さい入力を出す (pj testdata crosscheck 用)。
2000 以上なら、N と Q を 3000 までにした入力を出す。
"""

import random
import sys

MAX_N = 5 * 10**4
MAX_Q = 10**5
LIM = 10**9  # 座標の絶対値と D と C の上限

Square = tuple[int, int, int, int]
Point = tuple[int, int]

SAMPLES: list[tuple[list[Square], list[Point]]] = [
    ([(1, 3, 6, 10), (3, 6, 6, 20)], [(4, 7), (-1, -1), (1, 4), (7, 13)]),
    ([(-3, 5, 4, 100), (1, 9, 7, 30)], [(1, 9), (1, 8), (8, 10)]),
    (
        [(17, 2, 17, LIM), (7, 12, 12, LIM), (2, 12, 8, LIM), (2, 12, 2, LIM), (3, 9, 16, LIM),
         (8, 13, 15, LIM), (8, 1, 3, LIM), (15, 9, 17, LIM), (16, 5, 5, LIM), (13, 12, 9, LIM)],
        [(17, 3), (4, 10), (1, 9), (5, 3), (17, 12), (14, 19), (19, 17), (17, 11), (16, 17), (12, 16)],
    ),
]


def around(x: int, y: int, d: int) -> list[Point]:
    """正方形 [x, x + d] x [y, y + d] の角、辺の上、その 1 つ外の点 (範囲内のものだけ)。"""
    xs, ys = [x - 1, x, x + 1, x + d // 2, x + d - 1, x + d, x + d + 1], [y - 1, y, y + 1, y + d // 2, y + d - 1, y + d, y + d + 1]
    return [(a, b) for a in xs for b in ys if -LIM <= a <= LIM and -LIM <= b <= LIM]


FIXED: list[tuple[list[Square], list[Point]]] = [
    ([(0, 0, 1, 7)], [(1, 1)]),  # 角の点
    ([(-5, 3, 10, 123)], around(-5, 3, 10)),
    ([(-LIM, -LIM, LIM, LIM), (LIM, LIM, LIM, 1), (0, -LIM, LIM, 2)],
     around(-LIM, -LIM, LIM) + [(LIM, LIM), (0, 0), (-LIM, LIM), (LIM, -LIM)]),
]
# (作り方, N, Q)。本番のケースのうち FIXED のあとに並べる。
PLANS = [
    ("zero_c", 1000, 1000),
    ("overflow", MAX_N, 10**4),
    ("wide", MAX_N, MAX_Q),
    ("narrow", MAX_N, MAX_Q),
    ("same_point", MAX_N, 10**4),
    ("vertical", MAX_N, 5 * 10**4),
    ("grid", MAX_N, MAX_Q),
    ("aligned", MAX_N, MAX_Q),
    ("huge", MAX_N, 5 * 10**4),
    ("crossing", MAX_N, MAX_Q),
]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)


def random_square(rng: random.Random, lo: int, hi: int, max_d: int) -> Square:
    x, y = rng.randint(lo, hi), rng.randint(lo, hi)
    return x, y, rng.randint(1, max_d), rng.randint(0, LIM)


def plan_case(rng: random.Random, how: str, n: int, q: int) -> tuple[list[Square], list[Point]]:
    pts = lambda lo, hi: [(rng.randint(lo, hi), rng.randint(lo, hi)) for _ in range(q)]  # noqa: E731
    if how == "zero_c":
        sq = [random_square(rng, -LIM, LIM, LIM)[:3] + (0,) for _ in range(n)]
        return sq, pts(-LIM, LIM)
    if how == "overflow":  # 左下の 4 分の 1 を覆う同じ正方形が n 個
        return [(-LIM, -LIM, LIM, LIM)] * n, pts(-LIM, LIM)
    if how == "wide":
        return [random_square(rng, -LIM, LIM, LIM) for _ in range(n)], pts(-LIM, LIM)
    if how == "narrow":  # 座標を [-100, 100] に固める
        return [random_square(rng, -100, 100, 60) for _ in range(n)], pts(-100, 100)
    if how == "same_point":
        a, b = rng.randint(-10, 10), rng.randint(-10, 10)
        return [random_square(rng, -40, 20, 40) for _ in range(n)], [(a, b)] * q
    if how == "vertical":
        return [random_square(rng, -1000, 1000, 1000) for _ in range(n)], [(0, rng.randint(-2000, 2000)) for _ in range(q)]
    if how == "grid":  # 316 x 316 の格子の点
        side, step = 316, 3
        grid = [(i * step, j * step) for i in range(side) for j in range(side)][:q]
        rng.shuffle(grid)
        return [random_square(rng, -50, side * step, 300) for _ in range(n)], grid
    if how == "aligned":  # 正方形の角と辺を、点の座標に合わせる
        ps = pts(-10**6, 10**6)
        sq = []
        for _ in range(n):
            (a, b), (c, e) = rng.choice(ps), rng.choice(ps)
            d = max(1, abs(c - a), abs(e - b))
            corner = rng.randrange(4)
            x = a if corner % 2 == 0 else a - d
            y = b if corner < 2 else b - d
            sq.append((x, y, d, rng.randint(0, LIM)))
        return sq, ps
    if how == "huge":
        return [(rng.randint(-LIM, 0), rng.randint(-LIM, 0), LIM, rng.randint(0, LIM)) for _ in range(n)], pts(-LIM, LIM)
    assert how == "crossing"  # 点は [0, 10^6]^2 に一様、正方形は一辺 3 x 10^5 から 7 x 10^5 で雲を横切る
    sq = []
    for _ in range(n):
        d = rng.randint(3 * 10**5, 7 * 10**5)
        sq.append((rng.randint(-d // 2, 10**6 - d // 2), rng.randint(-d // 2, 10**6 - d // 2), d, rng.randint(0, LIM)))
    return sq, pts(0, 10**6)


def small_case(rng: random.Random, max_n: int, max_q: int) -> tuple[list[Square], list[Point]]:
    """愚直解で解ける大きさ。座標の範囲を変え、辺の上の点を多めにする。"""
    n, q = rng.randint(1, max_n), rng.randint(1, max_q)
    span = rng.choice([5, 20, 1000, LIM])
    max_d = rng.choice([1, 3, span, LIM])
    sq = [random_square(rng, -span, span, max_d) for _ in range(n)]
    if rng.random() < 0.3:
        sq = [(x, y, d, 0 if rng.random() < 0.5 else LIM) for x, y, d, _ in sq]
    ps = []
    for _ in range(q):
        if rng.random() < 0.5:
            ps.append(rng.choice(around(*rng.choice(sq)[:3])))
        else:
            ps.append((rng.randint(-span, span), rng.randint(-span, span)))
    return sq, ps


def case_for(seed: int, rng: random.Random) -> tuple[list[Square], list[Point]]:
    if seed >= 2000:
        return small_case(rng, 3000, 3000)
    if seed >= 1000:
        return small_case(rng, 30, 30)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    return plan_case(rng, *PLANS[seed - len(FIXED)])


def main() -> None:
    seed = int(sys.argv[1])
    sq, ps = case_for(seed, random.Random(seed))
    n, q = len(sq), len(ps)
    assert 1 <= n <= MAX_N and 1 <= q <= MAX_Q
    assert all(-LIM <= x <= LIM and -LIM <= y <= LIM and 1 <= d <= LIM and 0 <= c <= LIM for x, y, d, c in sq)
    assert all(-LIM <= a <= LIM and -LIM <= b <= LIM for a, b in ps)
    out = [f"{n} {q}"] + [f"{x} {y} {d} {c}" for x, y, d, c in sq] + [f"{a} {b}" for a, b in ps]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
