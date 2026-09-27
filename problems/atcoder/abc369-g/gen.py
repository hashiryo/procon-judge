"""abc369-g (As far as possible) の入力を作る。N と木の辺 U_i V_i L_i (U_i < V_i) を出す。

seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース、N = 2 × 10^5、10^5、5 × 10^4 の木。
木の形は、ランダム、1 から伸びるパス、1 が真ん中にあるパス、1 が中心のスター、1 が葉のスター、毛虫、
二分木、ほうき、深いランダム。提出は頂点 1 から再帰で木をたどるので、1 から伸びるパスで再帰が最も深くなる。
深さ 2 × 10^5 の再帰は macOS のスタック (64 MB) を超えて参照実装が落ちるので、1 から伸びる道は 15 万までにし、
N = 2 × 10^5 では残りの頂点を道の先に付けたほうきにする。
長さは、10^9 までのランダム、1000 まで、全部 1 (同点だらけ)、全部 10^9 (答えは 4 × 10^14 まで) を混ぜる。
出力が N 行と大きいので、N = 2 × 10^5 は 2 ケースにして、長さの幅も狭くする。
seed が 1000 以上なら、選ぶ頂点の集合を全部試す愚直解で解ける N = 12 までの木を出す (pj testdata crosscheck 用)。
2000 以上なら、愚直解でまだ解ける N = 20 までの木を出す。
"""

import random
import sys

MAX_N = 2 * 10**5
MAX_L = 10**9
DEEP = 150000  # 1 から伸びる道の長さの上限。macOS のスタックで参照実装が落ちない長さ

Case = tuple[int, list[tuple[int, int, int]]]

SAMPLES: list[Case] = [
    (5, [(1, 2, 3), (2, 3, 5), (2, 4, 2), (1, 5, 3)]),
    (3, [(1, 2, MAX_L), (2, 3, MAX_L)]),
]
FIXED: list[Case] = [
    (2, [(1, 2, 1)]),
    (2, [(1, 2, MAX_L)]),
    (3, [(1, 2, 5), (1, 3, 7)]),
    (3, [(1, 3, 4), (2, 3, 4)]),  # 1 が葉
    (4, [(1, 2, 1), (2, 3, 1), (2, 4, 1)]),
]
# (N, 木の形, 長さの作り方)。本番のケースのうち角のケースのあとに並べる。
# root_path は 1 から伸びるパス (番号も端から順)、mid_path は 1 が真ん中のパス、leaf_star は 1 が葉のスター、
# long_broom は 1 から DEEP 頂点の道を伸ばして残りを先に付けたもの。
PLANS = [
    (MAX_N, "long_broom", "narrow"),
    (MAX_N, "random", "narrow"),
    (10**5, "star", "max"),
    (50000, "caterpillar", "wide"),
    (50000, "binary", "wide"),
    (50000, "broom", "wide"),
    (50000, "deep", "wide"),
    (50000, "leaf_star", "wide"),
    (50000, "mid_path", "wide"),
    (50000, "random", "one"),
    (1000, "random", "wide"),
    (1000, "root_path", "max"),
]
SHAPES = ["random", "root_path", "mid_path", "star", "leaf_star", "caterpillar", "binary", "broom", "deep"]


def tree(rng: random.Random, n: int, shape: str) -> list[tuple[int, int]]:
    """頂点 0 から n - 1 の木の辺を (親, 子) で返す。頂点 0 が問題の頂点 1 になる。"""
    if shape == "random":
        return [(rng.randrange(i), i) for i in range(1, n)]
    if shape == "root_path":
        assert n <= DEEP
        return [(i - 1, i) for i in range(1, n)]
    if shape == "long_broom":
        handle = min(n, DEEP)
        return [(i - 1, i) for i in range(1, handle)] + [(handle - 1, i) for i in range(handle, n)]
    if shape == "mid_path":
        # 0 から 2 本のパスを伸ばす。
        half = n // 2
        return [(0 if i == 1 else i - 1, i) for i in range(1, half + 1)] + [
            (0 if i == half + 1 else i - 1, i) for i in range(half + 1, n)
        ]
    if shape == "star":
        return [(0, i) for i in range(1, n)]
    if shape == "leaf_star":
        return [(0, 1)] + [(1, i) for i in range(2, n)]
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


def length(rng: random.Random, how: str) -> int:
    if how == "wide":
        return rng.randint(1, MAX_L)
    if how == "narrow":
        return rng.randint(1, 1000)
    if how == "one":
        return 1
    if how == "max":
        return MAX_L
    assert how == "mixed"
    return rng.choice([1, 2, 3, MAX_L, rng.randint(1, 10), rng.randint(1, MAX_L)])


def build(rng: random.Random, n: int, shape: str, how: str) -> Case:
    edges = tree(rng, n, shape)
    # 頂点 0 は 1 のまま、ほかの番号は形に沿ったもの (root_path) かランダム。
    perm = list(range(1, n + 1))
    if shape != "root_path":
        rest = perm[1:]
        rng.shuffle(rest)
        perm = [1] + rest
    out = [(min(perm[p], perm[c]), max(perm[p], perm[c]), length(rng, how)) for p, c in edges]
    rng.shuffle(out)
    return n, out


def case_for(seed: int, rng: random.Random) -> Case:
    if seed >= 1000:
        n = rng.randint(13, 20) if seed >= 2000 else rng.randint(2, 12)
        return build(rng, n, rng.choice(SHAPES), rng.choice(["wide", "narrow", "one", "max", "mixed"]))
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    return build(rng, *PLANS[seed - len(FIXED)])


def main() -> None:
    seed = int(sys.argv[1])
    n, edges = case_for(seed, random.Random(seed))
    assert 2 <= n <= MAX_N and len(edges) == n - 1
    assert all(1 <= u < v <= n and 1 <= w <= MAX_L for u, v, w in edges)
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
