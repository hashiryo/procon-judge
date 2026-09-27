"""abc223-g (Vertex Deletion) の入力を作る。N と、N - 1 本の辺 u v (u < v) を出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース (パス、スター、足の長さ 2 の蜘蛛)、
N = 2 × 10^5 の木。木の形は、パス、スター、毛虫、ランダムな親、深いランダム、完全二分木、
足の長さ 2 の蜘蛛、一様ランダム (Prüfer 列から作る)、ほうき。頂点の番号と辺の順は混ぜる。
番号と辺の順が 1, 2, ..., N のままのパスも 1 つ入れる。パスと蜘蛛は N が偶数だと完全マッチングが
あって答えが 0 に決まるので、番号を混ぜた方は N = 2 × 10^5 - 1 にする。
seed が 1000 以上なら、頂点を 1 つずつ消して数え直す愚直解で解ける小さい木を出す
(pj testdata crosscheck 用)。
"""

import heapq
import random
import sys

MAX_N = 2 * 10**5

SAMPLES = [
    (3, [(1, 2), (2, 3)]),
    (2, [(1, 2)]),
    (6, [(2, 5), (3, 5), (1, 4), (4, 5), (4, 6)]),
]
FIXED = [
    (3, [(1, 2), (1, 3)]),
    (4, [(1, 2), (2, 3), (3, 4)]),  # 完全マッチングがあるので、どの頂点を消しても減る
    (4, [(1, 2), (1, 3), (1, 4)]),
    (5, [(1, 2), (2, 3), (3, 4), (4, 5)]),
    (7, [(1, 2), (2, 3), (1, 4), (4, 5), (1, 6), (6, 7)]),
]
# (形, N)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    ("path", MAX_N - 1),  # N が奇数なので完全マッチングにならない
    ("path_sorted", MAX_N),
    ("star", MAX_N),
    ("caterpillar", MAX_N),
    ("random", MAX_N),
    ("deep", MAX_N),
    ("binary", MAX_N),
    ("spider", MAX_N - 1),
    ("prufer", MAX_N),
    ("broom", 10**5),
    ("random", 1000),
]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)
SHAPES = ["path", "star", "caterpillar", "random", "deep", "binary", "spider", "prufer", "broom"]


def prufer_edges(rng: random.Random, n: int) -> list[tuple[int, int]]:
    """Prüfer 列を一様に選んで木に戻す。頂点は 0 から n - 1。"""
    if n == 2:
        return [(0, 1)]
    seq = [rng.randrange(n) for _ in range(n - 2)]
    degree = [1] * n
    for v in seq:
        degree[v] += 1
    leaves = [v for v in range(n) if degree[v] == 1]
    heapq.heapify(leaves)
    edges = []
    for v in seq:
        leaf = heapq.heappop(leaves)
        edges.append((leaf, v))
        degree[v] -= 1
        if degree[v] == 1:
            heapq.heappush(leaves, v)
    edges.append((heapq.heappop(leaves), heapq.heappop(leaves)))
    return edges


def parents(rng: random.Random, shape: str, n: int) -> list[int]:
    """頂点 i (1 <= i < n) の親。親は i より小さい番号にする。0 番目は使わない。"""
    if shape in ("path", "path_sorted"):
        return [i - 1 for i in range(n)]
    if shape == "star":
        return [0] * n
    if shape == "random":
        return [rng.randrange(i) if i else 0 for i in range(n)]
    if shape == "deep":
        return [rng.randrange(max(0, i - 3), i) if i else 0 for i in range(n)]
    if shape == "binary":
        return [(i - 1) // 2 for i in range(n)]
    if shape == "caterpillar":
        spine = max(1, n // 2)
        return [i - 1 if i < spine else rng.randrange(spine) for i in range(n)]
    if shape == "spider":
        # 0 が中心で、1-2, 3-4, ... が足。n が偶数なら最後の 1 つは中心に付く葉。
        return [i - 1 if i and i % 2 == 0 else 0 for i in range(n)]
    assert shape == "broom"
    handle = max(1, n // 2)
    return [i - 1 if i < handle else handle - 1 for i in range(n)]


def make(rng: random.Random, shape: str, n: int) -> list[tuple[int, int]]:
    if shape == "prufer":
        edges = prufer_edges(rng, n)
    else:
        par = parents(rng, shape, n)
        edges = [(par[i], i) for i in range(1, n)]
    if shape == "path_sorted":
        return [(u + 1, v + 1) for u, v in edges]
    perm = list(range(1, n + 1))
    rng.shuffle(perm)
    edges = [tuple(sorted((perm[u], perm[v]))) for u, v in edges]
    rng.shuffle(edges)
    return edges


def case_for(seed: int, rng: random.Random) -> tuple[int, list[tuple[int, int]]]:
    if seed >= 1000:
        # 愚直解が O(N^2) で解ける大きさ。
        n = rng.randint(2, 12) if rng.random() < 0.3 else rng.randint(2, 300)
        return n, make(rng, rng.choice(SHAPES), n)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    if seed < len(SAMPLES) + len(FIXED):
        return FIXED[seed - len(SAMPLES)]
    shape, n = PLANS[seed - len(SAMPLES) - len(FIXED)]
    return n, make(rng, shape, n)


def is_tree(n: int, edges: list[tuple[int, int]]) -> bool:
    root = list(range(n + 1))

    def find(v: int) -> int:
        while root[v] != v:
            root[v] = root[root[v]]
            v = root[v]
        return v

    for u, v in edges:
        ru, rv = find(u), find(v)
        if ru == rv:
            return False
        root[ru] = rv
    return len(edges) == n - 1


def main() -> None:
    seed = int(sys.argv[1])
    n, edges = case_for(seed, random.Random(seed))
    assert 2 <= n <= MAX_N and all(1 <= u < v <= n for u, v in edges) and is_tree(n, edges)
    out = [str(n)] + [f"{u} {v}" for u, v in edges]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
