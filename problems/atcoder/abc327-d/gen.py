"""abc327-d (Good Tuple Problem) の入力を作る。N M、A_1 ... A_M、B_1 ... B_M を出す。

(A_i, B_i) を辺と見ると、答えはグラフが二部グラフなら Yes、そうでなければ No。
seed が 0 から count - 1 までは本番のケース。例の 4 つ、角のケース、N = M = 2 × 10^5 (8 ケース) などの大きいグラフ。
グラフは、ランダムな二部グラフ (Yes)、それに同じ色の 2 頂点を同じ連結成分の中で結ぶ辺を 1 本足したもの
(奇閉路ができて No)、自己ループを 1 本足したもの (No)、長いパスと長い閉路 (長さの偶奇で決まる)、
スター、完全二部グラフ、小さい閉路の集まり、多重辺、ランダムなグラフを混ぜる。頂点の番号、辺の順、
辺の両端の順はランダムに混ぜる。奇閉路を作る辺は辺の並びの先頭、真ん中、最後のどこかに置く。
seed が 1000 以上なら、X を 2^N 通り全部試す愚直解で解ける N ≤ 16 の入力を出す (pj testdata crosscheck 用)。
"""

import random
import sys

MAX_N = 2 * 10**5
MAX_M = 2 * 10**5

SAMPLES = [
    (3, [(1, 2), (2, 3)]),
    (3, [(1, 2), (2, 3), (3, 1)]),
    (10, [(1, 1)]),
    (7, [(1, 3), (6, 2), (2, 7), (7, 2), (5, 1), (4, 2), (2, 3), (2, 3)]),
]
FIXED = [
    (1, [(1, 1)]),
    (2, [(1, 2)]),
    (2, [(2, 2)]),
    (2, [(1, 2), (2, 1), (1, 2)]),
    (MAX_N, [(1, 2)]),  # 孤立点だらけ
    (MAX_N, [(MAX_N, MAX_N)]),
    (4, [(1, 2), (2, 3), (3, 4), (4, 1)]),
    (5, [(1, 2), (2, 3), (3, 4), (4, 5), (5, 1)]),
]
# (N, M, グラフの作り方)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    (MAX_N, MAX_M, "bipartite"),
    (MAX_N, MAX_M, "odd_edge"),
    (MAX_N, MAX_M, "self_loop"),
    (MAX_N, MAX_N, "cycle"),  # 長さ 2 × 10^5 の閉路 (Yes)
    (MAX_N - 1, MAX_N - 1, "cycle"),  # 長さ 2 × 10^5 - 1 の閉路 (No)
    (MAX_N, MAX_N - 1, "path"),
    (10**5, 10**5, "star_odd"),
    (MAX_N, MAX_M, "random"),
    (900, MAX_M, "complete_bipartite"),
    (900, MAX_M, "complete_bipartite_odd"),
    (10**5, 10**5, "small_cycles_even"),
    (MAX_N, MAX_M, "small_cycles_odd"),
    (10**5, 10**5, "multi"),
    (1000, 10, "random"),
    (1000, 2000, "odd_edge"),
    (10, MAX_M, "multi"),
]


def bipartite(rng: random.Random, n: int, m: int) -> tuple[list[int], list[tuple[int, int]]]:
    """頂点を 2 色にランダムに分け、違う色どうしを結ぶ辺を m 本作る。色と辺を返す (頂点は 0 始まり)。"""
    color = [rng.randrange(2) for _ in range(n)]
    color[0], color[-1] = 0, 1
    sides = [[v for v in range(n) if color[v] == c] for c in (0, 1)]
    return color, [(rng.choice(sides[0]), rng.choice(sides[1])) for _ in range(m)]


def component_pair(rng: random.Random, n: int, edges: list[tuple[int, int]], color: list[int]) -> tuple[int, int]:
    """同じ連結成分の中の、同じ色の 2 頂点を返す。そういう組が無ければ (小さいグラフ) 自己ループにする。"""
    parent = list(range(n))

    def find(v: int) -> int:
        while parent[v] != v:
            parent[v] = parent[parent[v]]
            v = parent[v]
        return v

    for u, v in edges:
        parent[find(u)] = find(v)
    groups: dict[tuple[int, int], list[int]] = {}
    for v in range(n):
        groups.setdefault((find(v), color[v]), []).append(v)
    candidates = [g for g in groups.values() if len(g) >= 2]
    if not candidates:
        v = rng.randrange(n)
        return v, v
    return tuple(rng.sample(rng.choice(candidates), 2))


def insert(rng: random.Random, edges: list[tuple[int, int]], edge: tuple[int, int]) -> list[tuple[int, int]]:
    """edge を辺の並びの先頭、真ん中、最後のどこかに入れる (m は 1 増える)。"""
    pos = rng.choice([0, len(edges) // 2, len(edges)])
    return edges[:pos] + [edge] + edges[pos:]


def graph(rng: random.Random, n: int, m: int, how: str) -> list[tuple[int, int]]:
    """頂点 0 から n - 1 のグラフの辺を m 本返す。"""
    if how == "bipartite":
        return bipartite(rng, n, m)[1]
    if how == "odd_edge":
        color, edges = bipartite(rng, n, m - 1)
        return insert(rng, edges, component_pair(rng, n, edges, color))
    if how == "self_loop":
        edges = bipartite(rng, n, m - 1)[1]
        v = rng.randrange(n)
        return insert(rng, edges, (v, v))
    if how == "cycle":
        return [(i, (i + 1) % n) for i in range(n)]
    if how == "path":
        return [(i, i + 1) for i in range(n - 1)]
    if how == "star_odd":
        edges = [(0, i) for i in range(1, m)]
        return insert(rng, edges, tuple(rng.sample(range(1, n), 2)))
    if how == "random":
        return [(rng.randrange(n), rng.randrange(n)) for _ in range(m)]
    if how in ("complete_bipartite", "complete_bipartite_odd"):
        left = n // 2
        edges = [(u, v) for u in range(left) for v in range(left, n)][:m]
        rng.shuffle(edges)
        edges = edges[: m - 1] if how == "complete_bipartite_odd" else edges
        if how == "complete_bipartite_odd":
            edges = insert(rng, edges, tuple(rng.sample(range(left), 2)))
        return edges
    if how in ("small_cycles_even", "small_cycles_odd"):
        # 長さ 4 か 6 の閉路をたくさん作る。odd では 1 つだけ長さ 5 にする。
        edges: list[tuple[int, int]] = []
        v = 0
        odd = how == "small_cycles_odd"
        while True:
            k = 5 if odd else rng.choice([4, 6])
            if v + k > n or len(edges) + k > m:
                break
            edges += [(v + i, v + (i + 1) % k) for i in range(k)]
            v += k
            odd = False
        rng.shuffle(edges)
        return edges
    assert how == "multi"
    # 少ない辺を何度もくり返す二部グラフ。
    color, base = bipartite(rng, n, max(1, min(m // 10, n)))
    return [rng.choice(base) for _ in range(m)]


def label(rng: random.Random, n: int, edges: list[tuple[int, int]]) -> list[tuple[int, int]]:
    """頂点に 1 から n の番号をランダムに付け、辺の両端の順を混ぜる。辺の順はそのまま。"""
    perm = list(range(1, n + 1))
    rng.shuffle(perm)
    return [(perm[u], perm[v]) if rng.random() < 0.5 else (perm[v], perm[u]) for u, v in edges]


def case_for(seed: int, rng: random.Random) -> tuple[int, list[tuple[int, int]]]:
    if seed >= 1000:
        n = rng.randint(1, 16)
        m = rng.randint(1, 30)
        how = rng.choice(["bipartite", "odd_edge", "self_loop", "cycle", "path", "random", "multi"])
        if n < 3 and how in ("odd_edge", "cycle"):
            how = "random"
        if how == "cycle":
            m = n
        elif how == "path":
            if n < 2:
                how, m = "random", 1
            else:
                m = n - 1
        if how in ("bipartite", "odd_edge", "self_loop", "multi") and n < 2:
            how = "random"
        edges = graph(rng, n, m, how)
        return n, label(rng, n, edges)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    n, m, how = PLANS[seed - len(FIXED)]
    edges = graph(rng, n, m, how)
    if how == "cycle":
        rng.shuffle(edges)
    return n, label(rng, n, edges)


COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)


def main() -> None:
    seed = int(sys.argv[1])
    n, edges = case_for(seed, random.Random(seed))
    m = len(edges)
    assert 1 <= n <= MAX_N and 1 <= m <= MAX_M
    assert all(1 <= a <= n and 1 <= b <= n for a, b in edges)
    out = [f"{n} {m}", " ".join(str(a) for a, _ in edges), " ".join(str(b) for _, b in edges)]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
