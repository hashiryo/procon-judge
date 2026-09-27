"""arc097-d (Monochrome Cat) の入力を作る。N、木の辺、色の列 c を出す。

seed が 0 から count - 1 までは本番のケース。例の 4 つ、角のケース、N = 1000 と 10^5 のいろいろな形の木と色。
木の形は、ランダム、パス、スター、毛虫、二分木、ほうき、深いランダム。色は、ランダム、全部白、全部黒
(答えは 0)、白がまばら (黒い葉を削ると小さい木が残る)、白が多い、葉だけ白、パスの上で交互。
Rerooting は頂点ごとに部分木の値を持ち直すので、パスでは列が長く、スターでは子が多くなる。
seed が 1000 以上なら、(猫の位置, 全頂点の色) の状態を幅優先探索する愚直解で解ける小さい木を出す
(pj testdata crosscheck 用)。2000 以上なら、愚直解でまだ解ける N = 12 から 16 の木を出す。
"""

import random
import sys

MAX_N = 10**5

SAMPLES = [
    (5, [(1, 2), (2, 3), (2, 4), (4, 5)], "WBBWW"),
    (6, [(3, 1), (4, 5), (2, 6), (6, 1), (3, 4)], "WWBWBB"),
    (1, [], "B"),
    (
        20,
        [
            (2, 19), (5, 13), (6, 4), (15, 6), (12, 19), (13, 19), (3, 11), (8, 3), (3, 20), (16, 13),
            (7, 14), (3, 17), (7, 8), (10, 20), (11, 9), (8, 18), (8, 2), (10, 1), (6, 13),
        ],
        "WBWBWBBWWWBBWWBBBBBW",
    ),
]
# (N, 木の形, 色の付け方)。本番のケースのうち例のあとに並べる。
PLANS = [
    (1, "path", "white"),
    (2, "path", "white"),
    (2, "path", "black"),
    (2, "path", "random"),
    (3, "star", "white"),
    (3, "path", "leaves"),
    (1000, "random", "random"),
    (1000, "random", "black"),
    (1000, "star", "white"),
    (1000, "path", "sparse"),
    (1000, "caterpillar", "leaves"),
    (1000, "path", "alternate"),
    (MAX_N, "random", "random"),
    (MAX_N, "path", "random"),
    (MAX_N, "star", "random"),
    (MAX_N, "caterpillar", "random"),
    (MAX_N, "binary", "random"),
    (MAX_N, "broom", "random"),
    (MAX_N, "deep", "random"),
    (MAX_N, "random", "white"),
    (MAX_N, "path", "white"),
    (MAX_N, "random", "sparse"),
    (MAX_N, "random", "dense"),
]
COUNT = len(SAMPLES) + len(PLANS)
SHAPES = ["random", "path", "star", "caterpillar", "binary", "broom", "deep"]
COLORINGS = ["random", "white", "black", "sparse", "dense", "leaves", "alternate"]


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


def coloring(rng: random.Random, n: int, edges: list[tuple[int, int]], how: str) -> list[str]:
    """頂点 0 から n - 1 の色。"""
    if how == "random":
        return [rng.choice("WB") for _ in range(n)]
    if how == "white":
        return ["W"] * n
    if how == "black":
        return ["B"] * n
    if how == "sparse":
        # 白は数個だけ。黒い葉を削ると、白をつなぐ小さい木が残る。
        c = ["B"] * n
        for v in rng.sample(range(n), min(n, rng.randint(2, 6))):
            c[v] = "W"
        return c
    if how == "dense":
        return ["W" if rng.random() < 0.9 else "B" for _ in range(n)]
    if how == "leaves":
        degree = [0] * n
        for p, q in edges:
            degree[p] += 1
            degree[q] += 1
        return ["W" if degree[v] <= 1 else "B" for v in range(n)]
    assert how == "alternate"
    return ["W" if v % 2 == 0 else "B" for v in range(n)]


def build(rng: random.Random, n: int, shape: str, how: str) -> tuple[int, list[tuple[int, int]], str]:
    edges = tree(rng, n, shape)
    colors = coloring(rng, n, edges, how)
    # 頂点の番号を混ぜ、辺の順と向きも混ぜる。
    perm = list(range(1, n + 1))
    rng.shuffle(perm)
    labeled = [(perm[p], perm[q]) if rng.random() < 0.5 else (perm[q], perm[p]) for p, q in edges]
    rng.shuffle(labeled)
    c = [""] * n
    for v in range(n):
        c[perm[v] - 1] = colors[v]
    return n, labeled, "".join(c)


def case_for(seed: int, rng: random.Random) -> tuple[int, list[tuple[int, int]], str]:
    if seed >= 1000:
        # 愚直解が N * 2^N の状態を探索できる大きさ。
        n = rng.randint(12, 16) if seed >= 2000 else rng.randint(1, 11)
        return build(rng, n, rng.choice(SHAPES), rng.choice(COLORINGS + ["random"] * 3))
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    n, shape, how = PLANS[seed - len(SAMPLES)]
    return build(rng, n, shape, how)


def main() -> None:
    seed = int(sys.argv[1])
    n, edges, c = case_for(seed, random.Random(seed))
    assert 1 <= n <= MAX_N and len(edges) == n - 1 and len(c) == n and set(c) <= set("WB")
    assert all(1 <= x <= n and 1 <= y <= n and x != y for x, y in edges)
    # 木になっているか (つながっているか) を確かめる。
    parent = list(range(n + 1))

    def find(v: int) -> int:
        while parent[v] != v:
            parent[v] = parent[parent[v]]
            v = parent[v]
        return v

    for x, y in edges:
        rx, ry = find(x), find(y)
        assert rx != ry
        parent[rx] = ry
    out = [str(n)] + [f"{x} {y}" for x, y in edges] + [c]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
