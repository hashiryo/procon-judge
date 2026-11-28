#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""永続セグメント木 (区間 max xor) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

書式:
  n q
  a_0 ... a_{n-1}
  続く q 行は l r x (a_l ... a_{r-1} のうち、x との xor が最大になるものの xor)。

seed 0 から 11 は本番のケース。例、n = 1、全部同じ、全部異なる、x = 0、x が全部 1、
n = q = 2e5 のランダム、値が小さい、区間が全体、区間が短い、上位 bit だけ、中くらい。
seed 1000 以上は、愚直解で解ける n, q <= 40 の小さい入力を出す。
"""
import sys

MASK64 = (1 << 64) - 1
NMAX = 200000
QMAX = 200000
VMAX = (1 << 30) - 1
# 本番のケースの数。seed 0 から COUNT - 1 がテストデータになる。
COUNT = 12


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
        """lo 以上 hi 以下。"""
        return lo + (self.next() * (hi - lo + 1) >> 64)


def fmt(n: int, a: list, qs: list) -> str:
    out = [f"{n} {len(qs)}", " ".join(map(str, a))]
    out += [" ".join(map(str, q)) for q in qs]
    return "\n".join(out) + "\n"


def rand_query(r: SplitMix64, n: int, vmax: int) -> tuple:
    if n == 1:
        return (0, 1, r.range(0, vmax))
    l = r.range(0, n - 1)
    return (l, r.range(l + 1, n), r.range(0, vmax))


def small(seed: int) -> str:
    """愚直解で解ける n, q <= 40 の入力。"""
    r = SplitMix64(seed * 7919 + 17)
    n = r.range(1, 40)
    q = r.range(1, 40)
    vmax = (1, 10, 1023, VMAX)[r.range(0, 3)]
    a = [r.range(0, vmax) for _ in range(n)]
    qs = [rand_query(r, n, vmax) for _ in range(q)]
    return fmt(n, a, qs)


def case(seed: int) -> str:
    if seed >= 1000:
        return small(seed)
    r = SplitMix64(seed * 1000003 + 12345)
    if seed == 0:
        return (
            "5 6\n"
            "1 2 3 4 5\n"
            "0 5 0\n"
            "0 5 7\n"
            "1 4 0\n"
            "2 5 1\n"
            "0 2 100\n"
            "3 5 6\n"
        )
    if seed == 1:
        n, q = 1, 40
        a = [r.range(0, VMAX)]
        qs = [(0, 1, r.range(0, VMAX)) for _ in range(q)]
        return fmt(n, a, qs)
    if seed == 2:
        n, q = 1000, 1000
        v = r.range(0, VMAX)
        a = [v] * n
        qs = [rand_query(r, n, VMAX) for _ in range(q)]
        return fmt(n, a, qs)
    if seed == 3:
        n, q = 1000, 1000
        a = list(range(n))
        qs = [rand_query(r, n, VMAX) for _ in range(q)]
        return fmt(n, a, qs)
    if seed == 4:
        n, q = NMAX, QMAX
        a = [r.range(0, VMAX) for _ in range(n)]
        qs = []
        for _ in range(q):
            l = r.range(0, n - 1)
            qs.append((l, r.range(l + 1, n), 0))
        return fmt(n, a, qs)
    if seed == 5:
        n, q = NMAX, QMAX
        a = [r.range(0, VMAX) for _ in range(n)]
        qs = []
        for _ in range(q):
            l = r.range(0, n - 1)
            qs.append((l, r.range(l + 1, n), VMAX))
        return fmt(n, a, qs)
    if seed == 6:
        n, q = NMAX, QMAX
        a = [r.range(0, VMAX) for _ in range(n)]
        qs = [rand_query(r, n, VMAX) for _ in range(q)]
        return fmt(n, a, qs)
    if seed == 7:
        n, q = NMAX, QMAX
        a = [r.range(0, 1023) for _ in range(n)]
        qs = [rand_query(r, n, 1023) for _ in range(q)]
        return fmt(n, a, qs)
    if seed == 8:
        n, q = NMAX, QMAX
        a = [r.range(0, VMAX) for _ in range(n)]
        qs = [(0, n, r.range(0, VMAX)) for _ in range(q)]
        return fmt(n, a, qs)
    if seed == 9:
        n, q = NMAX, QMAX
        a = [r.range(0, VMAX) for _ in range(n)]
        qs = []
        for _ in range(q):
            l = r.range(0, n - 2)
            qs.append((l, min(n, l + 1 + r.range(0, 15)), r.range(0, VMAX)))
        return fmt(n, a, qs)
    if seed == 10:
        n, q = NMAX, QMAX
        a = [r.range(1 << 29, VMAX) for _ in range(n)]
        qs = [rand_query(r, n, VMAX) for _ in range(q)]
        return fmt(n, a, qs)
    # seed 11
    n = r.range(NMAX // 2, NMAX)
    q = r.range(QMAX // 2, QMAX)
    a = [r.range(0, VMAX) for _ in range(n)]
    qs = [rand_query(r, n, VMAX) for _ in range(q)]
    return fmt(n, a, qs)


def main() -> None:
    if len(sys.argv) < 2:
        raise SystemExit("usage: gen.py <seed>")
    sys.stdout.write(case(int(sys.argv[1])))


if __name__ == "__main__":
    main()
