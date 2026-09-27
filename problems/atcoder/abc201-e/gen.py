"""abc201-e (Xor Distances) の入力を作る。N と、N - 1 本の辺 u v w を出す。答えは全部の組の XOR 距離の和。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、N = 2 x 10^5 のいろいろな形の木。
木の形は、ランダム、パス、スター、毛虫、二分木、ほうき、深いランダム。パスには、辺を
union by size の木が最も深くなる順 (同じ大きさの塊どうしを繋ぐ順) に並べたものも入れる。
lib-ufpu.cpp の UnionFind は経路圧縮をしないので、根までの長さがそのまま効く。
重みは、60 ビットのランダム、全部 0、全部 2^60 - 1、1 ビットだけ立ったもの、小さい値を混ぜる。
60 ビットの重みは 1 行が長いので、制約いっぱいの N で使うのは 2 つにして、残りは短い値か小さい N にする。
seed が 1000 以上なら、頂点ごとに木をたどって全部の組を足す愚直解で解ける小さい入力を出す
(pj testdata crosscheck 用)。
"""

import random
import sys

MAX_N = 2 * 10**5
TOP = 2**60 - 1

SAMPLES = [
    (3, [(1, 2, 1), (1, 3, 3)]),
    (5, [(3, 5, 2), (2, 3, 2), (1, 5, 1), (4, 5, 13)]),
    (10, [
        (5, 7, 459221860242673109), (6, 8, 248001948488076933), (3, 5, 371922579800289138),
        (2, 5, 773108338386747788), (6, 10, 181747352791505823), (1, 3, 803225386673329326),
        (7, 8, 139939802736535485), (9, 10, 657980865814127926), (2, 4, 146378247587539124),
    ]),
]
# (N, 木の形, 頂点の番号と辺の順の付け方, 重みの出し方)。本番のケースのうち例のあとに並べる。
PLANS = [
    (2, "path", "sorted", "zero"),
    (2, "path", "sorted", "top"),
    (3, "star", "random", "random"),
    (MAX_N, "random", "random", "random"),
    (MAX_N, "path", "union", "random"),
    (MAX_N, "path", "sorted", "small"),
    (MAX_N, "deep", "random", "bit"),
    (MAX_N, "binary", "random", "one"),  # 1 本だけ 2^59、残りは 0
    (50000, "star", "random", "top"),
    (50000, "caterpillar", "random", "random"),
    (50000, "broom", "random", "high"),
    (50000, "binary", "random", "zero"),
]
COUNT = len(SAMPLES) + len(PLANS)
SHAPES = ["random", "path", "star", "caterpillar", "binary", "broom", "deep"]
WEIGHTS = ["random", "zero", "top", "bit", "small", "high", "one"]


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


def weights(rng: random.Random, m: int, how: str) -> list[int]:
    if how == "random":
        return [rng.randrange(2**60) for _ in range(m)]
    if how == "zero":
        return [0] * m
    if how == "top":
        return [TOP] * m
    if how == "bit":
        return [1 << rng.randrange(60) for _ in range(m)]
    if how == "small":
        return [rng.randrange(4) for _ in range(m)]
    if how == "high":
        return [rng.choice([0, 1 << 59, TOP]) for _ in range(m)]
    assert how == "one"
    w = [0] * m
    w[rng.randrange(m)] = 1 << 59
    return w


def case_for(seed: int, rng: random.Random) -> tuple[int, list[tuple[int, int, int]]]:
    if seed >= 1000:
        # 愚直解が頂点ごとに木全体をたどっても速い大きさ。
        n = rng.randint(2, 80)
        shape = rng.choice(SHAPES)
        how = rng.choice(["sorted", "random", "union"] if shape == "path" else ["sorted", "random"])
        how_w = rng.choice(WEIGHTS)
    elif seed < len(SAMPLES):
        return SAMPLES[seed]
    else:
        n, shape, how, how_w = PLANS[seed - len(SAMPLES)]
    edges = label(rng, n, tree(rng, n, shape), how)
    return n, [(u, v, w) for (u, v), w in zip(edges, weights(rng, n - 1, how_w))]


def main() -> None:
    seed = int(sys.argv[1])
    n, edges = case_for(seed, random.Random(seed))
    assert 2 <= n <= MAX_N and len(edges) == n - 1
    assert all(1 <= u < v <= n and 0 <= w < 2**60 for u, v, w in edges)
    # 木になっているか (つながっているか) を確かめる。
    parent = list(range(n + 1))

    def find(v: int) -> int:
        while parent[v] != v:
            parent[v] = parent[parent[v]]
            v = parent[v]
        return v

    for u, v, _ in edges:
        ru, rv = find(u), find(v)
        assert ru != rv
        parent[ru] = rv
    out = [str(n)] + [f"{u} {v} {w}" for u, v, w in edges]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
