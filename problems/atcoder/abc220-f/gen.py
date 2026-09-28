"""abc220-f (Distance Sums 2) の入力を作る。N と木の辺 u_i < v_i を出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、N = 2 × 10^5 と 5 × 10^4 の木。
木の形は、ランダム、パス、スター、毛虫、二分木、ほうき、深いランダム。頂点の番号は、ランダムに
付けたものと、形に沿って付けたもの (パスなら端から 1, 2, ..., N、スターなら中心を 1 か N) を混ぜる。
端から順に番号を付けたパスでは、頂点 1 の答えが N(N - 1) / 2 で 2 × 10^10 近くになる (int に収まらない)。
出力が N 行で大きいので、N = 2 × 10^5 は 4 ケースにする。
seed が 1000 以上なら、頂点ごとに幅優先探索する愚直解で解ける小さい木を出す (pj testdata crosscheck 用)。
2000 以上なら、愚直解でまだ解ける N = 2000 までの木を出す。
"""

import random
import sys

MAX_N = 2 * 10**5

SAMPLES = [
    (3, [(1, 2), (2, 3)]),
    (2, [(1, 2)]),
    (6, [(1, 6), (1, 5), (1, 3), (1, 4), (1, 2)]),
]
FIXED = [
    (3, [(1, 3), (2, 3)]),
    (3, [(1, 3), (1, 2)]),
    (4, [(2, 4), (1, 3), (1, 4)]),
    (5, [(4, 5), (3, 4), (2, 3), (1, 2)]),
]
# (N, 木の形, 番号の付け方)。本番のケースのうち角のケースのあとに並べる。
# sorted は形に沿った番号 (頂点 i を i + 1 に)、reversed はその逆順 (頂点 i を N - i に)、random はランダム。
PLANS = [
    (1000, "random", "random"),
    (MAX_N, "random", "random"),
    (MAX_N, "path", "sorted"),
    (MAX_N, "path", "random"),
    (MAX_N, "star", "random"),
    (50000, "caterpillar", "random"),
    (50000, "deep", "random"),
    (50000, "path", "reversed"),
    (50000, "star", "sorted"),  # 中心が 1
    (50000, "star", "reversed"),  # 中心が N
    (50000, "binary", "sorted"),
    (50000, "broom", "random"),
    (50000, "deep", "sorted"),
]
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
    """頂点に 1 から n の番号を付け、辺の順を混ぜる。辺の 2 頂点は小さい番号を先に書く。"""
    if how == "sorted":
        perm = list(range(1, n + 1))
    elif how == "reversed":
        perm = list(range(n, 0, -1))
    else:
        perm = list(range(1, n + 1))
        rng.shuffle(perm)
    out = [tuple(sorted((perm[p], perm[c]))) for p, c in edges]
    rng.shuffle(out)
    return out


def case_for(seed: int, rng: random.Random) -> tuple[int, list[tuple[int, int]]]:
    if seed >= 1000:
        n = rng.randint(50, 2000) if seed >= 2000 else rng.randint(2, 40)
        shape = rng.choice(SHAPES)
        return n, label(rng, n, tree(rng, n, shape), rng.choice(["sorted", "reversed", "random", "random"]))
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    n, shape, how = PLANS[seed - len(FIXED)]
    return n, label(rng, n, tree(rng, n, shape), how)


def main() -> None:
    seed = int(sys.argv[1])
    n, edges = case_for(seed, random.Random(seed))
    assert 2 <= n <= MAX_N and len(edges) == n - 1 and all(1 <= u < v <= n for u, v in edges)
    # 木になっているか (つながっているか) を確かめる。
    parent = list(range(n + 1))

    def find(v: int) -> int:
        while parent[v] != v:
            parent[v] = parent[parent[v]]
            v = parent[v]
        return v

    for u, v in edges:
        ru, rv = find(u), find(v)
        assert ru != rv
        parent[ru] = rv
    out = [str(n)] + [f"{u} {v}" for u, v in edges]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
