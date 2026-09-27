"""abc209-d (Collision) の入力を作る。N Q、木の辺、Q 個の質問を出す。答えは c と d の距離の偶奇。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、N = Q = 10^5 のいろいろな形の木。
木の形は、ランダム、パス、スター、毛虫、二分木、ほうき、深いランダム。パスには、辺を
union by size の木が最も深くなる順 (同じ大きさの塊どうしを繋ぐ順) に並べたものも入れる。
lib-ufpu.cpp の UnionFind は経路圧縮をしないので、根までの長さがそのまま効く。
seed が 1000 以上なら、質問ごとに幅優先探索する愚直解で解ける小さい入力を出す (pj testdata crosscheck 用)。
"""

import random
import sys

MAX_N = 10**5
MAX_Q = 10**5

SAMPLES = [
    (4, [(1, 2), (2, 3), (2, 4)], [(1, 2)]),
    (5, [(1, 2), (2, 3), (3, 4), (4, 5)], [(1, 3), (1, 5)]),
    (
        9,
        [(2, 3), (5, 6), (4, 8), (8, 9), (4, 5), (3, 4), (1, 9), (3, 7)],
        [(7, 9), (2, 5), (2, 6), (4, 6), (2, 4), (5, 8), (7, 8), (3, 6), (5, 6)],
    ),
]
# (N, Q, 木の形, 頂点の番号と辺の順の付け方)。本番のケースのうち例のあとに並べる。
# Q = 0 は、全部の組を 1 回ずつ聞くという意味。
PLANS = [
    (2, 1, "path", "sorted"),
    (3, 0, "path", "random"),
    (2, MAX_Q, "path", "sorted"),
    (MAX_N, 1, "random", "random"),
    (447, 0, "random", "random"),  # 全部の組で 99681 個
    (MAX_N, MAX_Q, "random", "random"),
    (MAX_N, MAX_Q, "path", "sorted"),
    (MAX_N, MAX_Q, "path", "union"),
    (MAX_N, MAX_Q, "star", "random"),
    (MAX_N, MAX_Q, "caterpillar", "random"),
    (MAX_N, MAX_Q, "binary", "random"),
    (MAX_N, MAX_Q, "broom", "random"),
    (MAX_N, MAX_Q, "deep", "random"),
]
COUNT = len(SAMPLES) + len(PLANS)
SHAPES = ["random", "path", "star", "caterpillar", "binary", "broom", "deep"]


def tree(rng: random.Random, n: int, shape: str) -> list[tuple[int, int]]:
    """頂点 0 から n - 1 の木の辺を (親, 子) で返す。親の番号は子より小さい。"""
    if shape == "random":
        return [(rng.randrange(i), i) for i in range(1, n)]
    if shape == "path":
        return [(i - 1, i) for i in range(1, n)]
    if shape == "star":
        return [(0, i) for i in range(1, n)]
    if shape == "caterpillar":
        spine = max(1, n // 2)
        return [(i - 1, i) for i in range(1, spine)] + [(rng.randrange(spine), i) for i in range(spine, n)]
    if shape == "binary":
        return [((i - 1) // 2, i) for i in range(1, n)]
    if shape == "broom":
        handle = max(1, n // 2)
        return [(i - 1, i) for i in range(1, handle)] + [(handle - 1, i) for i in range(handle, n)]
    assert shape == "deep"
    return [(rng.randint(max(0, i - 3), i - 1), i) for i in range(1, n)]


def label(rng: random.Random, n: int, edges: list[tuple[int, int]], how: str) -> list[tuple[int, int]]:
    """頂点に 1 から n の番号を付け、辺を並べる。sorted は番号も順もそのまま、random は両方混ぜる。
    union はパスの辺 (i - 1, i) を i の末尾の 0 の数の順に並べる (番号は混ぜる)。
    """
    perm = list(range(1, n + 1))
    if how != "sorted":
        rng.shuffle(perm)
    if how == "union":
        edges = sorted(edges, key=lambda e: (e[1] & -e[1], e[1]))
    elif how == "random":
        edges = edges[:]
        rng.shuffle(edges)
    return [tuple(sorted((perm[p], perm[c]))) for p, c in edges]


def queries(rng: random.Random, n: int, q: int) -> list[tuple[int, int]]:
    if q == 0:
        return [(c, d) for c in range(1, n + 1) for d in range(c + 1, n + 1)]
    return [tuple(sorted(rng.sample(range(1, n + 1), 2))) for _ in range(q)]


def case_for(seed: int, rng: random.Random) -> tuple[int, list[tuple[int, int]], list[tuple[int, int]]]:
    if seed >= 1000:
        # 愚直解が質問ごとに木全体をたどっても速い大きさ。
        n = rng.randint(2, 60)
        shape = rng.choice(SHAPES)
        how = rng.choice(["sorted", "random", "union"] if shape == "path" else ["sorted", "random"])
        q = 0 if n <= 12 and rng.random() < 0.3 else rng.randint(1, 60)
        return n, label(rng, n, tree(rng, n, shape), how), queries(rng, n, q)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    n, q, shape, how = PLANS[seed - len(SAMPLES)]
    return n, label(rng, n, tree(rng, n, shape), how), queries(rng, n, q)


def main() -> None:
    seed = int(sys.argv[1])
    n, edges, qs = case_for(seed, random.Random(seed))
    assert 2 <= n <= MAX_N and 1 <= len(qs) <= MAX_Q and len(edges) == n - 1
    assert all(1 <= a < b <= n for a, b in edges) and all(1 <= c < d <= n for c, d in qs)
    # 木になっているか (つながっているか) を確かめる。
    parent = list(range(n + 1))

    def find(v: int) -> int:
        while parent[v] != v:
            parent[v] = parent[parent[v]]
            v = parent[v]
        return v

    for a, b in edges:
        ra, rb = find(a), find(b)
        assert ra != rb
        parent[ra] = rb
    out = [f"{n} {len(qs)}"] + [f"{a} {b}" for a, b in edges] + [f"{c} {d}" for c, d in qs]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
