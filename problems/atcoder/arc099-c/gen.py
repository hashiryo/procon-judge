"""arc099-c (Independence) の入力を作る。N M と M 本の道を出す。

道で結ばれていない 2 都市は別の州に分けるしかないので、分けられるのは、道の無い組を辺にした
グラフ (補グラフ) が二部グラフのときだけ。ケースは補グラフの形で決めて、それ以外の組を全部
道にする。都市の番号と道の順番と向きは混ぜる。

seed が 0 から count - 1 までは本番のケース。例の 4 つ、角のケース、N = 700 前後の大きいもの。
補グラフは、辺なし、完全グラフ、完全二部グラフ、マッチング、パス、偶数と奇数の閉路、三角形、
星の組、完全二部グラフの組、ランダムな二部グラフ (小さい成分だらけから 1 つの大きい成分まで) を
混ぜる。K_{1,99} を 7 つ並べた星の組は、Taka の人数をちょうど半分にできない。ランダムな
二部グラフに同じ側どうしの辺を足して -1 になりやすくしたものと、ランダムなグラフも入れる。
seed が 1000 以上なら、2^N 通りの分け方を全部試す愚直解で解ける N <= 16 の入力を出す
(pj testdata crosscheck 用)。
"""

import random
import sys

MAX_N = 700

SAMPLES = [
    (5, [(1, 2), (1, 3), (3, 4), (3, 5), (4, 5)]),  # 例 1
    (5, [(1, 2)]),  # 例 2
    (4, [(1, 2), (1, 3), (2, 3)]),  # 例 3
    (10, [
        (7, 2), (7, 1), (5, 6), (5, 8), (9, 10), (2, 8), (8, 7), (3, 10), (10, 1), (8, 10),
        (2, 3), (7, 4), (3, 9), (4, 10), (3, 4), (6, 1), (6, 7), (9, 5), (9, 7), (6, 9),
        (9, 4), (4, 6), (7, 5), (8, 3), (2, 5), (9, 2), (10, 7), (8, 6), (8, 9), (7, 3),
        (5, 3), (4, 5), (6, 3), (2, 10), (5, 10), (4, 2), (6, 2), (8, 4), (10, 6),
    ]),  # 例 4
]
# (N, 補グラフの形)。本番のケースのうち例のあとに並べる。
PLANS = [
    (2, "empty"),  # 道が 1 本。答えは 0
    (2, "complete"),  # 道が無いので別の州に分けるしかない。答えは 0
    (3, "complete"),  # 補グラフが三角形なので -1
    (3, "one"),
    (MAX_N, "empty"),  # 完全グラフ。M が最大
    (MAX_N - 1, "empty"),
    (MAX_N, "complete"),  # M = 0 なので -1
    (MAX_N, "star"),  # K_{1,699}。分け方は 1 通り
    (MAX_N, "halves"),  # K_{350,350}
    (500, "unbalanced"),  # K_{150,350}
    (500, "matching"),
    (MAX_N, "path"),
    (400, "even_cycle"),
    (MAX_N, "odd_cycle"),  # C_699 と孤立点なので -1
    (300, "triangle"),  # 三角形と孤立点なので -1
    (MAX_N, "stars"),  # K_{1,99} が 7 つ。Taka は 301 人か 399 人にしかできない
    (600, "blocks"),
    (MAX_N, "sparse"),  # 小さい成分と孤立点だらけ
    (MAX_N, "random"),  # 1 つの大きい成分
    (500, "dense"),
    (MAX_N, "random_plus"),
    (400, "random_graph"),
    (None, "sparse"),  # N はランダム
    (None, "random"),
    (None, "blocks"),
    (None, "random_plus"),
]
COUNT = len(SAMPLES) + len(PLANS)


def complete_bipartite(left: range, right: range) -> list[tuple[int, int]]:
    return [(u, v) for u in left for v in right]


def random_bipartite(rng: random.Random, n: int, q: float) -> tuple[list[bool], list[tuple[int, int]]]:
    """頂点を 2 つの側に分け、側をまたぐ組をそれぞれ確率 q で辺にする。"""
    frac = rng.uniform(0.2, 0.8)
    side = [rng.random() < frac for _ in range(n)]
    edges = [(u, v) for u in range(n) for v in range(u + 1, n) if side[u] != side[v] and rng.random() < q]
    return side, edges


def conflicts_for(rng: random.Random, n: int, shape: str) -> list[tuple[int, int]]:
    """補グラフの辺 (別の州に分けるしかない組) を 0 始まりの番号で返す。"""
    if shape == "empty":
        return []
    if shape == "complete":
        return [(u, v) for u in range(n) for v in range(u + 1, n)]
    if shape == "one":
        return [(0, 1)]
    if shape == "star":
        return [(0, v) for v in range(1, n)]
    if shape == "halves":
        return complete_bipartite(range(n // 2), range(n // 2, n))
    if shape == "unbalanced":
        return complete_bipartite(range(n * 3 // 10), range(n * 3 // 10, n))
    if shape == "matching":
        return [(2 * i, 2 * i + 1) for i in range(n // 2)]
    if shape == "path":
        return [(i, i + 1) for i in range(n - 1)]
    if shape in ("even_cycle", "odd_cycle"):
        # 長さ k の閉路と、余った孤立点。閉路が作れないほど小さければパスにする。
        k = n if (n % 2 == 1) == (shape == "odd_cycle") else n - 1
        if k < 3 or (k < 4 and shape == "even_cycle"):
            return [(i, i + 1) for i in range(k - 1)]
        return [(i, (i + 1) % k) for i in range(k)]
    if shape == "triangle":
        return [(0, 1), (1, 2), (0, 2)] if n >= 3 else [(0, 1)]
    if shape == "stars":
        size = 100 if n == MAX_N else rng.randint(2, n)
        return [(c, c + i) for c in range(0, n, size) for i in range(1, min(size, n - c))]
    if shape == "blocks":
        # 完全二部グラフ K_{s,t} を並べる。s と t の差がばらばらなので部分和が効く。
        edges, start, most = [], 0, max(1, n // 20)
        while start < n:
            s = min(rng.randint(1, most), n - start)
            t = min(rng.randint(0, 3 * most), n - start - s)
            edges += complete_bipartite(range(start, start + s), range(start + s, start + s + t))
            start += s + t
        return edges
    if shape in ("sparse", "random", "dense"):
        # 補グラフの平均次数がおよそ 0.7、4、n / 4 になるように。
        q = {"sparse": 1.4, "random": 8, "dense": n / 2}[shape] / n
        return random_bipartite(rng, n, min(q, 1.0))[1]
    if shape == "random_plus":
        # ランダムな二部グラフに、同じ側どうしの辺を 1 本か 2 本足す。
        side, edges = random_bipartite(rng, n, min(6 / n, 1.0))
        groups = [g for g in ([v for v in range(n) if side[v]], [v for v in range(n) if not side[v]]) if len(g) >= 2]
        for _ in range(rng.randint(1, 2) if groups else 0):
            edges.append(tuple(sorted(rng.sample(rng.choice(groups), 2))))
        return edges
    if shape == "random_graph":
        p = rng.choice([0.05, 0.2, 0.5]) if n > 16 else rng.choice([0.1, 0.3, 0.6])
        return [(u, v) for u in range(n) for v in range(u + 1, n) if rng.random() < p]
    raise ValueError(shape)


def roads(rng: random.Random, n: int, conflicts: list[tuple[int, int]]) -> list[tuple[int, int]]:
    """conflicts 以外の組を全部道にする。都市の番号と道の順番と向きを混ぜる。"""
    bad = {(min(u, v), max(u, v)) for u, v in conflicts}
    label = list(range(1, n + 1))
    rng.shuffle(label)
    edges = [
        (label[u], label[v]) if rng.random() < 0.5 else (label[v], label[u])
        for u in range(n) for v in range(u + 1, n) if (u, v) not in bad
    ]
    rng.shuffle(edges)
    return edges


def case_for(seed: int, rng: random.Random) -> tuple[int, list[tuple[int, int]]]:
    if seed >= 1000:
        # 2^N 通りを全部試せる大きさ。形は本番と同じものから選ぶ。
        n = rng.randint(2, 16)
        shape = rng.choice([
            "empty", "complete", "star", "halves", "matching", "path", "even_cycle", "odd_cycle",
            "triangle", "stars", "blocks", "sparse", "random", "dense", "random_plus", "random_plus",
            "random_graph",
        ])
        return n, roads(rng, n, conflicts_for(rng, n, shape))
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    n, shape = PLANS[seed - len(SAMPLES)]
    n = n or rng.randint(20, MAX_N)
    return n, roads(rng, n, conflicts_for(rng, n, shape))


def main() -> None:
    seed = int(sys.argv[1])
    n, edges = case_for(seed, random.Random(seed))
    assert 2 <= n <= MAX_N and len(edges) <= n * (n - 1) // 2
    assert all(1 <= a <= n and 1 <= b <= n and a != b for a, b in edges)
    assert len({(min(a, b), max(a, b)) for a, b in edges}) == len(edges)
    out = [f"{n} {len(edges)}"] + [f"{a} {b}" for a, b in edges]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
