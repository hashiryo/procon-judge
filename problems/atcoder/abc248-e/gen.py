"""atcoder-abc248-e (K-colinear Line) の入力を作る。

N 個の点と K を出し、K 個以上の点を通る直線の数を答える (K=1 なら Infinity)。
seed が 0 から count - 1 までが本番 (例、角、ランダム、最大、一直線に並ぶ点)。
seed が 1000 以上なら、N <= 10 の小さい入力を出す (pj testdata crosscheck 用)。
"""

import random
import sys

LIM = 10**9

SAMPLES = [
    ((5, 2), [(0, 0), (1, 0), (0, 1), (-1, 0), (0, -1)]),
    ((1, 1), [(0, 0)]),
]


def random_distinct(rng, n, lim):
    pts = set()
    while len(pts) < n:
        pts.add((rng.randint(-lim, lim), rng.randint(-lim, lim)))
    return list(pts)


def main():
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 1000:
        n = rng.randint(1, 10)
        pts = random_distinct(rng, n, 5)
        k = rng.randint(1, n)
    else:
        plans = [
            lambda: SAMPLES[0],
            lambda: SAMPLES[1],
            lambda: ((2, 2), [(0, 0), (LIM, LIM)]),
            lambda: ((3, 3), [(0, 0), (1, 1), (2, 2)]),
            lambda: ((3, 3), [(0, 0), (1, 0), (0, 1)]),
            lambda: ((300, 2), random_distinct(rng, 300, LIM)),
            lambda: ((300, 1), random_distinct(rng, 300, LIM)),
            lambda: ((300, 300), [(i, 2 * i + 1) for i in range(300)]),
            lambda: ((300, 50), [(i % 5, i // 5) for i in range(300)]),
            lambda: ((200, 3), [(i, i * i) for i in range(31)] + random_distinct(rng, 169, LIM)),
        ]
        (n, k), pts = plans[seed % len(plans)]()
        pts = pts[:n]
    out = [f"{len(pts)} {k}"] + [f"{x} {y}" for x, y in pts]
    sys.stdout.write("\n".join(out) + "\n")


main()
