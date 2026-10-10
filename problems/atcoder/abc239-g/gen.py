#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""ABC 239 G (Builder Takahashi) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

グラフは単純で連結、辺 (1, N) は無い。c_1 = c_N = 0、ほかは 1 以上 10^9 以下。
seed 0 から 10 は本番のケース。seed 1000 以上は、壁を置く頂点の集合を全部試す愚直解 (brute.cpp) で解ける
N ≤ 10 の入力で、族と大きさを splitmix64 で選ぶ。
"""
import sys

MASK64 = (1 << 64) - 1
CMAX = 10**9


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


def spanning_tree(r: SplitMix64, n: int) -> set[tuple[int, int]]:
    """頂点 1..n の乱択の全域木 (辺 (1, n) を使わない)。"""
    while True:
        order = list(range(1, n + 1))
        r.shuffle(order)
        es = set()
        for i in range(1, n):
            a, b = order[i], order[r.range(0, i - 1)]
            es.add((min(a, b), max(a, b)))
        if (1, n) not in es:
            return es


def add_random(r: SplitMix64, n: int, es: set, m: int) -> None:
    """辺が m 本になるまで、(1, n) 以外の乱択の辺を足す。"""
    cap = n * (n - 1) // 2 - 1
    m = min(m, cap)
    if m > cap // 2:
        allp = [(a, b) for a in range(1, n + 1) for b in range(a + 1, n + 1) if (a, b) != (1, n) and (a, b) not in es]
        r.shuffle(allp)
        for p in allp[: m - len(es)]:
            es.add(p)
        return
    while len(es) < m:
        a, b = r.range(1, n), r.range(1, n)
        if a == b:
            continue
        p = (min(a, b), max(a, b))
        if p != (1, n):
            es.add(p)


def layered(r: SplitMix64, n: int, w: int) -> set[tuple[int, int]]:
    """1 と n の間に幅 w の層を並べ、隣の層どうしをすべてつなぐ。"""
    mid = list(range(2, n))
    layers = [mid[i : i + w] for i in range(0, len(mid), w)]
    es = set()
    for v in layers[0]:
        es.add((1, v))
    for v in layers[-1]:
        es.add((min(v, n), max(v, n)))
    for a, b in zip(layers, layers[1:]):
        for u in a:
            for v in b:
                es.add((min(u, v), max(u, v)))
    return es


def two_paths(r: SplitMix64, n: int, cross: int) -> set[tuple[int, int]]:
    """1 から n への長い道 2 本と、その間の渡りを cross 本。"""
    mid = list(range(2, n))
    r.shuffle(mid)
    p, q = mid[: len(mid) // 2], mid[len(mid) // 2 :]
    es = set()
    for path in (p, q):
        seq = [1] + path + [n]
        for a, b in zip(seq, seq[1:]):
            es.add((min(a, b), max(a, b)))
    for _ in range(cross):
        a, b = p[r.range(0, len(p) - 1)], q[r.range(0, len(q) - 1)]
        es.add((min(a, b), max(a, b)))
    return es


def costs(r: SplitMix64, n: int, lo: int, hi: int) -> list[int]:
    return [0] + [r.range(lo, hi) for _ in range(n - 2)] + [0]


def fmt(n: int, es: set, c: list[int], r: SplitMix64) -> str:
    edges = sorted(es)
    r.shuffle(edges)
    lines = [f"{n} {len(edges)}"] + [f"{a} {b}" for a, b in edges] + [" ".join(map(str, c))]
    return "\n".join(lines) + "\n"


def case(seed: int) -> str:
    r = SplitMix64(seed * 1000003 + 239)
    if seed == 0:
        return "5 5\n1 2\n2 3\n3 5\n2 4\n4 5\n0 8 3 4 0\n"
    if seed == 1:  # 密な乱択 (辺の数が上限)
        n = 100
        es = spanning_tree(r, n)
        add_random(r, n, es, n * (n - 1) // 2 - 1)
        return fmt(n, es, costs(r, n, 1, CMAX), r)
    if seed == 2:  # 木に辺を少し足したもの
        n = 100
        es = spanning_tree(r, n)
        add_random(r, n, es, 150)
        return fmt(n, es, costs(r, n, 1, CMAX), r)
    if seed == 3:  # 木
        n = 100
        return fmt(n, spanning_tree(r, n), costs(r, n, 1, CMAX), r)
    if seed == 4:  # 費用がすべて等しい
        n = 100
        es = spanning_tree(r, n)
        add_random(r, n, es, 1500)
        return fmt(n, es, [0] + [CMAX] * (n - 2) + [0], r)
    if seed == 5:  # 費用が 1 から 3 で同点だらけ
        n = 100
        es = spanning_tree(r, n)
        add_random(r, n, es, 800)
        return fmt(n, es, costs(r, n, 1, 3), r)
    if seed == 6:  # 層状
        n = 98
        return fmt(n, layered(r, n, 8), costs(r, n, 1, CMAX), r)
    if seed == 7:  # 長い道 2 本と渡り
        n = 100
        return fmt(n, two_paths(r, n, 30), costs(r, n, 1, CMAX), r)
    if seed == 8:  # 密で、安い頂点が 1 つだけ
        n = 100
        es = spanning_tree(r, n)
        add_random(r, n, es, 3000)
        c = [0] + [CMAX] * (n - 2) + [0]
        c[r.range(1, n - 2)] = 1
        return fmt(n, es, c, r)
    if seed == 9:  # 最小
        return "3 2\n1 2\n2 3\n0 1000000000 0\n"
    if seed == 10:  # 星形 (1 と n 以外の全頂点が 1 と n の両方につながる)
        n = 100
        es = {(1, v) for v in range(2, n)} | {(v, n) for v in range(2, n)}
        return fmt(n, es, costs(r, n, 1, CMAX), r)
    # seed 1000 以上: 愚直解で解ける小さい入力
    n = r.range(3, 10)
    es = spanning_tree(r, n)
    cap = n * (n - 1) // 2 - 1
    add_random(r, n, es, r.range(len(es), cap))
    hi = [3, 10, CMAX][r.range(0, 2)]
    return fmt(n, es, costs(r, n, 1, hi), r)


def main() -> None:
    seed = int(sys.argv[1])
    if not (0 <= seed <= 10 or seed >= 1000):
        raise SystemExit(f"seed {seed} は 0 から 10 か、1000 以上にしてください")
    sys.stdout.write(case(seed))


if __name__ == "__main__":
    main()
