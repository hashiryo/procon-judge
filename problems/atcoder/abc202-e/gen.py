"""abc202-e (Count Descendants) の入力を作る。N、親 P_2 ... P_N、Q と Q 個の質問 U D を出す。

seed が 0 から count - 1 までは本番のケース。例の 1 つ、角のケース、N = Q = 2 × 10^5 と
N = Q = 5 × 10^4 のいろいろな形の木。木の形は、ランダム、パス、スター、毛虫、二分木、ほうき、
深いランダム。パス以外は、親が子より小さい番号になる範囲で番号をランダムに付け直す。
質問の D は、U の部分木の中にある深さ (答えが 1 以上になりやすい) を多めにし、U より浅い深さ
(答えは 0)、U 自身の深さ (答えは 1)、0 から N - 1 のランダムを混ぜる。根だけを聞くケースも入れる。
入力が大きい (1 ケース 4 MB ほど) ので、N = Q = 2 × 10^5 は 6 ケースにする。
seed が 1000 以上なら、質問ごとに U の部分木をたどる愚直解で解ける小さい入力を出す
(pj testdata crosscheck 用)。2000 以上なら、愚直解でまだ解ける N = Q = 3000 までの入力を出す。
"""

import random
import sys

MAX_N = 2 * 10**5
MAX_Q = 2 * 10**5
MID = 5 * 10**4

SAMPLES = [
    (7, [1, 1, 2, 2, 4, 2], [(1, 2), (7, 2), (4, 1), (5, 5)]),
]
# 全部の (U, D) を聞く小さい木 (N, 木の形)。
ALL_PAIRS = [(2, "path"), (3, "path"), (3, "star"), (10, "random"), (12, "binary"), (300, "random")]
# (N, Q, 木の形, 質問の出し方)。本番のケースのうち例と角のケースのあとに並べる。
PLANS = [
    (MAX_N, MAX_Q, "random", "mixed"),
    (MAX_N, MAX_Q, "path", "mixed"),
    (MAX_N, MAX_Q, "star", "mixed"),
    (MAX_N, MAX_Q, "caterpillar", "mixed"),
    (MAX_N, MAX_Q, "deep", "mixed"),
    (MAX_N, MAX_Q, "binary", "root"),
    (MID, MID, "random", "root"),
    (MID, MID, "path", "root"),
    (MID, MID, "binary", "mixed"),
    (MID, MID, "broom", "mixed"),
    (MID, MID, "caterpillar", "root"),
    (MID, MID, "deep", "random"),
    (MID, MID, "star", "root"),
    (MID, MID, "broom", "root"),
    (MID, 1, "path", "mixed"),
    (1000, MID, "random", "mixed"),
]
SHAPES = ["random", "path", "star", "caterpillar", "binary", "broom", "deep"]


def tree(rng: random.Random, n: int, shape: str) -> list[int]:
    """頂点 0 から n - 1 の木の親を返す (根は 0 で、親は -1)。親の番号は子より小さい。"""
    if shape == "random":
        return [-1] + [rng.randrange(i) for i in range(1, n)]
    if shape == "path":
        return [-1] + [i - 1 for i in range(1, n)]
    if shape == "star":
        return [-1] + [0] * (n - 1)
    if shape == "caterpillar":
        spine = max(1, n // 2)
        return [-1] + [i - 1 for i in range(1, spine)] + [rng.randrange(spine) for _ in range(spine, n)]
    if shape == "binary":
        return [-1] + [(i - 1) // 2 for i in range(1, n)]
    if shape == "broom":
        handle = max(1, n // 2)
        return [-1] + [i - 1 for i in range(1, handle)] + [handle - 1] * (n - handle)
    assert shape == "deep"
    return [-1] + [rng.randint(max(0, i - 3), i - 1) for i in range(1, n)]


def relabel(rng: random.Random, parent: list[int]) -> list[int]:
    """親が子より前に来る順番をランダムに選び、その順に 0, 1, ... と付け直した親を返す。"""
    n = len(parent)
    children: list[list[int]] = [[] for _ in range(n)]
    for v in range(1, n):
        children[parent[v]].append(v)
    new = [0] * n
    order = []
    ready = [0]
    while ready:
        i = rng.randrange(len(ready))
        ready[i], ready[-1] = ready[-1], ready[i]
        v = ready.pop()
        new[v] = len(order)
        order.append(v)
        ready.extend(children[v])
    return [-1] + [new[parent[v]] for v in order[1:]]


def queries(rng: random.Random, parent: list[int], q: int, how: str) -> list[tuple[int, int]]:
    """質問を (U, D) で返す。U は 1 始まり。"""
    n = len(parent)
    depth = [0] * n
    for v in range(1, n):
        depth[v] = depth[parent[v]] + 1
    # 部分木の中の最も深い頂点の深さ。
    deepest = depth[:]
    for v in range(n - 1, 0, -1):
        deepest[parent[v]] = max(deepest[parent[v]], deepest[v])
    out = []
    for _ in range(q):
        u = 0 if how == "root" else rng.randrange(n)
        kind = rng.random()
        if how == "random":
            d = rng.randrange(n)
        elif kind < 0.7:
            d = rng.randint(depth[u], deepest[u])
        elif kind < 0.8 and depth[u] > 0:
            d = rng.randrange(depth[u])
        elif kind < 0.9:
            d = depth[u]
        else:
            d = rng.randrange(n)
        out.append((u + 1, d))
    return out


def case_for(seed: int, rng: random.Random) -> tuple[list[int], list[tuple[int, int]]]:
    """親の列 (0 始まり、先頭は -1) と質問を返す。"""
    if seed >= 1000:
        # 愚直解が質問ごとに部分木をたどっても速い大きさ。
        limit = 3000 if seed >= 2000 else 40
        n = rng.randint(2, limit)
        shape = rng.choice(SHAPES)
        parent = tree(rng, n, shape)
        if shape != "path" and rng.random() < 0.7:
            parent = relabel(rng, parent)
        how = rng.choice(["mixed", "mixed", "root", "random"])
        return parent, queries(rng, parent, rng.randint(1, limit), how)
    if seed < len(SAMPLES):
        n, p, qs = SAMPLES[seed]
        return [-1] + [x - 1 for x in p], qs
    seed -= len(SAMPLES)
    if seed < len(ALL_PAIRS):
        # 全部の (U, D) を 1 回ずつ聞く。N = 300 なら 90000 個。
        n, shape = ALL_PAIRS[seed]
        parent = tree(rng, n, shape)
        if shape != "path":
            parent = relabel(rng, parent)
        return parent, [(u, d) for u in range(1, n + 1) for d in range(n)]
    seed -= len(ALL_PAIRS)
    if seed == 0:
        # 深さ N - 1 の頂点を根から聞く (パスの端)。
        return tree(rng, MAX_N, "path"), [(1, MAX_N - 1), (MAX_N, MAX_N - 1), (MAX_N, 0), (1, 0)]
    n, q, shape, how = PLANS[seed - 1]
    parent = tree(rng, n, shape)
    if shape != "path":
        parent = relabel(rng, parent)
    return parent, queries(rng, parent, q, how)


COUNT = len(SAMPLES) + len(ALL_PAIRS) + 1 + len(PLANS)


def main() -> None:
    seed = int(sys.argv[1])
    parent, qs = case_for(seed, random.Random(seed))
    n = len(parent)
    assert 2 <= n <= MAX_N and 1 <= len(qs) <= MAX_Q
    assert parent[0] == -1 and all(0 <= parent[v] < v for v in range(1, n))
    assert all(1 <= u <= n and 0 <= d <= n - 1 for u, d in qs)
    out = [str(n), " ".join(str(parent[v] + 1) for v in range(1, n)), str(len(qs))]
    out += [f"{u} {d}" for u, d in qs]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
