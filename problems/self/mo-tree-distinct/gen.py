#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""木のパスの distinct の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

形式は
  n q
  c_1 ... c_n
  n-1 行の辺 u v (1-indexed)
  q 行のクエリ u v (1-indexed のパス)
で、各クエリのパス上の相異なる色の数を出す問題の入力になる。

seed 0 から 11 は本番のケース。例、n=1、n=2、深さ 1e5 のパス、星、ランダム木、全部同じ色、
全部異なる色、色が 3 種類、二分木、ほうき形、n=q=1e5 のランダムを並べる。
seed 1000 以上は愚直解 (brute.cpp) が解ける小さい入力 (n, q <= 40) を出す。木の形と色の
付け方を splitmix64 で選ぶ。
"""
import sys

MASK64 = (1 << 64) - 1
N_MAX = 100000
C_MAX = 10**9


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


def random_tree(r: SplitMix64, n: int) -> list:
    """頂点 i を 1 から i-1 のどれかにつなぐ。"""
    return [(r.range(1, i - 1), i) for i in range(2, n + 1)]


def chain(n: int) -> list:
    return [(i - 1, i) for i in range(2, n + 1)]


def star(n: int) -> list:
    return [(1, i) for i in range(2, n + 1)]


def binary_tree(n: int) -> list:
    return [(i // 2, i) for i in range(2, n + 1)]


def broom(n: int) -> list:
    """長さ n//2 の柄の先へ、残りの頂点を葉として付ける。"""
    handle = max(1, n // 2)
    return [(i - 1, i) for i in range(2, handle + 1)] + [(handle, i) for i in range(handle + 1, n + 1)]


def random_queries(r: SplitMix64, n: int, q: int) -> list:
    return [(r.range(1, n), r.range(1, n)) for _ in range(q)]


def fmt(n: int, colors: list, edges: list, queries: list) -> str:
    out = [f"{n} {len(queries)}", " ".join(map(str, colors))]
    out += [f"{u} {v}" for u, v in edges]
    out += [f"{u} {v}" for u, v in queries]
    return "\n".join(out) + "\n"


def case(seed: int) -> str:
    r = SplitMix64(seed * 1000003 + 20261128)
    if seed == 0:  # 例。色の重なる小さな木。
        return fmt(7, [1, 2, 1, 3, 2, 3, 1], [(1, 2), (1, 3), (2, 4), (2, 5), (3, 6), (3, 7)],
                   [(4, 5), (4, 6), (1, 1), (2, 7), (5, 3)])
    if seed == 1:  # n = 1。色は制約の最大。
        return fmt(1, [C_MAX], [], [(1, 1)])
    if seed == 2:  # n = 2。同じ色の頂点と違う色の頂点、クエリの向きを両方入れる。
        return fmt(2, [7, 9], [(1, 2)], [(1, 1), (2, 2), (1, 2), (2, 1)])
    if seed == 3:  # 深さ 1e5 のパス
        n = N_MAX
        return fmt(n, [r.range(1, n) for _ in range(n)], chain(n), random_queries(r, n, N_MAX))
    if seed == 4:  # 星
        n = N_MAX
        return fmt(n, [r.range(1, C_MAX) for _ in range(n)], star(n), random_queries(r, n, N_MAX))
    if seed == 5:  # ランダム木 (中くらいの大きさ)
        n = 30000
        return fmt(n, [r.range(1, C_MAX) for _ in range(n)], random_tree(r, n), random_queries(r, n, n))
    if seed == 6:  # 全部同じ色
        n = N_MAX
        return fmt(n, [5] * n, random_tree(r, n), random_queries(r, n, N_MAX))
    if seed == 7:  # 全部異なる色
        n = N_MAX
        return fmt(n, list(range(1, n + 1)), random_tree(r, n), random_queries(r, n, N_MAX))
    if seed == 8:  # 色が 3 種類
        n = N_MAX
        return fmt(n, [r.range(1, 3) for _ in range(n)], random_tree(r, n), random_queries(r, n, N_MAX))
    if seed == 9:  # 二分木
        n = N_MAX
        return fmt(n, [r.range(1, C_MAX) for _ in range(n)], binary_tree(n), random_queries(r, n, N_MAX))
    if seed == 10:  # ほうき形
        n = N_MAX
        return fmt(n, [r.range(1, C_MAX) for _ in range(n)], broom(n), random_queries(r, n, N_MAX))
    # n=q=1e5 のランダム
    n = N_MAX
    return fmt(n, [r.range(1, C_MAX) for _ in range(n)], random_tree(r, n), random_queries(r, n, N_MAX))


def small_case(seed: int) -> str:
    """愚直解で解ける n, q <= 40 の入力。seed 1000 から 1299 が使う。"""
    r = SplitMix64(seed * 1000003 + 9176)
    n = r.range(1, 40)
    q = r.range(1, 40)
    shape = r.range(0, 4)
    if shape == 0:
        edges = random_tree(r, n)
    elif shape == 1:
        edges = chain(n)
    elif shape == 2:
        edges = star(n)
    elif shape == 3:
        edges = binary_tree(n)
    else:
        perm = list(range(1, n + 1))
        r.shuffle(perm)
        edges = [(perm[i], perm[i + 1]) for i in range(n - 1)]
    mode = r.range(0, 2)
    if mode == 0:
        colors = [C_MAX] * n
    elif mode == 1:
        k = r.range(1, 4)
        colors = [r.range(1, k) for _ in range(n)]
    else:
        colors = [r.range(1, C_MAX) for _ in range(n)]
    return fmt(n, colors, edges, random_queries(r, n, q))


def main() -> None:
    seed = int(sys.argv[1])
    if not (0 <= seed <= 11 or seed >= 1000):
        raise SystemExit(f"seed {seed} は 0 から 11 か、1000 以上にしてください")
    sys.stdout.write(case(seed) if seed <= 11 else small_case(seed))


if __name__ == "__main__":
    main()
