#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""cf-343-e (Pumping Stations) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

どのケースも連結で、同じ駅の組を結ぶ管は高々 1 本 (問題文の約束)。seed 0 から 11 は本番のケースで、問題文の例、n = 2、
n = 200 と m = 1000 の乱択 (全域木に辺を足す)、木、完全グラフ (n = 45、m = 990)、塊、容量がどれも 1、どれも 100、輪と弦、
道に近い形、中心、格子に辺を足した形。駅の番号と辺の順は乱択で並べ替える。seed 1000 以上は、駅が 8 個以下の入力を出す
(愚直解 brute.cpp 用)。
"""
import random
import sys

SAMPLE = """6 11
1 2 10
1 6 8
2 3 4
2 5 2
2 6 3
3 4 5
3 5 4
3 6 2
4 5 7
4 6 2
5 6 3"""


def build(rng: random.Random, n: int, m: int, base: list[tuple[int, int]], cap) -> str:
    """base の辺 (連結にするもの) に、乱択の組を m 本まで足す。"""
    seen: set[tuple[int, int]] = set()
    edges: list[tuple[int, int]] = []

    def add(u: int, v: int) -> None:
        if u != v and (min(u, v), max(u, v)) not in seen:
            seen.add((min(u, v), max(u, v)))
            edges.append((u, v))

    for u, v in base:
        add(u, v)
    m = min(m, n * (n - 1) // 2)
    while len(edges) < m:
        add(rng.randrange(n), rng.randrange(n))
    p = list(range(n))
    rng.shuffle(p)
    rng.shuffle(edges)
    lines = [f"{n} {len(edges)}"]
    for u, v in edges:
        if rng.random() < 0.5:
            u, v = v, u
        lines.append(f"{p[u] + 1} {p[v] + 1} {cap(rng)}")
    return "\n".join(lines)


def rnd_tree(rng: random.Random, n: int) -> list[tuple[int, int]]:
    return [(v, rng.randrange(v)) for v in range(1, n)]


def main_case(seed: int) -> str:
    rng = random.Random(seed * 7919 + 1)
    c100 = lambda r: r.randint(1, 100)
    if seed == 0:
        return SAMPLE
    if seed == 1:
        return build(rng, 2, 1, [(0, 1)], c100)
    if seed == 2:
        return build(rng, 200, 1000, rnd_tree(rng, 200), c100)
    if seed == 3:
        return build(rng, 200, 199, rnd_tree(rng, 200), c100)
    if seed == 4:
        return build(rng, 45, 990, rnd_tree(rng, 45), c100)
    if seed == 5:
        base = []
        for k in range(4):
            lo = 50 * k
            base += [(lo + v, lo + rng.randrange(v)) for v in range(1, 50)]
            base += [(lo + rng.randrange(50), lo + rng.randrange(50)) for _ in range(180)]
        base += [(50 * k + rng.randrange(50), 50 * (k + 1) + rng.randrange(50)) for k in range(3)]
        return build(rng, 200, 1000, base, c100)
    if seed == 6:
        return build(rng, 200, 1000, rnd_tree(rng, 200), lambda r: 1)
    if seed == 7:
        return build(rng, 200, 1000, rnd_tree(rng, 200), lambda r: 100)
    if seed == 8:
        base = [(v, (v + 1) % 200) for v in range(200)]
        return build(rng, 200, 400, base, c100)
    if seed == 9:
        base = [(v, v + 1) for v in range(199)] + [(v, min(199, v + rng.randint(2, 5))) for v in range(0, 199, 1)]
        return build(rng, 200, 400, base, c100)
    if seed == 10:
        base = [(0, v) for v in range(1, 200)]
        return build(rng, 200, 600, base, c100)
    if seed == 11:
        base = []
        for i in range(10):
            for j in range(20):
                v = i * 20 + j
                if j + 1 < 20:
                    base.append((v, v + 1))
                if i + 1 < 10:
                    base.append((v, v + 20))
        return build(rng, 200, 1000, base, c100)
    raise SystemExit(f"seed {seed} は 0 から 11 か、1000 以上にしてください")


def small_case(seed: int) -> str:
    rng = random.Random(seed * 104729 + 7)
    n = rng.randint(2, 8)
    m = rng.randint(n - 1, n * (n - 1) // 2)
    c = rng.randint(1, 5)
    return build(rng, n, m, rnd_tree(rng, n), lambda r: r.randint(1, c))


def main() -> None:
    seed = int(sys.argv[1])
    print(small_case(seed) if seed >= 1000 else main_case(seed))


if __name__ == "__main__":
    main()
