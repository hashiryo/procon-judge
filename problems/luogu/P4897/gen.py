#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""洛谷 P4897 (最小割树の模板) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

seed 0 から 9 は本番のケースで、どれも頂点 501 個 (n = 500)、辺 1500 本、問い合わせ 10^5 個の連通グラフ。乱択の
全域木に辺を足して連通にし、形を変える (乱択、容量の小さい乱択、格子、輪と弦、塊、道に近い形、中心、木に近い形、層、
容量 0 を含む乱択)。頂点の番号は乱択に並べ替える。seed 1000 以上は愚直解 (brute.hpp) と突き合わせる小さい入力。
"""
import random
import sys


def shuffle_labels(rng: random.Random, n1: int, edges: list[tuple[int, int, int]]) -> list[tuple[int, int, int]]:
    p = list(range(n1))
    rng.shuffle(p)
    out = [(p[u], p[v], w) if rng.random() < 0.5 else (p[v], p[u], w) for u, v, w in edges]
    rng.shuffle(out)
    return out


def connect(rng: random.Random, n1: int, edges: list[tuple[int, int, int]], wl: int, wh: int) -> None:
    """辺の列に、つながっていない成分どうしを結ぶ辺を足す。"""
    par = list(range(n1))

    def find(x: int) -> int:
        while par[x] != x:
            par[x] = par[par[x]]
            x = par[x]
        return x

    for u, v, _ in edges:
        par[find(u)] = find(v)
    roots = [v for v in range(n1) if find(v) == v]
    for a, b in zip(roots, roots[1:]):
        edges.append((a, b, rng.randint(wl, wh)))


def make(seed: int, shape: str, n: int, m: int, q: int, wl: int, wh: int) -> str:
    rng = random.Random(seed)
    n1 = n + 1
    e: list[tuple[int, int, int]] = []

    def rnd_edge(lo: int = 0, hi: int = n1) -> None:
        u = rng.randrange(lo, hi)
        v = rng.randrange(lo, hi - 1)
        e.append((u, v + (v >= u), rng.randint(wl, wh)))

    if shape == "random":
        for v in range(1, n1):
            e.append((v, rng.randrange(v), rng.randint(wl, wh)))
    elif shape == "grid":
        w = 25
        for v in range(n1):
            if (v + 1) % w and v + 1 < n1:
                e.append((v, v + 1, rng.randint(wl, wh)))
            if v + w < n1:
                e.append((v, v + w, rng.randint(wl, wh)))
    elif shape == "cycle":
        for v in range(n1):
            e.append((v, (v + 1) % n1, rng.randint(wl, wh)))
    elif shape == "cluster":
        k = 10
        for v in range(n1):
            c = v * k // n1
            lo, hi = c * n1 // k, (c + 1) * n1 // k
            if v > lo:
                e.append((v, rng.randrange(lo, v), rng.randint(wl, wh)))
        while len(e) < m - 30:
            c = rng.randrange(k)
            lo, hi = c * n1 // k, (c + 1) * n1 // k
            rnd_edge(lo, hi)
    elif shape == "path":
        for v in range(1, n1):
            e.append((v - 1, v, rng.randint(wl, wh)))
        while len(e) < m:
            u = rng.randrange(n1)
            v = min(n1 - 1, u + rng.randint(2, 6))
            if u != v:
                e.append((u, v, rng.randint(wl, wh)))
    elif shape == "hub":
        for v in range(3, n1):
            e.append((v, rng.randrange(3), rng.randint(wl, wh)))
    elif shape == "tree":
        for v in range(1, n1):
            e.append((v, rng.randrange(max(0, v - 5), v), rng.randint(wh // 2, wh)))
    elif shape == "layer":
        w = 20 if n1 >= 60 else max(1, n1 // 4)
        for v in range(w, n1):
            e.append((v, (v // w - 1) * w + rng.randrange(w), rng.randint(wl, wh)))
        for v in range(1, min(w, n1)):
            e.append((v, 0, rng.randint(wl, wh)))
        while len(e) < m:
            u = rng.randrange(w, n1)
            e.append((u, (u // w - 1) * w + rng.randrange(w), rng.randint(wl, wh)))
    while len(e) < m:
        rnd_edge()
    connect(rng, n1, e, wl, wh)
    e = shuffle_labels(rng, n1, e)
    lines = [f"{n} {len(e)}"]
    lines += [f"{u} {v} {w}" for u, v, w in e]
    lines.append(str(q))
    for _ in range(q):
        u = rng.randrange(n1)
        v = rng.randrange(n1 - 1)
        lines.append(f"{u} {v + (v >= u)}")
    return "\n".join(lines)


CASES = {
    0: ("random", 500, 1500, 100000, 1, 10000),
    1: ("random", 500, 1500, 100000, 1, 10),
    2: ("grid", 500, 1500, 100000, 1, 10000),
    3: ("cycle", 500, 1500, 100000, 1, 10000),
    4: ("cluster", 500, 1500, 100000, 1, 10000),
    5: ("path", 500, 1500, 100000, 1, 10000),
    6: ("hub", 500, 1500, 100000, 1, 10000),
    7: ("tree", 500, 1500, 100000, 1, 10000),
    8: ("layer", 500, 1500, 100000, 1, 10000),
    9: ("random", 500, 1500, 100000, 0, 10000),
}


def main() -> None:
    seed = int(sys.argv[1])
    if seed in CASES:
        print(make(seed, *CASES[seed]))
    elif seed >= 1000:
        rng = random.Random(seed * 7919 + 3)
        n = rng.randint(1, 11)
        shapes = ["random", "grid", "cycle", "cluster", "path", "hub", "tree", "layer"]
        print(make(seed, rng.choice(shapes), n, rng.randint(n, 25), 30, rng.randint(0, 2), rng.randint(2, 6)))
    else:
        raise SystemExit(f"seed {seed} は 0 から {len(CASES) - 1} か、1000 以上にしてください")


if __name__ == "__main__":
    main()
