"""abc199-d (RGB Coloring 2) の入力を作る。N M と M 本の辺を出す。答えは、辺の両端の色が違う 3 色の塗り方の数。

seed が 0 から count - 1 までは本番のケース。例の 4 つ、角のケース、いろいろな形のグラフ。
形は、完全グラフ、パス、偶数と奇数の閉路、スター、木、完全 2 部グラフ、完全 3 部グラフ、三角形を並べたもの、
車輪、Petersen グラフ、Grötzsch グラフ (三角形が無いのに 3 色で塗れない)、格子、超立方体、
3 色で塗れるように作ったランダム、ランダム。
N が 13 以上だと egf_T は rnk_zeta を使う計算を通り、12 以下だと素朴な計算だけを通るので、両方を入れる。
頂点の番号、辺の向き、辺の順は混ぜる。
seed が 1000 以上なら、頂点の番号順に色を決めて、同じ色の辺ができた所で打ち切る愚直解で解ける
N = 12 までのグラフを出す (pj testdata crosscheck 用)。
2000 以上なら、同じ愚直解でまだ解ける、辺の多いつながった N = 20 までのグラフを出す。
"""

import itertools
import random
import sys

MAX_N = 20

SAMPLES = [
    (3, [(1, 2), (2, 3), (3, 1)]),
    (3, []),
    (4, [(1, 2), (2, 3), (3, 4), (2, 4), (1, 3), (1, 4)]),
    (20, []),
]
FIXED = [
    (1, []),
    (2, [(1, 2)]),
    (2, []),
    (20, [(1, 20)]),
]
# (N, 形)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    (20, "complete"),  # 答えは 0
    (20, "path"),  # 3 * 2^19
    (20, "cycle"),  # 2^20 + 2
    (19, "cycle"),  # 2^19 - 2
    (20, "star"),  # 3 * 2^19
    (20, "tree"),
    (20, "bipartite"),  # K_{10,10}
    (20, "tripartite"),  # K_{7,7,6}。答えは 6
    (20, "tripartite_plus"),  # K_{7,7,6} に同じ組の中の辺を 1 本足す。答えは 0
    (20, "triangles"),  # 三角形 6 個と孤立点 2 個。6^6 * 9
    (20, "wheel"),  # 中心と 19 頂点の閉路。答えは 0
    (19, "wheel"),  # 中心と 18 頂点の閉路。答えは 6
    (10, "petersen"),
    (11, "grotzsch"),  # 答えは 0
    (20, "grid"),  # 4 × 5
    (16, "hypercube"),
    (20, "cocktail"),  # 完全グラフから完全マッチングを抜いたもの。答えは 0
    (20, "k4_isolated"),  # K4 と孤立点 16 個。答えは 0
    (20, "planted0.3"),
    (20, "planted0.6"),
    (20, "planted0.9"),
    (20, "random0.1"),
    (20, "random0.2"),
    (13, "random0.3"),
    (12, "random0.3"),
    (13, "empty"),  # 3^13
    (18, "planted0.2"),
]


def shape(rng: random.Random, n: int, how: str) -> list[tuple[int, int]]:
    """頂点 0 から n - 1 のグラフの辺を返す。"""
    if how == "empty":
        return []
    if how == "complete":
        return list(itertools.combinations(range(n), 2))
    if how == "path":
        return [(i - 1, i) for i in range(1, n)]
    if how == "cycle":
        return [(i - 1, i) for i in range(1, n)] + [(n - 1, 0)]
    if how == "star":
        return [(0, i) for i in range(1, n)]
    if how == "tree":
        return [(rng.randrange(i), i) for i in range(1, n)]
    if how == "bipartite":
        h = n // 2
        return [(i, j) for i in range(h) for j in range(h, n)]
    if how.startswith("tripartite"):
        part = [i % 3 for i in range(n)]
        edges = [(i, j) for i, j in itertools.combinations(range(n), 2) if part[i] != part[j]]
        if how == "tripartite_plus":
            edges.append((0, 3))
        return edges
    if how == "triangles":
        return [(3 * t + i, 3 * t + j) for t in range(n // 3) for i, j in ((0, 1), (1, 2), (0, 2))]
    if how == "wheel":
        rim = n - 1
        return [(0, i) for i in range(1, n)] + [(i, i % rim + 1) for i in range(1, n)]
    if how == "petersen":
        assert n == 10
        return [(i, (i + 1) % 5) for i in range(5)] + [(i, i + 5) for i in range(5)] + [
            (5 + i, 5 + (i + 2) % 5) for i in range(5)
        ]
    if how == "grotzsch":
        # 5 頂点の閉路の Mycielski グラフ。頂点 0..4 が閉路、5..9 がその影、10 が影と全部つながる頂点。
        assert n == 11
        cycle = [(i, (i + 1) % 5) for i in range(5)]
        return cycle + [(5 + u, v) for u, v in cycle] + [(u, 5 + v) for u, v in cycle] + [(10, 5 + i) for i in range(5)]
    if how == "grid":
        rows, cols = 4, n // 4
        edges = []
        for r in range(rows):
            for c in range(cols):
                if c + 1 < cols:
                    edges.append((r * cols + c, r * cols + c + 1))
                if r + 1 < rows:
                    edges.append((r * cols + c, (r + 1) * cols + c))
        return edges
    if how == "hypercube":
        assert n == 16
        return [(v, v ^ (1 << b)) for v in range(n) for b in range(4) if v < v ^ (1 << b)]
    if how == "cocktail":
        return [(i, j) for i, j in itertools.combinations(range(n), 2) if j != i + 1 or i % 2 == 1]
    if how == "k4_isolated":
        return list(itertools.combinations(range(4), 2))
    if how.startswith("planted"):
        p = float(how[len("planted"):])
        part = [rng.randrange(3) for _ in range(n)]
        return [(i, j) for i, j in itertools.combinations(range(n), 2) if part[i] != part[j] and rng.random() < p]
    assert how.startswith("random")
    p = float(how[len("random"):])
    return [(i, j) for i, j in itertools.combinations(range(n), 2) if rng.random() < p]


def connected(rng: random.Random, n: int) -> list[tuple[int, int]]:
    """ランダムな木に、ランダムな辺を足したもの。愚直解が途中で打ち切れるように、辺を多めにする。"""
    edges = {(rng.randrange(i), i) for i in range(1, n)}
    p = rng.uniform(0.15, 0.6)
    if rng.random() < 0.5:
        part = [rng.randrange(3) for _ in range(n)]
        extra = [(i, j) for i, j in itertools.combinations(range(n), 2) if part[i] != part[j] and rng.random() < p]
    else:
        extra = [(i, j) for i, j in itertools.combinations(range(n), 2) if rng.random() < p]
    return sorted(edges | set(extra))


def small(rng: random.Random, n: int) -> list[tuple[int, int]]:
    how = rng.choice(["empty", "complete", "path", "cycle", "star", "tree", "bipartite", "tripartite",
                      "triangles", "planted0.4", "planted0.8", "random0.1", "random0.3", "random0.5"])
    if how == "cycle" and n < 3:
        how = "path"
    return shape(rng, n, how)


def label(rng: random.Random, n: int, edges: list[tuple[int, int]]) -> list[tuple[int, int]]:
    """頂点に 1 から n の番号を付け、辺の順と向きを混ぜる。"""
    perm = list(range(1, n + 1))
    rng.shuffle(perm)
    out = [(perm[u], perm[v]) if rng.random() < 0.5 else (perm[v], perm[u]) for u, v in edges]
    rng.shuffle(out)
    return out


def case_for(seed: int, rng: random.Random) -> tuple[int, list[tuple[int, int]]]:
    if seed >= 2000:
        n = rng.randint(13, MAX_N)
        return n, label(rng, n, connected(rng, n))
    if seed >= 1000:
        n = rng.randint(1, 12)
        return n, label(rng, n, small(rng, n))
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    n, how = PLANS[seed - len(FIXED)]
    return n, label(rng, n, shape(rng, n, how))


def main() -> None:
    seed = int(sys.argv[1])
    n, edges = case_for(seed, random.Random(seed))
    assert 1 <= n <= MAX_N and len(edges) <= n * (n - 1) // 2
    assert all(1 <= a <= n and 1 <= b <= n and a != b for a, b in edges)
    assert len({(min(a, b), max(a, b)) for a, b in edges}) == len(edges)
    out = [f"{n} {len(edges)}"] + [f"{a} {b}" for a, b in edges]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
