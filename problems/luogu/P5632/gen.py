#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""洛谷 P5632 (【模板】Stoer-Wagner) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

どのケースも連結で、同じ組を結ぶ辺は高々 1 本、重みは正で和は 10^9 以下 (問題文の約束)。seed 0 は問題文の例。seed 1 から
11 は n = 600 のケースで、完全グラフ (重みが乱択、どれも 1、裾の重い乱択)、疎な乱択 (6000 本と 5 万本)、木、道、2 つの
密な塊を軽い辺 10 本でつないだ形、輪、20 × 30 の格子、60 頂点の塊 10 個を輪につないだ形。頂点の番号と辺の順は乱択に
並べ替える。seed 1000 以上は、愚直解 (brute.hpp) と突き合わせる小さい入力 (頂点 10 個まで)。
"""
import random
import sys

SAMPLE = """4 6
1 2 5
1 3 1
2 4 1
3 4 2
2 3 1
1 4 2"""


class G:
    def __init__(self, n: int) -> None:
        self.n = n
        self.e: dict[tuple[int, int], int] = {}

    def add(self, u: int, v: int, w: int) -> None:
        if u != v and (min(u, v), max(u, v)) not in self.e:
            self.e[(min(u, v), max(u, v))] = w

    def tree(self, rng: random.Random, w) -> None:
        for v in range(1, self.n):
            self.add(v, rng.randrange(v), w(rng))

    def out(self, rng: random.Random) -> str:
        p = list(range(self.n))
        rng.shuffle(p)
        es = list(self.e.items())
        rng.shuffle(es)
        assert sum(w for _, w in es) <= 10**9
        lines = [f"{self.n} {len(es)}"]
        for (u, v), w in es:
            if rng.random() < 0.5:
                u, v = v, u
            lines.append(f"{p[u] + 1} {p[v] + 1} {w}")
        return "\n".join(lines)


def random_fill(g: G, rng: random.Random, m: int, w) -> None:
    while len(g.e) < m:
        g.add(rng.randrange(g.n), rng.randrange(g.n), w(rng))


def main_case(seed: int) -> str:
    if seed == 0:
        return SAMPLE
    rng = random.Random(seed * 7919 + 31)
    n = 600
    g = G(n)
    if seed in (1, 2, 11):
        heavy = lambda r: r.randint(1, 1000) if r.random() < 0.99 else r.randint(10**5, 2 * 10**5)
        w = {1: lambda r: r.randint(1, 5000), 2: lambda r: 1, 11: heavy}[seed]
        for u in range(n):
            for v in range(u + 1, n):
                g.add(u, v, w(rng))
    elif seed == 3:
        g.tree(rng, lambda r: r.randint(1, 1000))
        random_fill(g, rng, 6000, lambda r: r.randint(1, 1000))
    elif seed == 4:
        g.tree(rng, lambda r: r.randint(1, 10**6))
    elif seed == 5:
        for v in range(1, n):
            g.add(v - 1, v, rng.randint(1, 10**6))
    elif seed == 6:
        h = n // 2
        for lo, hi in ((0, h), (h, n)):
            for u in range(lo, hi):
                for v in range(u + 1, hi):
                    if rng.random() < 0.7:
                        g.add(u, v, rng.randint(1, 100))
        for _ in range(10):
            g.add(rng.randrange(h), rng.randrange(h, n), rng.randint(1, 100))
    elif seed == 7:
        for v in range(n):
            g.add(v, (v + 1) % n, rng.randint(1, 10**6))
    elif seed == 8:
        R, C = 20, 30
        for i in range(R):
            for j in range(C):
                if j + 1 < C:
                    g.add(i * C + j, i * C + j + 1, rng.randint(1, 1000))
                if i + 1 < R:
                    g.add(i * C + j, (i + 1) * C + j, rng.randint(1, 1000))
    elif seed == 9:
        g.tree(rng, lambda r: r.randint(1, 20000))
        random_fill(g, rng, 50000, lambda r: r.randint(1, 20000))
    elif seed == 10:
        for c in range(10):
            lo = 60 * c
            for u in range(lo, lo + 60):
                for v in range(u + 1, lo + 60):
                    if rng.random() < 0.5:
                        g.add(u, v, rng.randint(1, 1000))
            for v in range(lo + 1, lo + 60):
                g.add(v, rng.randrange(lo, v), rng.randint(1, 1000))
            for _ in range(30):
                g.add(rng.randrange(lo, lo + 60), rng.randrange(60 * ((c + 1) % 10), 60 * ((c + 1) % 10) + 60), rng.randint(1, 1000))
    else:
        raise SystemExit(f"seed {seed} は 0 から 11 か、1000 以上にしてください")
    return g.out(rng)


def small_case(seed: int) -> str:
    rng = random.Random(seed * 104729 + 7)
    n = rng.randint(2, 10)
    g = G(n)
    hi = rng.choice((1, 10, 1000))
    g.tree(rng, lambda r: r.randint(1, hi))
    random_fill(g, rng, rng.randint(n - 1, n * (n - 1) // 2), lambda r: r.randint(1, hi))
    return g.out(rng)


def main() -> None:
    seed = int(sys.argv[1])
    print(small_case(seed) if seed >= 1000 else main_case(seed))


if __name__ == "__main__":
    main()
