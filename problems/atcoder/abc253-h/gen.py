"""abc253-h (We Love Forest) の入力を作る。N M と、M 本の辺 u v を出す。多重辺もある。

seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース、N = 14 で M が大きいもの。
角のケースは、N = 2、木 (パス、スター)、全部の辺が同じ 2 点を結ぶもの (K >= 2 で確率 0)、
つながっていないグラフ (大きい K で確率 0)、三角形、完全グラフ。M = 500 では、完全グラフに多重度を
付けたもの、閉路、スター、半分ずつの完全グラフを 1 本の橋でつないだものなどを入れる。
seed が 1000 以上なら、辺を 1 本ずつ見て連結成分の分け方を状態にする DP の愚直解で解ける小さい入力を
出す (pj testdata crosscheck 用)。2000 以上なら、愚直解でまだ解ける N <= 10、M <= 200 の入力を出す。
"""

import random
import sys

MAX_N = 14
MAX_M = 500

SAMPLES = [
    (3, [(1, 2), (2, 3)]),
    (4, [(1, 2), (1, 2), (1, 4), (2, 3), (2, 4)]),
]
# (N, M, グラフの形)。本番のケースのうち例のあとに並べる。
PLANS = [
    (2, 1, "random"),
    (2, MAX_M, "random"),
    (3, 3, "triangle"),
    (MAX_N, 13, "path"),
    (MAX_N, 26, "star"),  # 13 本の木なら形によらず答えが同じなので、スターは多重辺を足す
    (MAX_N, 13, "same_pair"),
    (MAX_N, MAX_M, "same_pair"),
    (MAX_N, 20, "random"),
    (MAX_N, 91, "complete"),
    (MAX_N, MAX_M, "random"),
    (MAX_N, MAX_M, "random"),
    (MAX_N, MAX_M, "cycle"),
    (MAX_N, MAX_M, "star"),
    (MAX_N, MAX_M, "path"),
    (MAX_N, MAX_M, "half"),  # 頂点の半分だけで辺を張る。K >= 7 で確率 0
    (MAX_N, MAX_M, "bridge"),
    (13, 499, "random"),
    (10, 300, "random"),
    (8, 7, "path"),
    (5, MAX_M, "random"),
]
COUNT = len(SAMPLES) + len(PLANS)
KINDS = ["random", "random", "path", "star", "cycle", "same_pair", "half", "bridge", "complete"]


def edges_for(rng: random.Random, n: int, m: int, kind: str) -> list[tuple[int, int]]:
    """頂点 1 から n、m 本の辺。形ごとに使える頂点の組を決め、足りない分はその中からランダムに足す。"""
    if kind == "triangle":
        pairs = [(1, 2), (2, 3), (1, 3)]
    elif kind == "path":
        pairs = [(i, i + 1) for i in range(1, n)]
    elif kind == "star":
        pairs = [(1, i) for i in range(2, n + 1)]
    elif kind == "cycle":
        pairs = [(i, i + 1) for i in range(1, n)] + ([(1, n)] if n >= 3 else [])
    elif kind == "same_pair":
        pairs = [(1, 2)]
    elif kind == "half":
        h = max(2, n // 2)
        pairs = [(a, b) for a in range(1, h + 1) for b in range(a + 1, h + 1)]
    elif kind == "bridge":
        h = max(1, n // 2)
        left = [(a, b) for a in range(1, h + 1) for b in range(a + 1, h + 1)]
        right = [(a, b) for a in range(h + 1, n + 1) for b in range(a + 1, n + 1)]
        pairs = left + right
    else:
        pairs = [(a, b) for a in range(1, n + 1) for b in range(a + 1, n + 1)]
    if kind == "bridge":
        # 橋 (h, h + 1) は 1 本だけにする。
        out = [(h, h + 1)] + [rng.choice(pairs) for _ in range(m - 1)] if pairs else [(h, h + 1)] * m
    elif kind in ("path", "star", "cycle", "complete", "triangle") and m >= len(pairs):
        out = pairs + [rng.choice(pairs) for _ in range(m - len(pairs))]
    else:
        out = [rng.choice(pairs) for _ in range(m)]
    rng.shuffle(out)
    return [(a, b) if rng.random() < 0.5 else (b, a) for a, b in out]


def case_for(seed: int, rng: random.Random) -> tuple[int, list[tuple[int, int]]]:
    if seed >= 1000:
        if seed >= 2000:
            n = rng.randint(8, 10)
            m = rng.randint(n - 1, 200)
        else:
            n = rng.randint(2, 7)
            m = rng.randint(n - 1, 30)
        return n, edges_for(rng, n, m, rng.choice(KINDS))
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    n, m, kind = PLANS[seed - len(SAMPLES)]
    return n, edges_for(rng, n, m, kind)


def main() -> None:
    seed = int(sys.argv[1])
    n, edges = case_for(seed, random.Random(seed))
    assert 2 <= n <= MAX_N and n - 1 <= len(edges) <= MAX_M
    assert all(1 <= u <= n and 1 <= v <= n and u != v for u, v in edges)
    out = [f"{n} {len(edges)}"] + [f"{u} {v}" for u, v in edges]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
