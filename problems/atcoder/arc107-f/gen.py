"""arc107-f (Sum of Abs) の入力を作る。N M、A、B と M 本の辺を出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、N = 2 と 3 の角のケース、N = 300 か
M = 300 の大きいもの。グラフは、ランダム、パス、閉路、スター、木、完全グラフ K_25 (辺がちょうど
300 本)、格子、K_8 を鎖のようにつないだものに、足りない辺をランダムに足す。値は、ランダム、A も
|B| も最大 (答えは最大の 3 * 10^8)、B が頂点の順に交互に正負、格子の市松模様、A が 1 で消すのが
安いもの、B が 0、同点だらけの小さい値を混ぜる。頂点の番号と辺の順番と向きは混ぜる。
seed が 1000 以上なら、残す頂点の選び方 2^N 通りを全部試す愚直解で解ける N <= 14 の入力を出す
(pj testdata crosscheck 用)。
"""

import random
import sys

MAX_N = 300
MAX_M = 300
MAX_A = 10**6
MAX_B = 10**6

SAMPLES = [
    ([4, 1, 2, 3], [0, 2, -3, 1], [(1, 2), (2, 3), (3, 4), (4, 2)]),  # 例 1
    (
        [733454, 729489, 956011, 464983, 822120, 364691, 271012, 762026, 751760, 965431],
        [-817837, -880667, -822819, -131079, 740891, 581865, -191711, -383018, 273044, 476880],
        [(3, 1), (4, 1), (6, 9), (3, 8), (1, 6), (10, 5), (5, 6), (1, 5), (4, 3), (7, 1), (7, 4), (10, 3)],
    ),  # 例 2
    ([1, 1, 1, 1], [1, 1, -1, -1], [(1, 2), (3, 4)]),  # 例 3
]
# 小さい角のケース。本番のケースのうち例のすぐあとに並べる。
FIXED = [
    ([1, 1], [0, 0], [(1, 2)]),  # 答えは 0
    ([1, 1], [MAX_B, -MAX_B], [(1, 2)]),  # 片方を消す。答えは 10^6 - 1
    ([MAX_A, MAX_A], [MAX_B, -MAX_B], [(2, 1)]),  # 消しても得にならない。答えは 0
    ([1, 1], [-MAX_B, -MAX_B], [(1, 2)]),  # 答えは 2 * 10^6
    ([1, MAX_A, 1], [MAX_B, 1, -MAX_B], [(1, 2), (2, 3)]),
    ([2, 2, 2], [3, -1, -1], [(1, 2), (2, 3), (3, 1)]),
]
# (N, グラフの形, M, 値の出し方)。M が形の辺より多ければ、残りをランダムに足す。
PLANS = [
    (MAX_N, "random", MAX_M, "random"),
    (MAX_N, "random", MAX_M, "max"),  # 答えは 3 * 10^8
    (MAX_N, "random", MAX_M, "max_neg"),
    (MAX_N, "path", MAX_M, "alternating_cheap"),
    (MAX_N, "cycle", MAX_M, "alternating"),
    (MAX_N, "cycle", MAX_M, "random"),
    (MAX_N, "star", MAX_M, "random"),
    (MAX_N, "star", MAX_M, "alternating_cheap"),
    (MAX_N, "tree", MAX_N - 1, "cheap"),
    (MAX_N, "tree", MAX_M, "ties"),
    (25, "complete", MAX_M, "random"),
    (25, "complete", MAX_M, "alternating_cheap"),
    (150, "grid", MAX_M, "checker"),
    (150, "grid", MAX_M, "cheap"),
    (MAX_N, "cliques", MAX_M, "random"),
    (MAX_N, "random", MAX_M, "zero"),
    (MAX_N, "random", MAX_M, "cheap"),
    (MAX_N, "random", MAX_M, "ties"),
    (60, "random", MAX_M, "random"),
    (100, "random", MAX_M, "signs"),
    (None, "random", None, "random"),  # N と M はランダム
    (None, "tree", None, "signs"),
    (None, "random", None, "ties"),
]
SHAPES = ["random", "path", "cycle", "star", "tree", "complete", "grid", "cliques"]
VALUES = ["random", "max", "max_neg", "alternating", "alternating_cheap", "checker", "cheap", "zero", "ties", "signs"]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)


def base_edges(rng: random.Random, n: int, shape: str) -> list[tuple[int, int]]:
    """形の辺を 0 始まりの番号で返す。"""
    if shape == "random":
        return []
    if shape == "path" or (shape == "cycle" and n < 3):
        return [(i, i + 1) for i in range(n - 1)]
    if shape == "cycle":
        return [(i, (i + 1) % n) for i in range(n)]
    if shape == "star":
        return [(0, i) for i in range(1, n)]
    if shape == "tree":
        return [(rng.randrange(i), i) for i in range(1, n)]
    if shape == "complete":
        return [(u, v) for u in range(n) for v in range(u + 1, n)]
    if shape == "grid":
        # 行の数が 2 か 10 の格子。余った頂点は孤立点。
        rows = 10 if n >= 100 else 2
        cols = n // rows
        cell = lambda r, c: r * cols + c
        edges = [(cell(r, c), cell(r, c + 1)) for r in range(rows) for c in range(cols - 1)]
        return edges + [(cell(r, c), cell(r + 1, c)) for r in range(rows - 1) for c in range(cols)]
    if shape == "cliques":
        # K_8 (小さい N では K_4) を 1 本ずつの辺で鎖のようにつなぐ。
        size = 8 if n >= 80 else 4
        count = min(n // size, 10) or 1
        edges = [(k * size + u, k * size + v) for k in range(count) for u in range(size) for v in range(u + 1, size)]
        edges = [(u, v) for u, v in edges if v < n]
        return edges + [(k * size, (k + 1) * size + 1) for k in range(count - 1)]
    raise ValueError(shape)


def values(rng: random.Random, n: int, how: str) -> tuple[list[int], list[int]]:
    if how == "max":
        return [MAX_A] * n, [MAX_B] * n
    if how == "max_neg":
        return [MAX_A] * n, [-MAX_B] * n
    if how == "ties":
        return [rng.randint(1, 3) for _ in range(n)], [rng.randint(-3, 3) for _ in range(n)]
    if how == "zero":
        return [rng.randint(1, MAX_A) for _ in range(n)], [0] * n
    if how in ("alternating", "alternating_cheap", "checker"):
        # 頂点の順 (格子なら市松模様) に B を +10^6 と -10^6 にする。
        cols = n // 10 if n >= 100 else max(n // 2, 1)
        sign = [(i // cols + i % cols) % 2 if how == "checker" else i % 2 for i in range(n)]
        cost = [1] * n if how == "alternating_cheap" else [rng.randint(1, MAX_A) for _ in range(n)]
        return cost, [MAX_B if s else -MAX_B for s in sign]
    if how == "signs":
        return [rng.randint(1, MAX_A) for _ in range(n)], [rng.choice([MAX_B, -MAX_B]) for _ in range(n)]
    cost = [1] * n if how == "cheap" else [rng.randint(1, MAX_A) for _ in range(n)]
    return cost, [rng.randint(-MAX_B, MAX_B) for _ in range(n)]


def build(rng: random.Random, n: int, shape: str, m: int | None, how: str) -> tuple[list[int], list[int], list[tuple[int, int]]]:
    """形の辺に、M 本になるまでランダムな辺を足す。頂点の番号と辺の順番と向きを混ぜる。"""
    edges = base_edges(rng, n, shape)
    pairs = {(min(u, v), max(u, v)) for u, v in edges}
    most = min(MAX_M, n * (n - 1) // 2)
    m = m if m is not None else rng.randint(max(len(pairs), 1), most)
    assert len(pairs) == len(edges) <= m <= most
    rest = [(u, v) for u in range(n) for v in range(u + 1, n) if (u, v) not in pairs]
    edges += rng.sample(rest, m - len(edges))
    a, b = values(rng, n, how)
    label = list(range(n))
    rng.shuffle(label)
    out_a, out_b = [0] * n, [0] * n
    for i in range(n):
        out_a[label[i]], out_b[label[i]] = a[i], b[i]
    edges = [(label[u] + 1, label[v] + 1) if rng.random() < 0.5 else (label[v] + 1, label[u] + 1) for u, v in edges]
    rng.shuffle(edges)
    return out_a, out_b, edges


def case_for(seed: int, rng: random.Random) -> tuple[list[int], list[int], list[tuple[int, int]]]:
    if seed >= 1000:
        # 2^N 通りを全部試せる大きさ。形と値の出し方は本番と同じものから選ぶ。
        n = rng.randint(2, 14)
        return build(rng, n, rng.choice(SHAPES), None, rng.choice(VALUES))
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    n, shape, m, how = PLANS[seed - len(FIXED)]
    return build(rng, n or rng.randint(2, MAX_N), shape, m, how)


def main() -> None:
    seed = int(sys.argv[1])
    a, b, edges = case_for(seed, random.Random(seed))
    n, m = len(a), len(edges)
    assert 1 <= n <= MAX_N and len(b) == n and 1 <= m <= MAX_M
    assert all(1 <= x <= MAX_A for x in a) and all(-MAX_B <= x <= MAX_B for x in b)
    assert all(1 <= u <= n and 1 <= v <= n and u != v for u, v in edges)
    assert len({(min(u, v), max(u, v)) for u, v in edges}) == m
    out = [f"{n} {m}", " ".join(map(str, a)), " ".join(map(str, b))] + [f"{u} {v}" for u, v in edges]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
