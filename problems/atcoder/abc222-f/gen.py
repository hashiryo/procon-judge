"""abc222-f (Expensive Expense) の入力を作る。N、辺 A_i B_i C_i、D_1 ... D_N を出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、N = 2 × 10^5 と 3 × 10^4 の
いろいろな形の木。木の形は、ランダム、パス、スター、毛虫、二分木、ほうき、深いランダム。
頂点の番号、辺の順、辺の両端の順はランダムに混ぜる。
重みは、ランダム、全部 10^9、小さい値 (同点だらけ)、全部 1、D が 1 から 3 頂点だけ大きい、C だけ大きい、
を混ぜる。D が 1 頂点だけ大きいと、ほかの頂点の答えはその頂点になり、その頂点自身の答えだけは自分を
除いて決まる (j ≠ i を忘れると間違える)。10^9 のパスでは答えが 3 × 10^13 ほどになる (int に収まらない)。
入力と出力が大きい (10^9 の値なら 1 ケース 10 MB ほど) ので、N = 2 × 10^5 は小さい重みの 4 ケースにする。
seed が 1000 以上なら、頂点ごとに木全体をたどる愚直解で解ける小さい木を出す (pj testdata crosscheck 用)。
2000 以上なら、愚直解でまだ解ける N = 2000 までの木を出す。
"""

import random
import sys

MAX_N = 2 * 10**5
MID = 3 * 10**4
MAX_V = 10**9

# (N, 辺 (A, B, C), D)
SAMPLES = [
    (3, [(1, 2, 2), (2, 3, 3)], [1, 2, 3]),
    (6, [(1, 2, 3), (1, 3, 1), (1, 4, 4), (1, 5, 1), (1, 6, 5)], [9, 2, 6, 5, 3, 100]),
    (6, [(i, i + 1, MAX_V) for i in range(1, 6)], [1, 2, 3, 4, 5, 6]),
]
FIXED = [
    (2, [(1, 2, 1)], [1, 1]),
    (2, [(2, 1, MAX_V)], [MAX_V, 1]),
    (3, [(1, 2, 1), (1, 3, 1)], [MAX_V, 1, 1]),  # 中心の D だけ大きい
    (3, [(1, 2, 1), (3, 1, 1)], [1, 1, MAX_V]),  # 葉の D だけ大きい
    (4, [(1, 2, MAX_V), (2, 3, MAX_V), (3, 4, MAX_V)], [MAX_V] * 4),
]
# (N, 木の形, 重みの出し方)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    (MAX_N, "path", "small"),
    (MAX_N, "star", "small"),
    (MAX_N, "random", "small"),
    (MAX_N, "caterpillar", "one"),
    (MID, "path", "max"),
    (MID, "random", "random"),
    (MID, "caterpillar", "random"),
    (MID, "broom", "random"),
    (MID, "deep", "big_c"),
    (MID, "star", "big_d"),
    (MID, "binary", "random"),
    (MID, "binary", "big_d"),
    (MID, "random", "big_d"),
    (1000, "random", "random"),
    (1000, "star", "max"),
]
SHAPES = ["random", "path", "star", "caterpillar", "binary", "broom", "deep"]
WEIGHTS = ["random", "max", "small", "one", "big_d", "big_c"]


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


def weights(rng: random.Random, n: int, how: str) -> tuple[list[int], list[int]]:
    """辺の C (n - 1 個) と頂点の D (n 個) を返す。"""
    if how == "random":
        return [rng.randint(1, MAX_V) for _ in range(n - 1)], [rng.randint(1, MAX_V) for _ in range(n)]
    if how == "max":
        return [MAX_V] * (n - 1), [MAX_V] * n
    if how == "small":
        return [rng.randint(1, 3) for _ in range(n - 1)], [rng.randint(1, 3) for _ in range(n)]
    if how == "one":
        return [1] * (n - 1), [1] * n
    if how == "big_d":
        d = [rng.randint(1, 10) for _ in range(n)]
        for v in rng.sample(range(n), rng.randint(1, min(3, n))):
            d[v] = MAX_V - rng.randint(0, 5)
        return [rng.randint(1, 10) for _ in range(n - 1)], d
    assert how == "big_c"
    return [MAX_V - rng.randint(0, 10) for _ in range(n - 1)], [rng.randint(1, 10) for _ in range(n)]


def label(rng: random.Random, n: int, edges: list[tuple[int, int]], c: list[int]) -> list[tuple[int, int, int]]:
    """頂点に 1 から n の番号をランダムに付け、辺の順と辺の両端の順を混ぜる。"""
    perm = list(range(1, n + 1))
    rng.shuffle(perm)
    out = [(perm[p], perm[q], w) if rng.random() < 0.5 else (perm[q], perm[p], w) for (p, q), w in zip(edges, c)]
    rng.shuffle(out)
    return out


def case_for(seed: int, rng: random.Random) -> tuple[int, list[tuple[int, int, int]], list[int]]:
    if seed >= 1000:
        n = rng.randint(50, 2000) if seed >= 2000 else rng.randint(2, 40)
        c, d = weights(rng, n, rng.choice(WEIGHTS))
        return n, label(rng, n, tree(rng, n, rng.choice(SHAPES)), c), d
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    n, shape, how = PLANS[seed - len(FIXED)]
    c, d = weights(rng, n, how)
    return n, label(rng, n, tree(rng, n, shape), c), d


COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)


def main() -> None:
    seed = int(sys.argv[1])
    n, edges, d = case_for(seed, random.Random(seed))
    assert 2 <= n <= MAX_N and len(edges) == n - 1 and len(d) == n
    assert all(1 <= a <= n and 1 <= b <= n and 1 <= w <= MAX_V for a, b, w in edges)
    assert all(1 <= x <= MAX_V for x in d)
    # 木になっているか (つながっているか) を確かめる。
    parent = list(range(n + 1))

    def find(v: int) -> int:
        while parent[v] != v:
            parent[v] = parent[parent[v]]
            v = parent[v]
        return v

    for a, b, _ in edges:
        ra, rb = find(a), find(b)
        assert ra != rb
        parent[ra] = rb
    out = [str(n)] + [f"{a} {b} {w}" for a, b, w in edges] + [" ".join(map(str, d))]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
