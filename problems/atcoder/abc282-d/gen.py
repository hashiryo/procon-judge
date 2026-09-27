"""abc282-d (Make Bipartite 2) の入力を作る。N M と、M 本の辺 u v を出す。グラフは単純にする。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、N = 2 × 10^5 のグラフ。
グラフは、辺なし (答えが N(N-1)/2 で 2^32 を超える)、ランダムな二部グラフ、それに奇閉路を作る辺を
最後に 1 本足したもの (答えは 0)、パス、偶数の閉路、奇数の閉路、中心が 2 つのスター、完全マッチング、
森、完全二部グラフ K_{400,500} から辺を 1000 本抜いたもの (密で、答えは抜いた本数)。
中心が 1 つのスターは完全二部グラフで答えが 0 に決まるので、中心を 2 つにする。
頂点の番号と辺の順と向きは混ぜる。
seed が 1000 以上なら、辺の無い組を全部試して毎回 2 色に塗り直す愚直解で解ける小さいグラフを出す
(pj testdata crosscheck 用)。
"""

import random
import sys
from collections import deque

MAX_N = 2 * 10**5
MAX_M = 2 * 10**5

Edge = tuple[int, int]

SAMPLES = [
    (5, [(4, 2), (3, 1), (5, 2), (3, 2)]),
    (4, [(3, 1), (3, 2), (1, 2)]),
    (9, [(4, 9), (9, 1), (8, 2), (8, 3), (9, 2), (8, 4), (6, 7), (4, 6), (7, 5), (4, 5), (7, 8)]),
]
FIXED = [
    (2, []),
    (2, [(1, 2)]),
    (3, [(1, 2), (2, 3), (3, 1)]),
    (3, []),
    (4, [(1, 2), (3, 4)]),
    (6, [(1, 2), (2, 3), (3, 4), (4, 5), (5, 6), (6, 1)]),
    (5, [(1, 2), (2, 3), (3, 4), (4, 5), (5, 1)]),
]
# (形, N, M)。M は形によっては決まった値になるので、使わないこともある。
PLANS = [
    ("empty", MAX_N, 0),
    ("bipartite", MAX_N, MAX_M),
    ("odd_last", MAX_N, MAX_M),
    ("path", MAX_N, MAX_N - 1),
    ("even_cycle", MAX_N, MAX_N),
    ("odd_cycle", MAX_N, MAX_N - 1),
    ("double_star", MAX_N, MAX_N - 1),
    ("matching", MAX_N, MAX_N // 2),
    ("forest", MAX_N, 3 * MAX_N // 4),
    ("complete_bipartite", 900, 400 * 500 - 1000),
    ("bipartite", 1000, 2000),
    ("odd_last", 1000, 1500),
]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)
# 小さい入力の形。答えが 0 に偏らないよう、二部グラフになる形を多めにする。
SMALL_SHAPES = ["bipartite"] * 3 + ["forest"] * 2 + [
    "path", "even_cycle", "double_star", "complete_bipartite", "odd_last", "general", "odd_cycle",
]


def colors(n: int, edges: list[Edge]) -> tuple[list[int], list[int]]:
    """二部グラフの連結成分の番号と色。頂点は 0 から n - 1。"""
    adj: list[list[int]] = [[] for _ in range(n)]
    for u, v in edges:
        adj[u].append(v)
        adj[v].append(u)
    comp, color = [-1] * n, [0] * n
    for s in range(n):
        if comp[s] != -1:
            continue
        comp[s] = s
        queue = deque([s])
        while queue:
            v = queue.popleft()
            for u in adj[v]:
                if comp[u] == -1:
                    comp[u], color[u] = s, color[v] ^ 1
                    queue.append(u)
    return comp, color


def random_bipartite(rng: random.Random, n: int, m: int) -> list[Edge]:
    side = [rng.randrange(2) for _ in range(n)]
    left = [v for v in range(n) if side[v] == 0]
    right = [v for v in range(n) if side[v] == 1]
    m = min(m, len(left) * len(right))
    edges: set[Edge] = set()
    while len(edges) < m:
        edges.add((rng.choice(left), rng.choice(right)))
    return list(edges)


def odd_edge(rng: random.Random, n: int, edges: list[Edge]) -> Edge | None:
    """足すと奇閉路ができる辺 (同じ連結成分の同じ色の 2 頂点)。無ければ None。"""
    comp, color = colors(n, edges)
    groups: dict[tuple[int, int], list[int]] = {}
    for v in range(n):
        groups.setdefault((comp[v], color[v]), []).append(v)
    candidates = [g for g in groups.values() if len(g) >= 2]
    if not candidates:
        return None
    return tuple(rng.sample(rng.choice(candidates), 2))


def random_forest(rng: random.Random, n: int, m: int) -> list[Edge]:
    """m 本の辺を持つランダムな森。n - m 個の木に分かれる。"""
    order = list(range(n))
    rng.shuffle(order)
    roots = set(rng.sample(range(1, n), n - 1 - m)) if m < n - 1 else set()
    return [(order[rng.randrange(i)], order[i]) for i in range(1, n) if i not in roots]


def make(rng: random.Random, shape: str, n: int, m: int) -> list[Edge]:
    """頂点は 0 から n - 1。"""
    if shape == "empty":
        return []
    if shape == "bipartite":
        return random_bipartite(rng, n, m)
    if shape == "odd_last":
        edges = random_bipartite(rng, n, m - 1)
        extra = odd_edge(rng, n, edges)
        return edges + [extra] if extra else edges
    if shape == "general":
        m = min(m, n * (n - 1) // 2)
        edges: set[Edge] = set()
        while len(edges) < m:
            u, v = rng.sample(range(n), 2)
            edges.add((min(u, v), max(u, v)))
        return list(edges)
    if shape == "forest":
        return random_forest(rng, n, min(m, n - 1))
    if shape == "path":
        return [(i, i + 1) for i in range(n - 1)]
    if shape == "even_cycle":
        k = n if n % 2 == 0 else n - 1
        return [(i, (i + 1) % k) for i in range(k)] if k >= 4 else [(0, 1)]
    if shape == "odd_cycle":
        k = n if n % 2 == 1 else n - 1
        return [(i, (i + 1) % k) for i in range(k)] if k >= 3 else [(0, 1)]
    if shape == "double_star":
        # 中心の 0 と 1 をつなぎ、残りをどちらかの葉にする。答えは 2 つの葉の数の積。
        split = rng.randint(2, n)
        return [(0, 1)] + [(0 if i < split else 1, i) for i in range(2, n)]
    if shape == "matching":
        return [(2 * i, 2 * i + 1) for i in range(n // 2)]
    assert shape == "complete_bipartite"
    a = n * 4 // 9 if n > 30 else rng.randint(1, n - 1)
    edges = [(u, v) for u in range(a) for v in range(a, n)]
    return rng.sample(edges, min(m, len(edges)))


def relabel(rng: random.Random, n: int, edges: list[Edge], keep_last: bool) -> list[Edge]:
    """番号を混ぜ、辺の順と向きを混ぜる。keep_last なら最後の辺は最後のままにする。"""
    perm = list(range(1, n + 1))
    rng.shuffle(perm)
    out = [(perm[u], perm[v]) if rng.random() < 0.5 else (perm[v], perm[u]) for u, v in edges]
    if keep_last and out:
        body = out[:-1]
        rng.shuffle(body)
        return body + out[-1:]
    rng.shuffle(out)
    return out


def case_for(seed: int, rng: random.Random) -> tuple[int, list[Edge]]:
    if seed >= 1000:
        # 愚直解が N^2 回塗り直せる大きさ。
        n = rng.randint(2, 8) if rng.random() < 0.3 else rng.randint(2, 40)
        shape = rng.choice(SMALL_SHAPES)
        m = rng.randint(0, min(n * (n - 1) // 2, 3 * n))
        if shape == "complete_bipartite":
            m = rng.randint(0, n * n // 4)
        return n, relabel(rng, n, make(rng, shape, n, m), keep_last=shape == "odd_last")
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    if seed < len(SAMPLES) + len(FIXED):
        return FIXED[seed - len(SAMPLES)]
    shape, n, m = PLANS[seed - len(SAMPLES) - len(FIXED)]
    return n, relabel(rng, n, make(rng, shape, n, m), keep_last=shape == "odd_last")


def main() -> None:
    seed = int(sys.argv[1])
    n, edges = case_for(seed, random.Random(seed))
    m = len(edges)
    assert 2 <= n <= MAX_N and 0 <= m <= min(MAX_M, n * (n - 1) // 2)
    assert all(1 <= u <= n and 1 <= v <= n and u != v for u, v in edges)
    assert len({(min(u, v), max(u, v)) for u, v in edges}) == m
    out = [f"{n} {m}"] + [f"{u} {v}" for u, v in edges]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
