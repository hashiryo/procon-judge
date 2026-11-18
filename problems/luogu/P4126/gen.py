#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""洛谷 P4126 ([AHOI2009] 最小割) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

有向グラフで、多重辺があり、容量は 1 以上 10^5 以下 (問題文の約束)。seed 0 から 9 は本番のケースで、大きさは問題文の
表 (N が 10 から 4000、M が 50 から 60000) に合わせ、乱択と層の形を交互に置く。層の形は s から t へ層を並べ、隣の層へ
前向きの辺を張り、後ろ向きの辺と飛ばす辺を少し混ぜる。容量の小さいケース (1 から 3 や 1 から 10) は最小カットが
たくさんある。1 つの層の辺だけ容量を小さくして、そこにだけ最小カットが集まる形もある。頂点の番号は乱択で並べ替える。
seed 1000 以上は、頂点が 9 個以下の入力を出す (愚直解 brute.cpp 用)。
"""
import random
import sys


def emit(rng: random.Random, n: int, edges: list[tuple[int, int, int]], s: int, t: int) -> str:
    p = list(range(n))
    rng.shuffle(p)
    rng.shuffle(edges)
    lines = [f"{n} {len(edges)} {p[s] + 1} {p[t] + 1}"]
    lines += [f"{p[u] + 1} {p[v] + 1} {c}" for u, v, c in edges]
    return "\n".join(lines)


def rnd(rng: random.Random, n: int, m: int, cmax: int) -> str:
    edges = []
    for _ in range(m):
        u = rng.randrange(n)
        v = rng.randrange(n - 1)
        edges.append((u, v + (v >= u), rng.randint(1, cmax)))
    return emit(rng, n, edges, 0, n - 1)


def layered(rng: random.Random, n: int, m: int, cmax: int, bottleneck: bool = False) -> str:
    """頂点 0 が s、n - 1 が t。間を k 層に分けて前向きの辺を張る。"""
    inner = n - 2
    k = max(1, int(inner ** 0.5) // 2)
    layer = [[] for _ in range(k)]
    for v in range(1, n - 1):
        layer[(v - 1) * k // inner].append(v)
    weak = rng.randrange(k + 1) if bottleneck else -1  # この番目の層の間の辺だけ容量を小さくする
    edges = []

    def cap(i: int) -> int:
        return rng.randint(1, max(1, cmax // 1000)) if i == weak else rng.randint(1, cmax)

    while len(edges) < m:
        r = rng.random()
        i = rng.randrange(k + 1)  # 層 i - 1 から層 i へ (層 -1 は s、層 k は t)
        src = [0] if i == 0 else layer[i - 1]
        dst = [n - 1] if i == k else layer[i]
        if r < 0.85:
            edges.append((rng.choice(src), rng.choice(dst), cap(i)))
        elif r < 0.95:
            edges.append((rng.choice(dst), rng.choice(src), rng.randint(1, cmax)))
        else:
            u, v = rng.randrange(n), rng.randrange(n)
            if u != v:
                edges.append((u, v, rng.randint(1, cmax)))
    return emit(rng, n, edges, 0, n - 1)


CASES = {
    0: lambda r: rnd(r, 10, 50, 10),
    1: lambda r: layered(r, 20, 200, 5),
    2: lambda r: layered(r, 200, 2000, 100),
    3: lambda r: rnd(r, 200, 2000, 3),
    4: lambda r: layered(r, 1000, 20000, 100000, True),
    5: lambda r: rnd(r, 1000, 20000, 100),
    6: lambda r: layered(r, 1000, 40000, 10),
    7: lambda r: rnd(r, 2000, 50000, 100000),
    8: lambda r: layered(r, 3000, 60000, 3),
    9: lambda r: layered(r, 4000, 60000, 100000, True),
}


def small_case(seed: int) -> str:
    rng = random.Random(seed * 104729 + 3)
    n = rng.randint(2, 9)
    m = rng.randint(0, 25)
    c = rng.randint(1, 4)
    edges = []
    for _ in range(m):
        u = rng.randrange(n)
        v = rng.randrange(n - 1)
        edges.append((u, v + (v >= u), rng.randint(1, c)))
    s = rng.randrange(n)
    t = rng.randrange(n - 1)
    return emit(rng, n, edges, s, t + (t >= s))


def main() -> None:
    seed = int(sys.argv[1])
    if seed in CASES:
        print(CASES[seed](random.Random(seed * 7919 + 11)))
    elif seed >= 1000:
        print(small_case(seed))
    else:
        raise SystemExit(f"seed {seed} は 0 から 9 か、1000 以上にしてください")


if __name__ == "__main__":
    main()
