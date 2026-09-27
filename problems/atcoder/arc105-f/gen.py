"""arc105-f (Lights Out on Connected Graph) の入力を作る。N M と M 本の辺 a_i b_i を出す。

答えは、辺を消して作る部分グラフのうち、連結で二部グラフのもの (頂点は全部残す) の数。
seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、N = 17 のいろいろなグラフ。
角のケースは、N = 1 (M = 0 で答えは 1)、N = 2、三角形 (答えは 3)、長さ 4 の閉路 (答えは 5)、木 (答えは 1)。
N = 17 は、完全グラフ (M = 136)、完全二部グラフ K_{8,9} (部分グラフは全部二部グラフ)、奇数と偶数の閉路
(答えは 17)、パスとスター (答えは 1)、辺の数がいろいろなランダムの連結グラフ。提出は頂点の部分集合の
冪級数で数えるので、手間は N だけで決まり、M によらない。
seed が 1000 以上なら、辺の部分集合を全部試す愚直解で解ける小さいグラフを出す (pj testdata crosscheck 用)。
2000 以上なら、同じ愚直解でまだ解ける M = 22 までのグラフを出す。
"""

import random
import sys

MAX_N = 17

SAMPLES = [
    (3, [(1, 2), (2, 3)]),
    (4, [(1, 2), (1, 3), (1, 4), (2, 3), (2, 4), (3, 4)]),
    (
        17,
        [
            (16, 17), (10, 9), (16, 10), (5, 17), (6, 15), (5, 9), (15, 11), (16, 1), (8, 13), (6, 17),
            (15, 3), (16, 15), (11, 3), (7, 6), (1, 4), (11, 13), (10, 6), (10, 12), (3, 16), (7, 3),
            (16, 5), (13, 3), (12, 13), (7, 11), (3, 12), (13, 10), (1, 12), (9, 15), (11, 14), (4, 6),
            (13, 2), (6, 1), (15, 2), (1, 14), (15, 17), (2, 11), (14, 13), (16, 9), (16, 8), (8, 17),
            (17, 12), (1, 11), (6, 12), (17, 2), (8, 1), (14, 6), (9, 7), (11, 10), (5, 14), (17, 7),
        ],
    ),
]
FIXED = [
    (1, []),
    (2, [(1, 2)]),
    (2, [(2, 1)]),
    (3, [(1, 2), (2, 3), (3, 1)]),
    (4, [(1, 2), (2, 3), (3, 4), (4, 1)]),
    (5, [(1, 2), (1, 3), (3, 4), (3, 5)]),
]
# (N, 形, 辺の数)。本番のケースのうち角のケースのあとに並べる。辺の数はランダムの連結グラフでだけ使う。
PLANS = [
    (MAX_N, "complete", 0),
    (MAX_N, "complete_bipartite", 0),
    (MAX_N, "cycle", 0),
    (16, "cycle", 0),
    (MAX_N, "path", 0),
    (MAX_N, "star", 0),
    (MAX_N, "random", 20),
    (MAX_N, "random", 40),
    (MAX_N, "random", 80),
    (MAX_N, "random", 120),
    (MAX_N, "random", 135),
    (MAX_N, "bipartite_random", 50),
    (10, "random", 30),
]


def graph(rng: random.Random, n: int, shape: str, m: int) -> list[tuple[int, int]]:
    """頂点 0 から n - 1 の辺を返す。"""
    if shape == "complete":
        return [(u, v) for u in range(n) for v in range(u + 1, n)]
    if shape == "complete_bipartite":
        return [(u, v) for u in range(n // 2) for v in range(n // 2, n)]
    if shape == "cycle":
        return [(i, (i + 1) % n) for i in range(n)]
    if shape == "path":
        return [(i, i + 1) for i in range(n - 1)]
    if shape == "star":
        return [(0, i) for i in range(1, n)]
    # ランダムな全域木に辺を足す。bipartite_random は、頂点を 2 色に分けて色の違う組だけから足す
    # (全域木も色の違う辺で作るので、グラフ全体が二部グラフになる)。
    side = [i % 2 for i in range(n)] if shape == "bipartite_random" else [0] * n
    order = list(range(n))
    rng.shuffle(order)
    if shape == "bipartite_random" and n >= 2:
        # 先頭の 2 頂点を違う色にしておけば、3 番目からの頂点にはいつも前に違う色の頂点がある。
        j = next(j for j in range(1, n) if side[order[j]] != side[order[0]])
        order[1], order[j] = order[j], order[1]
    edges = set()
    for i in range(1, n):
        v = order[i]
        u = rng.choice([u for u in order[:i] if side[u] != side[v]] if shape == "bipartite_random" else order[:i])
        edges.add((min(u, v), max(u, v)))
    pairs = [
        (u, v) for u in range(n) for v in range(u + 1, n)
        if (u, v) not in edges and (shape != "bipartite_random" or side[u] != side[v])
    ]
    edges |= set(rng.sample(pairs, min(len(pairs), max(0, m - len(edges)))))
    return sorted(edges)


def label(rng: random.Random, n: int, edges: list[tuple[int, int]]) -> list[tuple[int, int]]:
    """頂点に 1 から n の番号を付け、辺の順と辺の中の 2 頂点の順を混ぜる。"""
    perm = list(range(1, n + 1))
    rng.shuffle(perm)
    out = [(perm[u], perm[v]) if rng.random() < 0.5 else (perm[v], perm[u]) for u, v in edges]
    rng.shuffle(out)
    return out


def case_for(seed: int, rng: random.Random) -> tuple[int, list[tuple[int, int]]]:
    if seed >= 1000:
        # 愚直解は O(2^M (N + M))。
        if seed >= 2000:
            n = rng.randint(6, 9)
            m = rng.randint(n - 1, min(22, n * (n - 1) // 2))
        else:
            n = rng.randint(1, 7)
            m = rng.randint(n - 1, min(16, n * (n - 1) // 2))
        shape = rng.choice(["random", "random", "random", "bipartite_random", "cycle", "complete"])
        if shape == "complete" and n * (n - 1) // 2 > m:
            shape = "random"
        if shape == "cycle" and n < 3:
            shape = "random"
        return n, label(rng, n, graph(rng, n, shape, m))
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    n, shape, m = PLANS[seed - len(FIXED)]
    return n, label(rng, n, graph(rng, n, shape, m))


def main() -> None:
    seed = int(sys.argv[1])
    n, edges = case_for(seed, random.Random(seed))
    m = len(edges)
    assert 1 <= n <= MAX_N and n - 1 <= m <= n * (n - 1) // 2
    assert all(1 <= a <= n and 1 <= b <= n and a != b for a, b in edges)
    assert len({(min(a, b), max(a, b)) for a, b in edges}) == m
    # 連結か確かめる。
    parent = list(range(n + 1))

    def find(v: int) -> int:
        while parent[v] != v:
            parent[v] = parent[parent[v]]
            v = parent[v]
        return v

    for a, b in edges:
        parent[find(a)] = find(b)
    assert len({find(v) for v in range(1, n + 1)}) == 1
    out = [f"{n} {m}"] + [f"{a} {b}" for a, b in edges]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
