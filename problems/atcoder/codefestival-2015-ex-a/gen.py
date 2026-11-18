#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""atcoder-codefestival-2015-ex-a (高橋王国と青木王国) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

seed 0 から 2 は問題文の例。seed 3 から 11 は N = 5000、M = 50000 (seed 11 は N = 100)、Q = 10^5 のケースで、乱択、
1 から N へ層を並べた形 (層の間の辺を乱択に張り、最小カットの候補が多い)、どの辺も 2 本ずつの道 (最小カットが道の
どこでもよい)、1 と N のまわりの 2 つの塊を少ない辺でつないだ形、辺が N 本ほどの疎な乱択、辺の多くが 1 へ向かう形 (流れ
が 0)、往復の組の多い乱択、層ごとに幅 1 の関所を挟んだ形、頂点 100 個に辺 50000 本の多重辺。問いの組は乱択で、半分は
1 と N を含む。頂点の番号は 1 と N を除いて乱択に並べ替える。seed 1000 以上は、愚直解 (brute.hpp) と突き合わせる
小さい入力 (頂点 8 個まで)。
"""
import random
import sys

SAMPLES = [
    """3 5
1 2
1 2
1 2
2 3
2 3
3
1 2
2 3
1 3""",
    """3 5
3 2
3 2
3 2
2 1
2 1
3
1 2
2 3
1 3""",
    """8 10
1 2
1 3
1 4
2 3
3 5
4 6
5 8
6 7
6 8
7 8
7
1 2
2 3
2 6
3 6
4 5
5 6
6 7""",
]


def out(rng: random.Random, n: int, edges: list[tuple[int, int]], q: int, relabel: bool = True) -> str:
    """頂点は 0 から n - 1 (0 が 1、n - 1 が N)。"""
    p = list(range(n))
    if relabel:
        mid = p[1:-1]
        rng.shuffle(mid)
        p = [0] + mid + [n - 1]
    rng.shuffle(edges)
    qs = []
    for _ in range(q):
        c = rng.randrange(n)
        d = rng.choice((0, n - 1)) if rng.random() < 0.5 else rng.randrange(n)
        while d == c:
            d = rng.randrange(n)
        qs.append((c, d) if rng.random() < 0.5 else (d, c))
    lines = [f"{n} {len(edges)}"] + [f"{p[a] + 1} {p[b] + 1}" for a, b in edges] + [str(q)]
    lines += [f"{p[c] + 1} {p[d] + 1}" for c, d in qs]
    return "\n".join(lines)


def rnd_pair(rng: random.Random, lo: int, hi: int) -> tuple[int, int]:
    a = rng.randrange(lo, hi)
    b = rng.randrange(lo, hi - 1)
    return (a, b + (b >= a))


def layered(rng: random.Random, n: int, m: int, width: int, gate: bool) -> list[tuple[int, int]]:
    """頂点 1 から n - 2 を幅 width の層に並べ、0 から最初の層へ、層から次の層へ、最後の層から n - 1 へ辺を張る。gate なら
    層の 10 個ごとに、層の頂点を 1 つだけ残して次の層へつなぐ。"""
    inner = list(range(1, n - 1))
    layers = [inner[i : i + width] for i in range(0, len(inner), width)]
    edges = [(0, v) for v in layers[0]] + [(v, n - 1) for v in layers[-1]]
    for i in range(len(layers) - 1):
        a, b = layers[i], layers[i + 1]
        if gate and i % 10 == 5:
            a = a[:1]
        for v in a:
            edges.append((v, rng.choice(b)))
    while len(edges) < m:
        i = rng.randrange(len(layers) - 1)
        if gate and i % 10 == 5:
            continue
        edges.append((rng.choice(layers[i]), rng.choice(layers[i + 1])))
    return edges


def main_case(seed: int) -> str:
    if seed < 3:
        return SAMPLES[seed]
    rng = random.Random(seed * 7919 + 29)
    n, m, q = 5000, 50000, 10**5
    if seed == 3:
        return out(rng, n, [rnd_pair(rng, 0, n) for _ in range(m)], q)
    if seed == 4:
        return out(rng, n, layered(rng, n, m, 50, False), q)
    if seed == 5:
        e = [(v, v + 1) for v in range(n - 1)] * 2
        e += [(v + rng.randint(1, 3), v) for v in (rng.randrange(n - 4) for _ in range(m - len(e)))]
        return out(rng, n, e, q, relabel=False)
    if seed == 6:
        h = n // 2
        e = [rnd_pair(rng, 0, h) for _ in range(m // 2 - 20)] + [rnd_pair(rng, h, n) for _ in range(m // 2 - 20)]
        e += [(rng.randrange(h), rng.randrange(h, n)) for _ in range(40)]
        return out(rng, n, e, q)
    if seed == 7:
        return out(rng, n, [rnd_pair(rng, 0, n) for _ in range(n)], q)
    if seed == 8:
        e = []
        for _ in range(m):
            a, b = rnd_pair(rng, 0, n)
            e.append((max(a, b), min(a, b)) if rng.random() < 0.95 else (a, b))
        return out(rng, n, e, q)
    if seed == 9:
        e = []
        while len(e) < m:
            a, b = rnd_pair(rng, 0, n)
            e += [(a, b), (b, a)] if rng.random() < 0.5 else [(a, b)]
        return out(rng, n, e[:m], q)
    if seed == 10:
        return out(rng, n, layered(rng, n, m, 20, True), q)
    if seed == 11:
        return out(rng, 100, [rnd_pair(rng, 0, 100) for _ in range(m)], q)
    raise SystemExit(f"seed {seed} は 0 から 11 か、1000 以上にしてください")


def small_case(seed: int) -> str:
    rng = random.Random(seed * 104729 + 7)
    n = rng.randint(2, 8)
    m = rng.randint(1, 14)
    e = [rnd_pair(rng, 0, n) for _ in range(m)]
    qs = rng.randint(1, 20)
    return out(rng, n, e, qs)


def main() -> None:
    seed = int(sys.argv[1])
    print(small_case(seed) if seed >= 1000 else main_case(seed))


if __name__ == "__main__":
    main()
