"""agc011-c (Squared Graph) の入力を作る。N M と、M 本の辺 u v (u < v、重複なし) を出す。

seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース、小さいランダム、N = 10^5 のケース。
答えは、孤立点、2 部グラフの成分、そうでない成分の数で決まる。大きいケースは、M = 0 (答えは 10^10)、
ランダム、2 部グラフ、2 部グラフに奇数の閉路を作る辺を最後に 1 本足したもの、K2 や三角形ばかりのもの、
パス、長い奇数の閉路と偶数の閉路、スター、辺の数が制約いっぱいの完全グラフと完全 2 部グラフである。
seed が 1000 以上なら、N^2 頂点のグラフを作って幅優先で数える愚直解で解ける小さい入力を出す (pj testdata crosscheck 用)。
"""

import random
import sys

MAX_N = 10**5
MAX_M = 2 * 10**5

SAMPLES = [
    (3, [(1, 2)]),
    (7, [(1, 2), (3, 4), (3, 5), (4, 5), (2, 6)]),
]
FIXED = [
    (2, []),
    (2, [(1, 2)]),
    (3, [(1, 2), (1, 3), (2, 3)]),
    (4, [(1, 2), (2, 3), (3, 4)]),
    (5, [(1, 2), (1, 3), (2, 3), (4, 5)]),  # 三角形と K2
    (MAX_N, []),  # 全部孤立点なので N^2 = 10^10 個
    (MAX_N, [(1, MAX_N)]),
]
# 小さいランダム。(N, M)
SMALL = [(10, 8), (1000, 700), (1000, 3000)]
PLANS = [
    "random",
    "bipartite",
    "bipartite_odd",
    "pieces",
    "path",
    "odd_cycle",
    "even_cycle",
    "star",
    "complete",
    "complete_bipartite",
]
COUNT = len(SAMPLES) + len(FIXED) + len(SMALL) + len(PLANS)


def relabel(rng: random.Random, n: int, edges: list[tuple[int, int]], shuffle: bool = True) -> list[tuple[int, int]]:
    """頂点の番号をランダムに付け替え、u < v にそろえる。shuffle なら辺の順番も混ぜる。"""
    perm = rng.sample(range(1, n + 1), n)
    out = [(min(perm[u], perm[v]), max(perm[u], perm[v])) for u, v in edges]
    if shuffle:
        rng.shuffle(out)
    return out


def random_edges(rng: random.Random, n: int, m: int, color: list[int] | None = None) -> list[tuple[int, int]]:
    """重複のない m 本の辺。color を渡すと、色の違う頂点の間だけに張る。"""
    seen = set()
    while len(seen) < m:
        u, v = rng.randrange(n), rng.randrange(n)
        if u == v or (color is not None and color[u] == color[v]):
            continue
        seen.add((min(u, v), max(u, v)))
    return list(seen)


def cycle(first: int, k: int) -> list[tuple[int, int]]:
    return [(first + i, first + (i + 1) % k) for i in range(k)]


def plan_case(rng: random.Random, how: str) -> tuple[int, list[tuple[int, int]]]:
    n = MAX_N
    if how == "random":
        return n, relabel(rng, n, random_edges(rng, n, MAX_M))
    if how in ("bipartite", "bipartite_odd"):
        color = [rng.randrange(2) for _ in range(n)]
        edges = relabel(rng, n, random_edges(rng, n, MAX_M - 1, color))
        if how == "bipartite":
            return n, edges
        # 同じ側の 2 頂点を結ぶ辺を最後に足す。その成分だけ 2 部グラフでなくなる。
        return n, edges + [odd_edge(rng, n, edges)]
    if how == "pieces":
        # K2 を 2 万個、三角形を 1 万 5 千個、残りは孤立点。
        edges = [(2 * i, 2 * i + 1) for i in range(20000)]
        edges += [e for i in range(15000) for e in cycle(40000 + 3 * i, 3)]
        return n, relabel(rng, n, edges)
    if how == "path":
        return n, relabel(rng, n, [(i, i + 1) for i in range(n - 1)])
    if how == "odd_cycle":
        # 99999 頂点の閉路と孤立点 1 つ。辺は閉路の順に出すので、奇数だと分かるのは最後の辺。
        return n, relabel(rng, n, cycle(0, n - 1), shuffle=False)
    if how == "even_cycle":
        return n, relabel(rng, n, cycle(0, n))
    if how == "star":
        return n, relabel(rng, n, [(0, i) for i in range(1, n)])
    if how == "complete":
        # 632 頂点の完全グラフで 199396 本。
        k = 632
        return n, relabel(rng, n, [(u, v) for u in range(k) for v in range(u + 1, k)])
    # complete_bipartite: 400 頂点と 500 頂点の完全 2 部グラフで 20 万本ちょうど。
    return n, relabel(rng, n, [(u, 400 + v) for u in range(400) for v in range(500)])


def odd_edge(rng: random.Random, n: int, edges: list[tuple[int, int]]) -> tuple[int, int]:
    """2 部グラフの 1 つの成分の中で、同じ側の 2 頂点を結ぶまだ無い辺。"""
    g = [[] for _ in range(n + 1)]
    for u, v in edges:
        g[u].append(v)
        g[v].append(u)
    start = edges[0][0]
    side = {start: 0}
    stack = [start]
    while stack:
        u = stack.pop()
        for v in g[u]:
            if v not in side:
                side[v] = 1 - side[u]
                stack.append(v)
    same = [v for v in side if side[v] == 0]
    present = set(edges)
    while True:
        u, v = rng.sample(same, 2)
        e = (min(u, v), max(u, v))
        if e not in present:
            return e


def small_case(rng: random.Random) -> tuple[int, list[tuple[int, int]]]:
    """愚直解が N^2 頂点のグラフを作れる大きさ。2 部グラフや閉路も混ぜる。M = 0 は角のケースにあるので、なるべく辺を張る。"""
    n = rng.randint(2, 10)
    how = rng.choice(["random", "random", "bipartite", "cycles"])
    all_pairs = n * (n - 1) // 2
    if how == "cycles":
        edges, first = [], 0
        while first < n - 1:
            k = rng.randint(2, n - first)
            edges += [(first + i, first + i + 1) for i in range(k - 1)]
            if k >= 3 and rng.random() < 0.6:
                edges.append((first, first + k - 1))
            first += k + rng.randint(0, 1)
        return n, relabel(rng, n, edges)
    if how == "bipartite":
        color = [rng.randrange(2) for _ in range(n)]
        possible = sum(1 for u in range(n) for v in range(u + 1, n) if color[u] != color[v])
        return n, relabel(rng, n, random_edges(rng, n, rng.randint(min(possible, 1), min(possible, 20)), color))
    return n, relabel(rng, n, random_edges(rng, n, rng.randint(1, min(all_pairs, 20))))


def case_for(seed: int, rng: random.Random) -> tuple[int, list[tuple[int, int]]]:
    if seed >= 1000:
        return small_case(rng)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    seed -= len(FIXED)
    if seed < len(SMALL):
        n, m = SMALL[seed]
        return n, relabel(rng, n, random_edges(rng, n, m))
    return plan_case(rng, PLANS[seed - len(SMALL)])


def main() -> None:
    seed = int(sys.argv[1])
    n, edges = case_for(seed, random.Random(seed))
    assert 2 <= n <= MAX_N and 0 <= len(edges) <= MAX_M
    assert all(1 <= u < v <= n for u, v in edges) and len(set(edges)) == len(edges)
    out = [f"{n} {len(edges)}"] + [f"{u} {v}" for u, v in edges]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
