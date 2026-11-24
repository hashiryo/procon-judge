#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""dmoj-dmopc19c3p6 (Triple Spanning Tree) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

2 ≤ N ≤ 500、1 ≤ M ≤ 1500、多重辺あり、連結とは限らない (問題文の約束)。seed 0 と 1 は問題文の例。seed 2 から 13 は、
一様な乱択の全域木 3 本と乱択の辺 (N = 500)、その全域木の辺を 1 本抜き乱択の辺を足した形 (足した辺が穴を埋める)、乱択の辺だけ (次数が小さく -1)、完全グラフ K_55
(辺 1485 本)、N = 30 の多重辺、N = 2 の多重辺、2 つの成分、格子に平行辺を足した形、2 本の全域木に 1 本の森を足した形、
N = 400 の全域木 3 本の辺をちょうど並べた形、全域木 5 本の辺、全域木 3 本から辺を 3 本抜いた形。辺の順と各辺の向きは乱択に並べ替える。
seed 1000 以上は、愚直解 (brute.hpp) と突き合わせる小さい入力 (N は 2 から 6、M は 9 まで)。
"""
import random
import sys

SAMPLE1 = """4 10
1 2
1 3
2 4
3 4
4 1
3 2
4 2
1 4
2 3
3 1"""

SAMPLE2 = """5 4
1 2
2 3
1 3
3 1"""


def out(rng: random.Random, n: int, es: list[tuple[int, int]]) -> str:
    assert 2 <= n <= 500 and 1 <= len(es) <= 1500, (n, len(es))
    es = [(u, v) if rng.random() < 0.5 else (v, u) for u, v in es]
    rng.shuffle(es)
    return "\n".join([f"{n} {len(es)}"] + [f"{u + 1} {v + 1}" for u, v in es])


def uniform_tree(rng: random.Random, n: int, verts: list[int] | None = None) -> list[tuple[int, int]]:
    """verts の上の一様な乱択の全域木 (Aldous-Broder)。"""
    vs = verts if verts is not None else list(range(n))
    k = len(vs)
    seen = {vs[rng.randrange(k)]}
    cur = next(iter(seen))
    es = []
    while len(seen) < k:
        nx = vs[rng.randrange(k)]
        if nx == cur:
            continue
        if nx not in seen:
            seen.add(nx)
            es.append((cur, nx))
        cur = nx
    return es


def random_edges(rng: random.Random, n: int, m: int) -> list[tuple[int, int]]:
    es = []
    while len(es) < m:
        u, v = rng.randrange(n), rng.randrange(n)
        if u != v:
            es.append((u, v))
    return es


def main_case(seed: int) -> str:
    if seed == 0:
        return SAMPLE1
    if seed == 1:
        return SAMPLE2
    rng = random.Random(seed * 7919 + 61)
    if seed == 2:  # 全域木 3 本 (1497 本) と乱択の辺 3 本
        n = 500
        es = [e for _ in range(3) for e in uniform_tree(rng, n)] + random_edges(rng, n, 3)
        return out(rng, n, es)
    if seed == 3:  # 全域木 3 本の辺を 1 本抜き、乱択の辺を 3 本足す (足した辺が穴を埋めて取れる)
        n = 500
        es = [e for _ in range(3) for e in uniform_tree(rng, n)]
        es.pop(rng.randrange(len(es)))
        return out(rng, n, es + random_edges(rng, n, 3))
    if seed == 4:  # 乱択の辺だけ (次数が小さい頂点があり、取れない)
        return out(rng, 500, random_edges(rng, 500, 1500))
    if seed == 5:  # K_55 (辺 1485 本、取れる)
        n = 55
        return out(rng, n, [(u, v) for u in range(n) for v in range(u + 1, n)])
    if seed == 6:  # 頂点が少なく多重辺が多い (取れる)
        return out(rng, 30, random_edges(rng, 30, 1500))
    if seed == 7:  # N = 2 の多重辺 (取れる)
        return out(rng, 2, [(0, 1)] * 1500)
    if seed == 8:  # 2 つの成分 (どちらも密だが、つながらず取れない)
        n = 100
        es = [(u, v) for u in range(50) for v in range(u + 1, 50)] + [(50 + u, 50 + v) for u in range(40) for v in range(u + 1, 40)]
        return out(rng, n, es[:1500])
    if seed == 9:  # 格子に平行辺を足す (取れない)
        h = w = 20
        es = []
        for i in range(h):
            for j in range(w):
                v = i * w + j
                if j + 1 < w:
                    es.append((v, v + 1))
                if i + 1 < h:
                    es.append((v, v + w))
        es = es + es[: 1500 - len(es)]
        return out(rng, h * w, es)
    if seed == 10:  # 2 本の全域木と 1 本の森 (取れない)
        n = 500
        es = [e for _ in range(2) for e in uniform_tree(rng, n)]
        vs = list(range(n))
        rng.shuffle(vs)
        es += uniform_tree(rng, n, vs[: n // 2]) + random_edges(rng, n, 4)
        return out(rng, n, es)
    if seed == 11:  # N = 400 の全域木 3 本をちょうど並べる (取れる)
        n = 400
        return out(rng, n, [e for _ in range(3) for e in uniform_tree(rng, n)])
    if seed == 12:  # 全域木 3 本に、全域木を 2 本足す (取れる、余りの辺が多い)
        n = 300
        return out(rng, n, [e for _ in range(5) for e in uniform_tree(rng, n)])
    # seed 13: 全域木 3 本の辺を 3 本抜く (3(N - 1) に足りず、取れない)
    n = 500
    es = [e for _ in range(3) for e in uniform_tree(rng, n)]
    for _ in range(3):
        es.pop(rng.randrange(len(es)))
    return out(rng, n, es)


def small_case(seed: int) -> str:
    """N が 2 から 6、M が 9 までの入力。取れる入力が混ざるように、全域木を 3 本並べたものを 3 回に 1 回は使う。"""
    rng = random.Random(seed * 1000003 + 7)
    n = rng.randint(2, 4)
    if seed % 3 == 0 and n >= 2:
        es = [e for _ in range(3) for e in uniform_tree(rng, n)]
        if len(es) <= 9:
            if rng.random() < 0.4 and es:
                es.pop(rng.randrange(len(es)))
            es += random_edges(rng, n, rng.randint(0, 9 - len(es)))
            return out(rng, n, es)
    n = rng.randint(2, 6)
    return out(rng, n, random_edges(rng, n, rng.randint(1, 9)))


def main() -> None:
    seed = int(sys.argv[1])
    print(main_case(seed) if seed < 14 else small_case(seed))


if __name__ == "__main__":
    main()
