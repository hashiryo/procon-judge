#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""大きい Q の区間 distinct の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

形式 (1-indexed):
  n q
  a_1 ... a_n
  q 行の l r (閉区間 [l, r] の distinct 数を出す)

seed 0 から 7 は本番のケース。例、n=1、全部同じ (q=1e6)、全部異なる (q=1e6)、ランダム
(n=1e5, q=1e6)、値が 5 種類でクエリが狭い窓、n=1e4 で q=1e6、クエリが全部 [1,n] を並べる。
seed 1000 以上は、クエリごとに区間を走査する愚直解 (brute.cpp) で解ける n, q <= 40 の
小さい入力を出す。
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


def random_queries(rng: SplitMix64, n: int, q: int) -> list[tuple[int, int]]:
    qs = []
    for _ in range(q):
        l = rng.range(1, n)
        r = rng.range(1, n)
        if l > r:
            l, r = r, l
        qs.append((l, r))
    return qs


def build(seed: int) -> tuple[list[int], list[tuple[int, int]]]:
    rng = SplitMix64(seed)
    if seed == 0:
        n, q = 10, 20
        a = [1, 2, 1, 3, 2, 4, 5, 1, 3, 6]
        return a, random_queries(rng, n, q)
    if seed == 1:
        return [7], [(1, 1)]
    if seed == 2:
        n, q = 100000, 1000000
        return [1] * n, random_queries(rng, n, q)
    if seed == 3:
        n, q = 100000, 1000000
        return list(range(1, n + 1)), random_queries(rng, n, q)
    if seed == 4:
        n, q = 100000, 1000000
        a = [rng.range(1, 10**9) for _ in range(n)]
        return a, random_queries(rng, n, q)
    if seed == 5:
        n, q = 100000, 1000000
        a = [rng.range(1, 5) for _ in range(n)]
        qs = []
        for _ in range(q):
            l = rng.range(40000, 60000)
            r = rng.range(40000, 60000)
            if l > r:
                l, r = r, l
            qs.append((l, r))
        return a, qs
    if seed == 6:
        n, q = 10000, 1000000
        a = [rng.range(1, 10**9) for _ in range(n)]
        return a, random_queries(rng, n, q)
    if seed == 7:
        n, q = 100000, 1000000
        a = [rng.range(1, 10**9) for _ in range(n)]
        return a, [(1, n)] * q
    # seed 1000 以上: 愚直解が解ける小さい入力。
    n = rng.range(1, 40)
    q = rng.range(1, 40)
    a = [rng.range(1, 6) for _ in range(n)]
    return a, random_queries(rng, n, q)


def main() -> None:
    seed = int(sys.argv[1])
    a, qs = build(seed)
    out = [f"{len(a)} {len(qs)}", " ".join(map(str, a))]
    out.extend(f"{l} {r}" for l, r in qs)
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
