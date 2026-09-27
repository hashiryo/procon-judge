"""abc213-g (Connectivity 2) の入力を作る。N M と、単純グラフの辺 a_i b_i (a_i < b_i) を出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、N = 17 までのランダム。
提出の集合冪級数の計算は N だけで決まり、M によらないので、N = 17 のケースは 10 個にして、
形 (辺が無い、完全グラフ、パス、1 が中心のスター、1 が葉のスター、1 が孤立、2 つの完全グラフ、閉路、
ランダム) を変える。
seed が 1000 以上なら、辺の部分集合 2^M 通りを全部試す愚直解で解ける M = 16 までの入力を出す
(pj testdata crosscheck 用)。N は 17 まで使う。2000 以上なら、愚直解でまだ解ける M = 21 までの入力を出す。
"""

import random
import sys

MAX_N = 17


def complete(vs: list[int]) -> list[tuple[int, int]]:
    return [(u, v) for i, u in enumerate(vs) for v in vs[i + 1 :]]


SAMPLES = [
    (3, [(1, 2), (2, 3)]),
    (5, [(1, 2), (1, 4), (1, 5), (2, 3), (2, 5), (3, 4)]),
    (2, []),
]
FIXED = [
    (2, [(1, 2)]),
    (3, complete([1, 2, 3])),
    (MAX_N, []),
    (MAX_N, complete(list(range(1, MAX_N + 1)))),  # M = 136
    (MAX_N, [(i, i + 1) for i in range(1, MAX_N)]),  # パス
    (MAX_N, [(1, i) for i in range(2, MAX_N + 1)]),  # 1 が中心のスター
    (MAX_N, [(i, MAX_N) for i in range(1, MAX_N)]),  # 1 が葉のスター
    (MAX_N, complete(list(range(2, MAX_N + 1)))),  # 1 が孤立していて、答えは全部 0
    (MAX_N, complete(list(range(1, 9))) + complete(list(range(9, MAX_N + 1)))),
    (MAX_N, [(i, i + 1) for i in range(1, MAX_N)] + [(1, MAX_N)]),  # 閉路
    (16, complete(list(range(1, 17)))),
]
# (N, 辺を入れる確率)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    (MAX_N, 0.5),
    (MAX_N, 0.2),
    (15, 0.8),
    (12, 0.3),
    (10, 0.5),
    (8, 0.4),
]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)


def random_graph(rng: random.Random, n: int, p: float) -> list[tuple[int, int]]:
    edges = [e for e in complete(list(range(1, n + 1))) if rng.random() < p]
    rng.shuffle(edges)
    return edges


def case_for(seed: int, rng: random.Random) -> tuple[int, list[tuple[int, int]]]:
    if seed >= 1000:
        # 愚直解が辺の部分集合を全部試せる大きさ。2000 以上は辺を 17 本から 21 本にする。
        lo, hi = (17, 21) if seed >= 2000 else (0, 16)
        n = rng.randint(7 if seed >= 2000 else 2, MAX_N)
        pairs = complete(list(range(1, n + 1)))
        m = rng.randint(min(lo, len(pairs)), min(hi, len(pairs)))
        edges = rng.sample(pairs, m)
        return n, edges
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    n, p = PLANS[seed - len(FIXED)]
    return n, random_graph(rng, n, p)


def main() -> None:
    seed = int(sys.argv[1])
    n, edges = case_for(seed, random.Random(seed))
    assert 2 <= n <= MAX_N and 0 <= len(edges) <= n * (n - 1) // 2
    assert all(1 <= a < b <= n for a, b in edges) and len(set(edges)) == len(edges)
    out = [f"{n} {len(edges)}"] + [f"{a} {b}" for a, b in edges]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
