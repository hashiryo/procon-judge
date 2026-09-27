"""abc160-f (Distributing Integers) の入力を作る。N と N - 1 本の辺を出す。答えは根 k ごとの N! / (部分木の大きさの積)。

seed が 0 から count - 1 までは本番のケース。例の 4 つ、角のケース、いろいろな形の大きい木。
木の形は、ランダム、パス、スター、毛虫、二分木、ほうき、深いランダム、蜘蛛 (中心から長さ sqrt(N) の足を
sqrt(N) 本出したもの)。N = 2 × 10^5 は、出力も N 行あって大きいので 4 つにして、残りの形は N = 3 × 10^4 にする
(愚直解が 1 分かからずに解けるので、突き合わせられる)。
パスの答えは端から k 番目で C(N - 1, k - 1)、スターの答えは中心で (N - 1)!、葉で (N - 2)! になる。
seed が 1000 以上なら、愚直解が番号を書いた頂点の集合の DP で解ける N <= 16 の木を出す (pj testdata crosscheck 用)。
2000 以上なら、愚直解が根ごとに部分木の大きさから求める N <= 2000 の木を出す。
"""

import random
import sys

MAX_N = 2 * 10**5

SAMPLES = [
    (3, [(1, 2), (1, 3)]),
    (2, [(1, 2)]),
    (5, [(1, 2), (2, 3), (3, 4), (3, 5)]),
    (8, [(1, 2), (2, 3), (3, 4), (3, 5), (3, 6), (6, 7), (6, 8)]),
]
# (N, 木の形, 頂点の番号と辺の順の付け方)。本番のケースのうち例のあとに並べる。
PLANS = [
    (3, "path", "random"),
    (4, "star", "random"),
    (16, "binary", "random"),
    (1000, "random", "random"),
    (MAX_N, "random", "random"),
    (MAX_N, "path", "random"),
    (MAX_N, "star", "random"),
    (MAX_N, "deep", "random"),
    (30000, "path", "sorted"),
    (30000, "caterpillar", "random"),
    (30000, "binary", "random"),
    (30000, "broom", "random"),
    (30000, "spider", "random"),
]
COUNT = len(SAMPLES) + len(PLANS)
SHAPES = ["random", "path", "star", "caterpillar", "binary", "broom", "deep", "spider"]


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
    if shape == "spider":
        leg = max(1, int(n**0.5))
        return [(0 if (i - 1) % leg == 0 else i - 1, i) for i in range(1, n)]
    assert shape == "deep"
    return [(rng.randint(max(0, i - 3), i - 1), i) for i in range(1, n)]


def label(rng: random.Random, n: int, edges: list[tuple[int, int]], how: str) -> list[tuple[int, int]]:
    """頂点に 1 から n の番号を付け、辺を並べる。sorted は番号も順もそのまま、random は番号と順と辺の向きを混ぜる。"""
    perm = list(range(1, n + 1))
    if how == "sorted":
        return [(perm[p], perm[c]) for p, c in edges]
    rng.shuffle(perm)
    edges = edges[:]
    rng.shuffle(edges)
    return [(perm[p], perm[c]) if rng.random() < 0.5 else (perm[c], perm[p]) for p, c in edges]


def case_for(seed: int, rng: random.Random) -> tuple[int, list[tuple[int, int]]]:
    if seed >= 1000:
        n = rng.randint(17, 2000) if seed >= 2000 else rng.randint(2, 16)
        shape = rng.choice(SHAPES)
        return n, label(rng, n, tree(rng, n, shape), rng.choice(["sorted", "random"]))
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    n, shape, how = PLANS[seed - len(SAMPLES)]
    return n, label(rng, n, tree(rng, n, shape), how)


def main() -> None:
    seed = int(sys.argv[1])
    n, edges = case_for(seed, random.Random(seed))
    assert 2 <= n <= MAX_N and len(edges) == n - 1
    assert all(1 <= a <= n and 1 <= b <= n and a != b for a, b in edges)
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
    out = [str(n)] + [f"{a} {b}" for a, b in edges]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
