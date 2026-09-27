"""arc062-d (Painting Graphs with AtCoDeer) の入力を作る。N M K と M 本の辺を出す (自己ループと多重辺は無い)。

答えは 2 重連結成分ごとの積で、橋は K、輪 (頂点と辺の数が同じ) は長さ m の数珠の数、それ以外は辺の色の多重集合の数になる。
seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、N = 50 と M = 100 前後のいろいろな形のグラフ。
角のケースは、辺 1 本、三角形、辺を共有しない 3 本の道で結んだ 2 点 (theta)、頂点を共有する 2 つの三角形、K4、K = 1。
大きいケースは、木、長さ 50 の輪、辺の多い 2 重連結成分 (15 頂点に 100 本、K14 に橋)、仙人掌 (輪を頂点で
つないだもの)、三角形の風車、車輪、格子、弦を 1 本足した輪、橋でつないだ 2 つの輪、正方形の鎖、非連結なもの、ランダム。
頂点の番号と辺の順と向きは混ぜる。
seed が 1000 以上なら、K^M 通りの塗り方を全部作り、単純な輪ごとの巡回で移り合うものをまとめる愚直解で解ける
小さい入力を出す (pj testdata crosscheck 用)。
"""

import random
import sys

MAX_N = 50
MAX_M = 100
MAX_K = 100
# 愚直解が塗り方を全部作れる数の上限。
BRUTE_COLORINGS = 50000

Graph = tuple[int, list[tuple[int, int]]]

SAMPLES = [
    (4, [(1, 2), (2, 3), (3, 1), (3, 4)], 2),
    (5, [(1, 2), (4, 5)], 3),
    (11, [(3, 1), (8, 2), (4, 9), (5, 4), (1, 6), (2, 9), (8, 3), (10, 8), (4, 10), (8, 6), (11, 7), (1, 8)], 48),
]


def cycle(n: int, offset: int = 0) -> list[tuple[int, int]]:
    return [(offset + i, offset + (i + 1) % n) for i in range(n)]


def theta(lengths: list[int]) -> Graph:
    """頂点 0 と 1 を、長さ lengths の道で結ぶ (道どうしは頂点を共有しない)。"""
    edges, n = [], 2
    for length in lengths:
        path = [0] + list(range(n, n + length - 1)) + [1]
        n += length - 1
        edges += list(zip(path, path[1:]))
    return n, edges


def cactus(rng: random.Random, n: int, m: int, lengths: list[int]) -> Graph:
    """輪か橋を、すでにある頂点に 1 つずつつないでいく。頂点が n か辺が m を超えるものは選び直し、20 回続けば止める。"""
    edges, used, misses = [], 1, 0
    while misses < 20:
        length = rng.choice(lengths)
        if used + max(1, length - 1) > n or len(edges) + length > m:
            misses += 1
            continue
        if length == 1:  # 橋
            edges.append((rng.randrange(used), used))
            used += 1
            continue
        at = rng.randrange(used)
        ring = [at] + list(range(used, used + length - 1))
        edges += [(ring[i], ring[(i + 1) % length]) for i in range(length)]
        used += length - 1
    return used, edges


def random_simple(rng: random.Random, n: int, m: int) -> Graph:
    pairs = [(a, b) for a in range(n) for b in range(a + 1, n)]
    return n, rng.sample(pairs, m)


def random_tree(rng: random.Random, n: int) -> Graph:
    return n, [(rng.randrange(i), i) for i in range(1, n)]


def union(parts: list[Graph]) -> Graph:
    n, edges = 0, []
    for k, part in parts:
        edges += [(a + n, b + n) for a, b in part]
        n += k
    return n, edges


def plan(rng: random.Random, name: str) -> Graph:
    if name == "edge":
        return 2, [(0, 1)]
    if name == "triangle":
        return 3, cycle(3)
    if name == "theta_small":
        return theta([2, 2, 2])
    if name == "eight":  # 頂点 0 を共有する 2 つの三角形
        return 5, [(0, 1), (1, 2), (2, 0), (0, 3), (3, 4), (4, 0)]
    if name == "k4":
        return 4, [(a, b) for a in range(4) for b in range(a + 1, 4)]
    if name == "tree":
        return random_tree(rng, MAX_N)
    if name == "cycle":
        return MAX_N, cycle(MAX_N)
    if name == "dense":  # 15 頂点の完全グラフから 5 本除く
        return random_simple(rng, 15, MAX_M)
    if name == "k14_bridges":  # K14 (91 本) と、そこから出る 9 本の橋
        edges = [(a, b) for a in range(14) for b in range(a + 1, 14)]
        return 23, edges + [(rng.randrange(14 + i), 14 + i) for i in range(9)]
    if name == "cactus":
        return cactus(rng, MAX_N, MAX_M, [1, 3, 4, 5, 6, 8, 12])
    if name == "windmill":  # 頂点 0 を共有する 24 個の三角形
        return 49, [e for i in range(24) for e in ((0, 2 * i + 1), (2 * i + 1, 2 * i + 2), (2 * i + 2, 0))]
    if name == "wheel":
        return MAX_N, cycle(MAX_N - 1, 1) + [(0, i) for i in range(1, MAX_N)]
    if name == "grid":  # 5 x 10
        cell = lambda i, j: i * 10 + j  # noqa: E731
        edges = [(cell(i, j), cell(i, j + 1)) for i in range(5) for j in range(9)]
        return MAX_N, edges + [(cell(i, j), cell(i + 1, j)) for i in range(4) for j in range(10)]
    if name == "chord":  # 長さ 50 の輪に弦を 1 本
        return MAX_N, cycle(MAX_N) + [(0, rng.randint(2, MAX_N - 2))]
    if name == "two_rings":  # 長さ 25 と 24 の輪を橋でつなぐ
        n, edges = union([(25, cycle(25)), (24, cycle(24))])
        return n + 1, edges + [(0, 25), (30, 49)]
    if name == "squares":  # 頂点を共有する正方形の鎖
        edges = []
        for i in range(16):
            a, b, c, d = 3 * i, 3 * i + 1, 3 * i + 2, 3 * i + 3
            edges += [(a, b), (b, d), (a, c), (c, d)]
        return 49, edges
    if name == "sparse_disconnected":  # 辺 1 本と、孤立点 48 個
        return MAX_N, [(0, 1)]
    if name == "mixed":  # 輪、theta、木、孤立点
        return union([(10, cycle(10)), theta([3, 4, 5]), random_tree(rng, 15), (13, [])])
    if name == "random_full":
        return random_simple(rng, MAX_N, MAX_M)
    if name == "random_sparse":
        return random_simple(rng, MAX_N, 60)
    if name == "random_thin":
        return random_simple(rng, MAX_N, 55)
    assert name == "random_small"
    return random_simple(rng, 30, 40)


# (グラフの名前, K)。本番のケースのうち例のあとに並べる。
PLANS = [
    ("edge", 1),
    ("edge", MAX_K),
    ("triangle", MAX_K),
    ("theta_small", 3),
    ("eight", 4),
    ("k4", 3),
    ("tree", MAX_K),
    ("cycle", MAX_K),
    ("dense", MAX_K),
    ("k14_bridges", MAX_K),
    ("cactus", MAX_K),
    ("windmill", MAX_K),
    ("wheel", MAX_K),
    ("grid", MAX_K),
    ("chord", MAX_K),
    ("two_rings", MAX_K),
    ("squares", 97),
    ("sparse_disconnected", 37),
    ("mixed", MAX_K),
    ("random_full", MAX_K),
    ("random_sparse", MAX_K),
    ("random_thin", 2),
    ("random_small", 1),
]
COUNT = len(SAMPLES) + len(PLANS)


def small_case(rng: random.Random) -> tuple[Graph, int]:
    """愚直解が K^M 通りの塗り方を全部作れる大きさ。"""
    kind = rng.choice(["random", "random", "random", "cycle", "theta", "eight", "k4", "chord", "tree", "cactus"])
    if kind == "random":
        n = rng.randint(2, 7)
        g = random_simple(rng, n, rng.randint(1, min(10, n * (n - 1) // 2)))
    elif kind == "cycle":
        k = rng.randint(3, 8)
        g = (k, cycle(k))
    elif kind == "theta":
        g = theta(sorted(rng.choice([[1, 2, 2], [2, 2, 2], [1, 2, 3], [2, 2, 3], [2, 3, 3]])))
    elif kind == "eight":
        g = plan(rng, "eight")
    elif kind == "k4":
        g = plan(rng, "k4")
    elif kind == "chord":
        k = rng.randint(4, 7)
        g = (k, cycle(k) + [(0, rng.randint(2, k - 2))])
    elif kind == "tree":
        g = random_tree(rng, rng.randint(2, 7))
    else:
        g = cactus(rng, 7, 9, [1, 3, 4])
    m = len(g[1])
    k = 1
    while (k + 1) ** m <= BRUTE_COLORINGS and k < MAX_K:
        k += 1
    # K = 1 は答えが 1 に決まるので、1 割だけにする (M <= 10 なので k >= 2)。
    return g, 1 if rng.random() < 0.1 else rng.randint(2, k)


def relabel(rng: random.Random, g: Graph) -> tuple[int, list[tuple[int, int]]]:
    """頂点に 1 から n の番号をランダムに付け、辺の順と向きも混ぜる。"""
    n, edges = g
    perm = list(range(1, n + 1))
    rng.shuffle(perm)
    edges = [(perm[a], perm[b]) if rng.random() < 0.5 else (perm[b], perm[a]) for a, b in edges]
    rng.shuffle(edges)
    return n, edges


def case_for(seed: int, rng: random.Random) -> tuple[int, list[tuple[int, int]], int]:
    if seed >= 1000:
        g, k = small_case(rng)
        return *relabel(rng, g), k
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    name, k = PLANS[seed - len(SAMPLES)]
    return *relabel(rng, plan(rng, name)), k


def main() -> None:
    seed = int(sys.argv[1])
    n, edges, k = case_for(seed, random.Random(seed))
    m = len(edges)
    assert 1 <= n <= MAX_N and 1 <= m <= MAX_M and 1 <= k <= MAX_K
    assert all(1 <= a <= n and 1 <= b <= n and a != b for a, b in edges)
    assert len({frozenset(e) for e in edges}) == m  # 多重辺が無い
    out = [f"{n} {m} {k}"] + [f"{a} {b}" for a, b in edges]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
