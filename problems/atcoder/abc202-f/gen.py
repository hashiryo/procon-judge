"""abc202-f (Integer Convex Hull) の入力を作る。N と N 個の点 X_i Y_i を出す。どの 3 点も一直線に並ばない。

seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース (N = 3 と 4、座標が端の値)、N = 80 のいろいろな点の置き方。
置き方は、ランダム、座標が全部偶数 (どの凸包の面積も整数なので、答えは 2^N - 1 - N - N(N-1)/2)、
狭い範囲、放物線の上 (全部の点が凸な位置で、答えは同じく 2^N - 1 - N - N(N-1)/2)、
円の近くの格子点の凸包の頂点 (全部の点が凸な位置で、面積の偶奇は混ざる)、凸な位置の点と内側の点、
同じ x の点が 2 個ずつ、座標の端の近く、円の近くのランダムな点、座標の偶奇が 3 通りだけ。
提出は x で並べて左端から上側と下側をたどるので、左端と右端に同じ x の点が 2 個ある形も入れる。
seed が 1000 以上なら、部分集合を全部試す愚直解で解ける N = 12 までの入力を出す (pj testdata crosscheck 用)。
2000 以上なら、愚直解でまだ解ける N = 20 までの入力を出す。
"""

import random
import sys

MAX_N = 80
MAX_C = 10**4

SAMPLES = [
    [(0, 0), (1, 2), (0, 1), (1, 0)],
    [
        (-5255, 7890), (5823, 7526), (5485, -113), (7302, 5708), (9149, 2722), (4904, -3918), (8566, -3267),
        (-3759, 2474), (-7286, -1043), (-1230, 1780), (3377, -7044), (-2596, -6003), (5813, -9452),
        (-9889, -7423), (2377, 1811), (5351, 4551), (-1354, -9611), (4244, 1958), (8864, -9889),
        (507, -8923), (6948, -5016), (-6139, 2769), (4103, 9241),
    ],
]
FIXED = [
    [(0, 0), (1, 0), (0, 1)],  # 面積 1/2。答えは 0
    [(0, 0), (2, 0), (0, 2)],  # 面積 2。答えは 1
    [(-MAX_C, -MAX_C), (MAX_C, -MAX_C), (MAX_C, MAX_C)],
    [(-MAX_C, -MAX_C), (MAX_C, MAX_C - 1), (MAX_C - 1, MAX_C)],
    [(0, 0), (4, 0), (0, 4), (1, 1)],  # 三角形の中に 1 点。中の点を選んでも選ばなくても数える
    [(-MAX_C, 0), (-MAX_C, 1), (MAX_C, 0), (MAX_C, -1)],  # 左端と右端に同じ x の点
]
# N = 80 の置き方。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    "random",
    "random",
    "even",
    "small_box",
    "parabola",
    "convex_circle",
    "convex_inner",
    "vertical_pairs",
    "corners",
    "circle",
    "three_classes",
    "same_x_ends",
    "random_40",
]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)


def collinear(p: tuple[int, int], q: tuple[int, int], r: tuple[int, int]) -> bool:
    return (q[0] - p[0]) * (r[1] - p[1]) - (q[1] - p[1]) * (r[0] - p[0]) == 0


def add_points(rng: random.Random, pts: list[tuple[int, int]], n: int, pick) -> None:
    """pick() で出した点を、今ある点のどの 2 点とも一直線に並ばないものだけ足して n 個にする。"""
    tries = 0
    while len(pts) < n:
        tries += 1
        assert tries < 10**6, "点が置けません"
        c = pick()
        if c in pts or any(collinear(pts[i], pts[j], c) for i in range(len(pts)) for j in range(i)):
            continue
        pts.append(c)


def uniform(rng: random.Random, lo: int, hi: int):
    return lambda: (rng.randint(lo, hi), rng.randint(lo, hi))


def points_for(plan: str, rng: random.Random, n: int) -> list[tuple[int, int]]:
    pts: list[tuple[int, int]] = []
    if plan == "random":
        add_points(rng, pts, n, uniform(rng, -MAX_C, MAX_C))
    elif plan == "random_40":
        add_points(rng, pts, 40, uniform(rng, -MAX_C, MAX_C))
    elif plan == "even":
        add_points(rng, pts, n, lambda: (2 * rng.randint(-MAX_C // 2, MAX_C // 2), 2 * rng.randint(-MAX_C // 2, MAX_C // 2)))
    elif plan == "small_box":
        add_points(rng, pts, n, uniform(rng, -45, 45))
    elif plan == "parabola":
        # (249x, 5x^2 - 4000)。狭義に凸な関数の上なので、どの 3 点も一直線に並ばない。
        # 3 点の三角形の面積の 2 倍は 249 * 5 * (x1 - x2)(x2 - x3)(x3 - x1) で偶数なので、答えは 2^N - 1 - N - N(N-1)/2。
        pts = [(249 * x, 5 * x * x - 4000) for x in range(-(n // 2), n - n // 2)]
    elif plan == "convex_circle":
        # 円の近くの格子点の凸包の頂点 (3 点が並ぶものは除く) から n 個選ぶ。全部の点が凸な位置にあり、面積の偶奇は混ざる。
        import math

        cand = set()
        while len(cand) < 3000:
            t = rng.random() * 2 * math.pi
            cand.add((round(MAX_C * math.cos(t)), round(MAX_C * math.sin(t))))
        cand = sorted(cand)
        hull: list[tuple[int, int]] = []
        for part in (cand, cand[::-1]):
            start = len(hull)
            for c in part:
                while len(hull) >= start + 2 and (
                    (hull[-1][0] - hull[-2][0]) * (c[1] - hull[-2][1]) - (hull[-1][1] - hull[-2][1]) * (c[0] - hull[-2][0]) <= 0
                ):
                    hull.pop()
                hull.append(c)
            hull.pop()
        pts = rng.sample(hull, n)
    elif plan == "convex_inner":
        # 上に凸な放物線と下に凸な放物線で囲んだ凸な領域の縁に 30 点 (凸な位置)、その内側に 50 点。
        def rim() -> tuple[int, int]:
            x = rng.randint(-40, 40)
            return (200 * x, (5 * x * x - 9000) if rng.random() < 0.5 else (9000 - 5 * x * x))

        add_points(rng, pts, 30, rim)
        add_points(rng, pts, n, lambda: (rng.randint(-4000, 4000), rng.randint(-1000, 1000)))
    elif plan == "vertical_pairs":
        xs = rng.sample(range(-MAX_C, MAX_C + 1), n // 2)
        for x in xs:
            add_points(rng, pts, len(pts) + 1, lambda: (x, rng.randint(-MAX_C, MAX_C)))
            add_points(rng, pts, len(pts) + 1, lambda: (x, rng.randint(-MAX_C, MAX_C)))
    elif plan == "corners":
        def pick() -> tuple[int, int]:
            sx, sy = rng.choice([-1, 1]), rng.choice([-1, 1])
            return (sx * (MAX_C - rng.randint(0, 60)), sy * (MAX_C - rng.randint(0, 60)))

        add_points(rng, pts, n, pick)
    elif plan == "circle":
        import math

        def pick() -> tuple[int, int]:
            t = rng.random() * 2 * math.pi
            r = MAX_C - rng.randint(0, 3)
            return (round(r * math.cos(t)), round(r * math.sin(t)))

        add_points(rng, pts, n, pick)
    elif plan == "three_classes":
        # (x mod 2, y mod 2) が (1, 1) の点を使わない。三角形の面積の偶奇が混ざる。
        def pick() -> tuple[int, int]:
            while True:
                p = (rng.randint(-MAX_C, MAX_C), rng.randint(-MAX_C, MAX_C))
                if p[0] % 2 == 0 or p[1] % 2 == 0:
                    return p

        add_points(rng, pts, n, pick)
    else:
        assert plan == "same_x_ends"
        pts = [(-MAX_C, -3), (-MAX_C, 5000), (MAX_C, 7), (MAX_C, -6000)]
        add_points(rng, pts, n, uniform(rng, -MAX_C + 1, MAX_C - 1))
    rng.shuffle(pts)
    return pts


def small_points(rng: random.Random, n: int) -> list[tuple[int, int]]:
    """愚直解用。座標の範囲と置き方を混ぜる。"""
    kind = rng.randrange(5)
    pts: list[tuple[int, int]] = []
    if kind == 0:
        add_points(rng, pts, n, uniform(rng, -MAX_C, MAX_C))
    elif kind == 1:
        half = 6 if n <= 12 else 15  # 13 x 13 の格子には、3 点が並ばないように 13 点以上を置きにくい
        add_points(rng, pts, n, uniform(rng, -half, half))
    elif kind == 2:
        add_points(rng, pts, n, lambda: (2 * rng.randint(-50, 50), 2 * rng.randint(-50, 50)))
    elif kind == 3:
        x0 = rng.randint(-MAX_C, MAX_C - 30)
        pts = [(x0 + x, x * x - 400) for x in rng.sample(range(0, 30), n)]
    else:
        pts = [(-MAX_C, -1), (-MAX_C, 2), (MAX_C, 3), (MAX_C, -4)][: min(n, 4)]
        add_points(rng, pts, n, uniform(rng, -30, 30))
    rng.shuffle(pts)
    return pts


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 1000:
        pts = small_points(rng, rng.randint(13, 20) if seed >= 2000 else rng.randint(3, 12))
    elif seed < len(SAMPLES):
        pts = SAMPLES[seed]
    elif seed < len(SAMPLES) + len(FIXED):
        pts = FIXED[seed - len(SAMPLES)]
    else:
        pts = points_for(PLANS[seed - len(SAMPLES) - len(FIXED)], rng, MAX_N)
    n = len(pts)
    assert 3 <= n <= MAX_N and all(abs(x) <= MAX_C and abs(y) <= MAX_C for x, y in pts)
    assert len(set(pts)) == n
    for i in range(n):
        for j in range(i):
            for k in range(j):
                assert not collinear(pts[i], pts[j], pts[k])
    sys.stdout.write("\n".join([str(n)] + [f"{x} {y}" for x, y in pts]) + "\n")


if __name__ == "__main__":
    main()
