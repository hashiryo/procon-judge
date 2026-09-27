"""abc248-g (GCD cost on the tree) の入力を作る。N、A_1 ... A_N、木の辺 U_i V_i (U_i < V_i) を出す。

seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース、N = 10^5 のいろいろな形の木。
木の形は、ランダム、パス、スター、毛虫、二分木、ほうき、深いランダム。A は、ランダム、約数の多い数
(10^5 以下で約数が 100 個以上のもの。提出は頂点ごとに A の約数の配列を持つので重くなる)、全部 83160
(約数が 128 個)、2520 の倍数、全部 1、大きな素数の数種類、全部 10^5 を混ぜる。
seed が 1000 以上なら、頂点ごとに木をたどって全部の組の gcd を求める愚直解で解ける小さい入力を出す
(pj testdata crosscheck 用)。2000 以上なら、愚直解でまだ解ける N = 2000 までの入力を出す。
"""

import random
import sys

MAX_N = 10**5
MAX_A = 10**5
# 10^5 以下で約数の多い数 (約数の個数が 100 以上)。
RICH = [
    83160, 98280, 55440, 65520, 75600, 85680, 90720, 92400, 95760, 60480, 73920, 87360,
    95040, 50400, 69300, 70560, 79200, 80640, 81900, 88200, 93600, 97020, 45360, 71280,
]
PRIMES = [99991, 99989, 99971, 99961, 99929]

SAMPLES = [
    ([24, 30, 28, 7], [(1, 2), (1, 3), (3, 4)]),
    (
        [180, 168, 120, 144, 192, 200, 198, 160, 156, 150],
        [(1, 2), (2, 3), (2, 4), (2, 5), (5, 6), (4, 7), (7, 8), (7, 9), (9, 10)],
    ),
]
FIXED = [
    ([1, 1], [(1, 2)]),
    ([MAX_A, MAX_A], [(1, 2)]),
    ([83160, 98280], [(1, 2)]),
    ([99991, 99989, 99991], [(1, 2), (2, 3)]),  # 端どうしは gcd が 1
    ([12, 18, 30, 42, 60], [(1, 2), (1, 3), (1, 4), (1, 5)]),
]
# (N, 木の形, A の出し方)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    (1000, "random", "random"),
    (MAX_N, "random", "random"),
    (MAX_N, "random", "rich"),
    (MAX_N, "path", "rich"),
    (MAX_N, "star", "rich"),
    (MAX_N, "caterpillar", "multiples"),
    (MAX_N, "binary", "rich"),
    (MAX_N, "broom", "random"),
    (MAX_N, "deep", "same_rich"),
    (MAX_N, "path", "one"),
    (MAX_N, "random", "primes"),
    (MAX_N, "star", "max"),
]
SHAPES = ["random", "path", "star", "caterpillar", "binary", "broom", "deep"]
VALUES = ["random", "rich", "same_rich", "multiples", "one", "primes", "max", "small"]


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


def values(rng: random.Random, n: int, how: str) -> list[int]:
    if how == "random":
        return [rng.randint(1, MAX_A) for _ in range(n)]
    if how == "rich":
        return [rng.choice(RICH) for _ in range(n)]
    if how == "same_rich":
        return [83160] * n
    if how == "multiples":
        return [2520 * rng.randint(1, MAX_A // 2520) for _ in range(n)]
    if how == "one":
        return [1] * n
    if how == "primes":
        return [rng.choice(PRIMES) for _ in range(n)]
    if how == "max":
        return [MAX_A] * n
    assert how == "small"
    return [rng.randint(1, 12) for _ in range(n)]


def label(rng: random.Random, n: int, edges: list[tuple[int, int]]) -> list[tuple[int, int]]:
    """頂点に 1 から n の番号をランダムに付け、辺の順も混ぜる。"""
    perm = list(range(1, n + 1))
    rng.shuffle(perm)
    edges = [tuple(sorted((perm[p], perm[c]))) for p, c in edges]
    rng.shuffle(edges)
    return edges


def case_for(seed: int, rng: random.Random) -> tuple[list[int], list[tuple[int, int]]]:
    if seed >= 1000:
        n = rng.randint(100, 2000) if seed >= 2000 else rng.randint(2, 60)
        return values(rng, n, rng.choice(VALUES)), label(rng, n, tree(rng, n, rng.choice(SHAPES)))
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    n, shape, how = PLANS[seed - len(FIXED)]
    return values(rng, n, how), label(rng, n, tree(rng, n, shape))


def main() -> None:
    seed = int(sys.argv[1])
    a, edges = case_for(seed, random.Random(seed))
    n = len(a)
    assert 2 <= n <= MAX_N and len(edges) == n - 1 and all(1 <= v <= MAX_A for v in a)
    assert all(1 <= u < v <= n for u, v in edges)
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
    out = [str(n), " ".join(map(str, a))] + [f"{u} {v}" for u, v in edges]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
