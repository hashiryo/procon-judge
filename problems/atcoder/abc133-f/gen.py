"""abc133-f (Colorful Tree) の入力を作る。N Q、辺 a b c d、質問 x y u v を出す。

seed が 0 から count - 1 までは本番のケース。例の 1 つ、角のケース、N = 10^5 のいろいろな形の木。
木の形は、ランダム、パス、スター、毛虫、二分木、ほうき、深いランダム。色は、ランダム、全部違う、
数色だけ、全部同じ (N - 1、永続配列の添字が最も大きい色) を混ぜる。質問の色は半分を木にある辺の色にする。
パスは頂点 1 を端に置いて、辺を全部同じ色、長さ 10^4 にし、質問は u = 1 と v = N、両方とも深い組、
遠い組を多めにする。答えは 10^9 近くになり、提出が int で持つ途中の値 (根からの長さの和や、
それに y と本数の積を足したもの) も 2 x 10^9 近くになる。
seed が 1000 以上なら、質問ごとに u から木をたどって道を足す愚直解で解ける小さい入力を出す
(pj testdata crosscheck 用)。
"""

import random
import sys

MAX_N = 10**5
MAX_Q = 10**5
MAX_D = 10**4

SAMPLES = [
    (
        5,
        [(1, 2, 1, 10), (1, 3, 2, 20), (2, 4, 4, 30), (5, 2, 1, 40)],
        [(1, 100, 1, 4), (1, 100, 1, 5), (3, 1000, 3, 4)],
    ),
]
# (N, Q, 木の形, 番号の付け方, 色, 長さ, 質問)。本番のケースのうち例のあとに並べる。
# 質問の all は、全部の組と全部の色を 1 回ずつ聞く。far は頂点 1 が端にあるパス用で、u = 1 と v = N、
# 両方とも末尾の 1 割、u が先頭の 1 割で v が末尾の 1 割、ランダムな組を混ぜる。
PLANS = [
    (2, 1, "path", "sorted", "one", "random", "random"),
    (3, 0, "path", "random", "random", "random", "all"),
    (30, 0, "random", "random", "few", "random", "all"),  # 435 組 x 29 色
    (MAX_N, 1, "random", "random", "random", "random", "random"),
    (MAX_N, MAX_Q, "random", "random", "random", "random", "random"),
    (MAX_N, MAX_Q, "path", "sorted", "one", "max", "far"),
    (MAX_N, MAX_Q, "deep", "random", "distinct", "random", "random"),
    (MAX_N, 50000, "caterpillar", "random", "few", "mixed", "random"),
    (MAX_N, 20000, "star", "random", "random", "random", "random"),
    (MAX_N, 20000, "binary", "random", "one", "random", "random"),
    (MAX_N, 20000, "broom", "random", "small", "max", "random"),
]
COUNT = len(SAMPLES) + len(PLANS)
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
    """頂点に 1 から n の番号を付ける。sorted は番号も辺の順も向きもそのまま、random は全部混ぜる。"""
    if how == "sorted":
        return [(p + 1, c + 1) for p, c in edges]
    perm = list(range(1, n + 1))
    rng.shuffle(perm)
    edges = [(perm[p], perm[c]) if rng.random() < 0.5 else (perm[c], perm[p]) for p, c in edges]
    rng.shuffle(edges)
    return edges


def colors(rng: random.Random, n: int, how: str) -> list[int]:
    m = n - 1
    if how == "random":
        return [rng.randint(1, m) for _ in range(m)]
    if how == "distinct":
        c = list(range(1, m + 1))
        rng.shuffle(c)
        return c
    if how == "few":
        palette = [rng.randint(1, m) for _ in range(rng.randint(1, 5))]
        return [rng.choice(palette) for _ in range(m)]
    if how == "small":
        return [rng.randint(1, min(m, 3)) for _ in range(m)]
    assert how == "one"
    return [m] * m


def lengths(rng: random.Random, m: int, how: str) -> list[int]:
    if how == "random":
        return [rng.randint(1, MAX_D) for _ in range(m)]
    if how == "max":
        return [MAX_D] * m
    assert how == "mixed"
    return [rng.choice([1, MAX_D, rng.randint(1, MAX_D)]) for _ in range(m)]


def queries(rng: random.Random, n: int, q: int, edge_colors: list[int], how: str) -> list[tuple[int, int, int, int]]:
    def color() -> int:
        return rng.choice(edge_colors) if rng.random() < 0.5 else rng.randint(1, n - 1)

    def length() -> int:
        return rng.choice([1, MAX_D, rng.randint(1, MAX_D), rng.randint(1, MAX_D)])

    if how == "all":
        return [(x, length(), u, v) for u in range(1, n + 1) for v in range(u + 1, n + 1) for x in range(1, n)]
    if how == "far":
        tenth = max(1, n // 10)
        out = [(rng.choice(edge_colors), MAX_D, 1, n) for _ in range(10)]
        out += [(color(), length(), n - 1, n) for _ in range(10)]
        for _ in range(q - 20):
            kind = rng.randrange(3)
            if kind == 0:
                u, v = rng.randint(1, tenth), rng.randint(n - tenth + 1, n)
            elif kind == 1 and tenth >= 2:
                u, v = sorted(rng.sample(range(n - tenth + 1, n + 1), 2))
            else:
                u, v = sorted(rng.sample(range(1, n + 1), 2))
            out.append((color(), rng.choice([MAX_D, length()]), u, v))
        return [(x, y, u, v) if u < v else (x, y, 1, n) for x, y, u, v in out]
    out = []
    for _ in range(q):
        u, v = sorted(rng.sample(range(1, n + 1), 2))
        out.append((color(), length(), u, v))
    return out


def case_for(seed: int, rng: random.Random):
    if seed >= 1000:
        # 愚直解が質問ごとに木全体をたどっても速い大きさ。
        n = rng.randint(2, 50)
        shape = rng.choice(SHAPES)
        how = rng.choice(["sorted", "random"])
        how_c = rng.choice(["random", "distinct", "few", "small", "one"])
        how_d = rng.choice(["random", "max", "mixed"])
        how_q = "all" if n <= 8 and rng.random() < 0.3 else rng.choice(["random", "far"])
        q = rng.randint(10, 50)
    elif seed < len(SAMPLES):
        return SAMPLES[seed]
    else:
        n, q, shape, how, how_c, how_d, how_q = PLANS[seed - len(SAMPLES)]
    ends = label(rng, n, tree(rng, n, shape), how)
    c, d = colors(rng, n, how_c), lengths(rng, n - 1, how_d)
    edges = [(a, b, ci, di) for (a, b), ci, di in zip(ends, c, d)]
    return n, edges, queries(rng, n, q, c, how_q)


def main() -> None:
    seed = int(sys.argv[1])
    n, edges, qs = case_for(seed, random.Random(seed))
    assert 2 <= n <= MAX_N and 1 <= len(qs) <= MAX_Q and len(edges) == n - 1
    assert all(1 <= a <= n and 1 <= b <= n and 1 <= c <= n - 1 and 1 <= d <= MAX_D for a, b, c, d in edges)
    assert all(1 <= x <= n - 1 and 1 <= y <= MAX_D and 1 <= u < v <= n for x, y, u, v in qs)
    # 木になっているか (つながっているか) を確かめる。
    parent = list(range(n + 1))

    def find(v: int) -> int:
        while parent[v] != v:
            parent[v] = parent[parent[v]]
            v = parent[v]
        return v

    for a, b, _, _ in edges:
        ra, rb = find(a), find(b)
        assert ra != rb
        parent[ra] = rb
    out = [f"{n} {len(qs)}"] + [" ".join(map(str, e)) for e in edges] + [" ".join(map(str, q)) for q in qs]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
