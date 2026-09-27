"""past202203-o (3-Permutation) の入力を作る。N M と、M 個の組 A B (A < B、全部違う) を出す。

P_A + P_B が 3 の倍数になるのは、3 で割った余りが (0, 0) か (1, 2) か (2, 1) のとき。連結成分ごとに、
全部を余り 0 にするか、2 部グラフなら片側を 1、反対側を 2 にする。余り 0、1、2 の個数は N / 3、
(N + 2) / 3、(N + 1) / 3 に決まっているので、それに合わせられるかを答える。
seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース、N が 1000 近くのいろいろな形。
角のケースは、N = 2、M = 0、三角形、完全グラフ、スター。
N が 1000 近くの形は、M = 0 (lib.cpp の DP は 2 部グラフの成分の数 × 余り 1 の数 × 余り 2 の数だけ回るので、
孤立点が 1000 個のときが最も遅い)、完全マッチング (N = 1000 では 1 と 2 の個数が合わず No、N = 998 では Yes)、
余りの個数にちょうど合う完全 2 部グラフと 1 ずれたもの、奇閉路の大きさが余り 0 の個数ちょうどと 2 多いもの、
三角形ばかり、スター、パス、森、ランダムなグラフ (M = 2 * 10^5 まで)。
ほかに、余りを先に決めて (0, 0) と (1, 2) の組からだけ辺を選んだもの (答えは Yes) と、それに合わない辺を
1 本足したものを入れる。
seed が 1000 以上なら、頂点ごとに余りを決めていく愚直解で解ける N = 12 までの入力を出す (pj testdata crosscheck 用)。
"""

import random
import sys

MAX_N = 1000
MAX_M = 2 * 10**5

SAMPLES = [
    (5, [(1, 2), (2, 3)]),
    (3, [(1, 2), (2, 3)]),
]


def complete(vs: list[int]) -> list[tuple[int, int]]:
    return [(vs[i], vs[j]) for i in range(len(vs)) for j in range(i + 1, len(vs))]


def cycle(vs: list[int]) -> list[tuple[int, int]]:
    return [(vs[i], vs[(i + 1) % len(vs)]) for i in range(len(vs))]


def path(vs: list[int]) -> list[tuple[int, int]]:
    return [(vs[i], vs[i + 1]) for i in range(len(vs) - 1)]


def bipartite(left: list[int], right: list[int]) -> list[tuple[int, int]]:
    return [(u, v) for u in left for v in right]


def matching(vs: list[int]) -> list[tuple[int, int]]:
    return [(vs[i], vs[i + 1]) for i in range(0, len(vs) - 1, 2)]


FIXED = [
    (2, []),
    (2, [(0, 1)]),
    (3, complete([0, 1, 2])),
    (3, [(0, 1)]),
    (4, []),
    (4, complete(list(range(4)))),
    (6, complete(list(range(6)))),
    (5, [(0, i) for i in range(1, 5)]),  # スター。片側 1 個と 4 個
    (6, cycle(list(range(6)))),
]


def counts(n: int) -> tuple[int, int, int]:
    """余り 0、1、2 の個数。"""
    return n // 3, (n + 2) // 3, (n + 1) // 3


def random_edges(rng: random.Random, n: int, m: int) -> list[tuple[int, int]]:
    if m > n * (n - 1) // 4:
        edges = complete(list(range(n)))
        rng.shuffle(edges)
        return edges[:m]
    seen = set()
    while len(seen) < m:
        u, v = rng.sample(range(n), 2)
        seen.add((min(u, v), max(u, v)))
    return list(seen)


def planted(rng: random.Random, n: int, m: int) -> tuple[list[int], list[tuple[int, int]]]:
    """余りを先に決め、(0, 0) と (1, 2) の組からだけ m 本の辺を選ぶ。答えは Yes。
    そういう組が m 個より少なければ、ある分だけ選ぶ。"""
    z, x, y = counts(n)
    m = min(m, z * (z - 1) // 2 + x * y)
    res = [0] * z + [1] * x + [2] * y
    rng.shuffle(res)
    by = [[v for v in range(n) if res[v] == r] for r in range(3)]
    seen = set()
    while len(seen) < m:
        if rng.random() < 0.3 and len(by[0]) >= 2:
            u, v = rng.sample(by[0], 2)
        else:
            u, v = rng.choice(by[1]), rng.choice(by[2])
        seen.add((min(u, v), max(u, v)))
    return res, list(seen)


def forest(rng: random.Random, n: int, largest: int) -> list[tuple[int, int]]:
    """大きさが 1 から largest の木を並べる。"""
    edges, start = [], 0
    while start < n:
        size = min(n - start, rng.randint(1, largest))
        edges += [(start + rng.randrange(i), start + i) for i in range(1, size)]
        start += size
    return edges


def big_case(rng: random.Random, shape: str) -> tuple[int, list[tuple[int, int]]]:
    n = MAX_N
    vs = list(range(n))
    z, x, y = counts(n)
    if shape == "empty":
        return n, []
    if shape == "matching":
        return n, matching(vs)
    if shape == "matching_998":
        return 998, matching(vs[:998])
    if shape == "kbip_exact":
        return n, bipartite(vs[:x], vs[x:x + y])  # 残りの z 個は孤立点
    if shape == "kbip_off":
        return n, bipartite(vs[:x + 1], vs[x + 1:x + y])
    if shape == "odd_exact":
        # 大きさ 333 の奇閉路 (余り 0 の個数ちょうど) と、残りの完全マッチング。
        return 999, cycle(vs[:333]) + matching(vs[333:999])
    if shape == "odd_over":
        return n, cycle(vs[:z + 2]) + matching(vs[z + 2:])
    if shape == "triangles":
        return 999, [e for i in range(0, 999, 3) for e in cycle(vs[i:i + 3])]
    if shape == "star":
        return n, [(0, v) for v in range(1, n)]
    if shape == "path_fit":
        return n, path(vs[:x + y])  # 片側 334 個と 333 個のパスと孤立点
    if shape == "path_all":
        return n, path(vs)
    if shape == "forest":
        return n, forest(rng, n, 8)
    if shape == "random_sparse":
        return n, random_edges(rng, n, 600)
    if shape == "random_dense":
        return n, random_edges(rng, n, MAX_M)
    if shape == "planted_sparse":
        return n, planted(rng, n, 450)[1]
    if shape == "planted_dense":
        return n, planted(rng, n, 150000)[1]
    assert shape == "planted_break"
    res, edges = planted(rng, n, 450)
    ones = [v for v in range(n) if res[v] == 1]
    u, v = rng.sample(ones, 2)  # 余り 1 どうしを繋ぐ辺を 1 本足す
    return n, edges + [(min(u, v), max(u, v))]


BIG = [
    "empty", "matching", "matching_998", "kbip_exact", "kbip_off", "odd_exact", "odd_over", "triangles",
    "star", "path_fit", "path_all", "forest", "random_sparse", "random_dense", "planted_sparse",
    "planted_dense", "planted_break",
]
COUNT = len(SAMPLES) + len(FIXED) + len(BIG)


def small_case(rng: random.Random) -> tuple[int, list[tuple[int, int]]]:
    n = rng.randint(2, 12)
    kind = rng.randrange(5)
    if kind == 0:
        return n, random_edges(rng, n, rng.randint(0, min(n * (n - 1) // 2, 3 * n)))
    if kind == 1:
        return n, planted(rng, n, rng.randint(0, n))[1]
    if kind == 2:
        _, edges = planted(rng, n, rng.randint(0, n))
        u, v = rng.sample(range(n), 2)
        if (min(u, v), max(u, v)) not in edges:
            edges.append((min(u, v), max(u, v)))
        return n, edges
    if kind == 3:
        return n, forest(rng, n, rng.randint(1, 5))
    vs = list(range(n))
    rng.shuffle(vs)
    k = rng.randint(2, n)
    parts = [cycle(vs[:k]) if k >= 3 else path(vs[:k]), matching(vs[k:])]
    return n, [e for part in parts for e in part]


def case_for(seed: int, rng: random.Random) -> tuple[int, list[tuple[int, int]]]:
    if seed >= 1000:
        return small_case(rng)
    if seed < len(SAMPLES):
        n, edges = SAMPLES[seed]
        return n, [(a - 1, b - 1) for a, b in edges]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    return big_case(rng, BIG[seed - len(FIXED)])


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    n, edges = case_for(seed, rng)
    if seed >= len(SAMPLES):
        # 頂点の番号と辺の順を混ぜる。例はそのまま出す。
        perm = list(range(1, n + 1))
        rng.shuffle(perm)
        edges = [tuple(sorted((perm[u], perm[v]))) for u, v in edges]
        rng.shuffle(edges)
    else:
        edges = [(u + 1, v + 1) for u, v in edges]
    assert 2 <= n <= MAX_N and len(edges) <= min(n * (n - 1) // 2, MAX_M)
    assert all(1 <= a < b <= n for a, b in edges) and len(set(edges)) == len(edges)
    sys.stdout.write("\n".join([f"{n} {len(edges)}"] + [f"{a} {b}" for a, b in edges]) + "\n")


if __name__ == "__main__":
    main()
