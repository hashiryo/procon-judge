"""self-geo-union-of-circles の入力を作る。

N と N 個の円 (中心 x y、半径 r) を出し、和集合の面積を答える。座標と半径は整数。
seed が 0 から count - 1 までが本番 (1 円、2 円の重なり・包含・接する・離れる、同じ円、格子、鎖、ランダム)。
seed が 1000 以上なら、N <= 12 の小さい入力を出す (pj testdata crosscheck で brute.cpp と突き合わせる)。
"""

import random
import sys

LIM = 10**4

PLANS = []


def plan(parts):
    PLANS.append(parts)
    return parts


SINGLE = [(0, 0, 10)]
TWO = [(0, 0, 10), (6, 0, 10)]
SAME = [(0, 0, 10), (0, 0, 10)]
INSIDE = [(0, 0, 100), (10, 0, 5)]
TOUCH_OUT = [(0, 0, 10), (20, 0, 10)]
TOUCH_IN = [(0, 0, 10), (10, 0, 20)]
FAR = [(0, 0, 10), (100, 0, 10)]
GRID = [(i * 30, j * 30, 25) for i in range(20) for j in range(20)]
LINE = [(i * 15, 0, 10) for i in range(1000)]


def random_circles(rng, n, lim, rmin, rmax):
    return [(rng.randint(-lim, lim), rng.randint(-lim, lim), rng.randint(rmin, rmax)) for _ in range(n)]


def main():
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 1000:
        cs = random_circles(rng, rng.randint(1, 12), 30, 1, 40)
    else:
        plans = [
            SINGLE,
            TWO,
            SAME,
            INSIDE,
            TOUCH_OUT,
            TOUCH_IN,
            FAR,
            GRID,
            LINE,
            random_circles(rng, 20, 100, 10, 80),
            random_circles(rng, 1000, LIM, 100, 2000),
            random_circles(rng, 1000, 100, 1, 60),
            random_circles(rng, 50, 5000, 1000, 4000),
            [(10, 10, 10)] * 200,
        ]
        cs = plans[seed % len(plans)]
    sys.stdout.write(str(len(cs)) + "\n" + "\n".join(f"{x} {y} {r}" for x, y, r in cs) + "\n")


main()
