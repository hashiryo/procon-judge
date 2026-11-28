"""atcoder-arc082-c (ConvexScore) の入力を作る。

N 個の点を出し、凸多角形をなす部分集合 S のスコア 2^(n-|S|) の総和を 998244353 で割った余りを答える。
seed が 0 から count - 1 までが本番 (例、角、ランダム、最大、一直線)。
seed が 1000 以上なら、N <= 10 の小さい入力を出す (pj testdata crosscheck 用)。
"""

import random
import sys

LIM = 10**4

SAMPLES = [
    [(0, 0), (0, 1), (1, 0), (1, 1)],
    [(0, 0), (0, 1), (0, 2), (0, 3), (1, 1)],
    [(3141, 2718)],
]


def random_distinct(rng, n, lim):
    pts = set()
    while len(pts) < n:
        pts.add((rng.randint(0, lim - 1), rng.randint(0, lim - 1)))
    return list(pts)


def main():
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 1000:
        pts = random_distinct(rng, rng.randint(1, 10), 50)
    else:
        plans = [
            lambda: SAMPLES[0],
            lambda: SAMPLES[1],
            lambda: SAMPLES[2],
            lambda: [(0, 0), (1000, 1001)],
            lambda: [(0, 0), (1, 1), (2, 2)],
            lambda: [(0, 0), (1, 0), (0, 1)],
            lambda: [(i % 3, i // 3) for i in range(9)],
            lambda: [(i, 2 * i + 1) for i in range(200)],
            lambda: random_distinct(rng, 200, LIM),
            lambda: [(i % 20, i // 20) for i in range(200)],
        ]
        pts = plans[seed % len(plans)]()
    sys.stdout.write(str(len(pts)) + "\n" + "\n".join(f"{x} {y}" for x, y in pts) + "\n")


main()
