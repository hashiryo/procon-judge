#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""CSES 1711 (Distinct Routes) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

部屋は 1 から n、転送装置は有向で、同じ (始点, 終点) の組は 2 つ無い。自己ループは作らない。
seed 0 から 9 は本番のケース。seed 1000 以上は、切る辺の集合を全部試す愚直解 (brute.cpp) で解ける
n ≤ 6、m ≤ 12 の入力で、族と大きさを splitmix64 で選ぶ。
"""
import sys

MASK64 = (1 << 64) - 1


class SplitMix64:
    def __init__(self, seed: int) -> None:
        self.x = seed & MASK64

    def next(self) -> int:
        self.x = (self.x + 0x9E3779B97F4A7C15) & MASK64
        z = self.x
        z = ((z ^ (z >> 30)) * 0xBF58476D1CE4E5B9) & MASK64
        z = ((z ^ (z >> 27)) * 0x94D049BB133111EB) & MASK64
        return z ^ (z >> 31)

    def range(self, lo: int, hi: int) -> int:
        return lo + (self.next() * (hi - lo + 1) >> 64)

    def shuffle(self, xs: list) -> None:
        for i in range(len(xs) - 1, 0, -1):
            j = self.range(0, i)
            xs[i], xs[j] = xs[j], xs[i]


def add(es: set, a: int, b: int) -> None:
    if a != b:
        es.add((a, b))


def random_graph(r: SplitMix64, n: int, m: int) -> set:
    es = set()
    while len(es) < m:
        add(es, r.range(1, n), r.range(1, n))
    return es


def layered(r: SplitMix64, n: int, w: int, m: int) -> set:
    """1 から n へ、幅 w の層を順に進む辺 (層の間は乱択) と、少しの後ろ向きの辺。"""
    mid = list(range(2, n))
    r.shuffle(mid)
    layers = [mid[i : i + w] for i in range(0, len(mid), w)]
    es = set()
    for v in layers[0]:
        add(es, 1, v)
    for v in layers[-1]:
        add(es, v, n)
    # 隣の層へは、どの頂点からも 1 本は出て、どの頂点へも 1 本は入るようにする。
    for a, b in zip(layers, layers[1:]):
        for u in a:
            add(es, u, b[r.range(0, len(b) - 1)])
        for v in b:
            add(es, a[r.range(0, len(a) - 1)], v)
    while len(es) < m:
        i = r.range(0, len(layers) - 2)
        a, b = layers[i][r.range(0, len(layers[i]) - 1)], layers[i + 1][r.range(0, len(layers[i + 1]) - 1)]
        if r.range(0, 9) == 0:
            a, b = b, a
        add(es, a, b)
    return es


def hubs(r: SplitMix64, n: int, m: int) -> set:
    """1 から多くの部屋へ、多くの部屋から n へ辺があり、間をまばらにつなぐ。答えが大きくなる形。"""
    es = set()
    for v in range(2, n):
        if r.range(0, 1):
            add(es, 1, v)
        if r.range(0, 1):
            add(es, v, n)
    while len(es) < m:
        add(es, r.range(2, n - 1), r.range(2, n - 1))
    return es


def cycles(r: SplitMix64, n: int, m: int) -> set:
    """1 から n を通って 1 へ戻る有向の閉路を何本も重ねる。行きと帰りの道が入り組み、流れに循環ができやすい形。"""
    es = set()
    mid = list(range(2, n))
    while len(es) < m:
        r.shuffle(mid)
        k = r.range(2, 30)
        go, back = mid[:k], mid[k : 2 * k]
        cyc = [1] + go + [n] + back
        for a, b in zip(cyc, cyc[1:] + cyc[:1]):
            add(es, a, b)
            if len(es) >= m:
                break
    return es


def fmt(n: int, es: set, r: SplitMix64) -> str:
    edges = sorted(es)
    r.shuffle(edges)
    return "\n".join([f"{n} {len(edges)}"] + [f"{a} {b}" for a, b in edges]) + "\n"


def case(seed: int) -> str:
    r = SplitMix64(seed * 1000003 + 1711)
    if seed == 0:
        return "6 7\n1 2\n1 3\n2 6\n3 4\n3 5\n4 6\n5 6\n"
    if seed == 1:
        return fmt(500, random_graph(r, 500, 1000), r)
    if seed == 2:
        return fmt(50, random_graph(r, 50, 1000), r)
    if seed == 3:
        return fmt(500, layered(r, 500, 20, 1000), r)
    if seed == 4:
        return fmt(500, layered(r, 500, 3, 1000), r)
    if seed == 5:
        return fmt(500, hubs(r, 500, 1000), r)
    if seed == 6:
        return fmt(500, cycles(r, 500, 1000), r)
    if seed == 7:  # n へ入る辺が無い (答え 0)
        es = {(a, b) for a, b in random_graph(r, 499, 1000)}
        return fmt(500, es, r)
    if seed == 8:  # 1 と n だけ
        return "2 1\n1 2\n"
    if seed == 9:  # 1 本の長い道と、それを短絡する辺
        n = 500
        order = [1] + list(range(2, n)) + [n]
        es = set()
        for a, b in zip(order, order[1:]):
            add(es, a, b)
        while len(es) < 1000:
            i, j = sorted((r.range(0, n - 1), r.range(0, n - 1)))
            add(es, order[i], order[j])
        return fmt(n, es, r)
    n = r.range(2, 6)
    m = r.range(1, min(12, n * (n - 1)))
    return fmt(n, random_graph(r, n, m), r)


def main() -> None:
    seed = int(sys.argv[1])
    if not (0 <= seed <= 9 or seed >= 1000):
        raise SystemExit(f"seed {seed} は 0 から 9 か、1000 以上にしてください")
    sys.stdout.write(case(seed))


if __name__ == "__main__":
    main()
