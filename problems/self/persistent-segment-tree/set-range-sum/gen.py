#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""永続セグメント木 (点更新・区間和) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

書式:
  n q
  a_0 ... a_{n-1}
  続く q 行は 0 v i x (版 v から新しい版を作り、a_i を x にする) か、
  1 v l r (版 v の a_l ... a_{r-1} の和を出す)。版 0 は初期の列。

seed 0 から 11 は本番のケース。例、n = 1、全部更新、全部クエリ、鎖 (v が直前の版)、
星 (v が 0)、n = q = 1e5 のランダム、更新 7 割、クエリだけ、端の添字、中くらいを並べる。
seed 1000 以上は、愚直解で解ける n, q <= 40 の小さい入力を出す。
"""
import sys

MASK64 = (1 << 64) - 1
NMAX = 100000
QMAX = 100000
VMAX = 10**9
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


def fmt(n: int, a: list, ops: list) -> str:
    out = [f"{n} {len(ops)}", " ".join(map(str, a))]
    out += [" ".join(map(str, op)) for op in ops]
    return "\n".join(out) + "\n"


def rand_set(r: SplitMix64, n: int, cur: int) -> tuple:
    return (0, r.range(0, cur), r.range(0, n - 1), r.range(-VMAX, VMAX))


def rand_sum(r: SplitMix64, n: int, cur: int) -> tuple:
    if n == 1:
        return (1, r.range(0, cur), 0, 1)
    l = r.range(0, n - 2)
    return (1, r.range(0, cur), l, r.range(l + 1, n - 1))


def mixed(r: SplitMix64, n: int, q: int, p_set: int) -> list:
    """p_set は 0..99。更新の割合 (パーセント)。"""
    ops = []
    cur = 0
    for _ in range(q):
        if r.range(0, 99) < p_set:
            ops.append(rand_set(r, n, cur))
            cur += 1
        else:
            ops.append(rand_sum(r, n, cur))
    return ops


def small(seed: int) -> str:
    """愚直解で解ける n, q <= 40 の入力。形と大きさを seed から決める。"""
    r = SplitMix64(seed * 7919 + 17)
    n = r.range(1, 40)
    q = r.range(1, 40)
    vmax = (1, 10, 1000, VMAX)[r.range(0, 3)]
    a = [r.range(-vmax, vmax) for _ in range(n)]
    shape = r.range(0, 4)
    ops = []
    cur = 0
    for i in range(q):
        if shape == 0:
            is_set = i % 2 == 0
        elif shape == 1:
            is_set = True
        elif shape == 2:
            is_set = False
        elif shape == 3:
            is_set = i < q // 2
        else:
            is_set = r.range(0, 1) == 0
        if is_set:
            ops.append((0, r.range(0, cur), r.range(0, n - 1), r.range(-vmax, vmax)))
            cur += 1
        else:
            ops.append(rand_sum(r, n, cur))
    return fmt(n, a, ops)


def case(seed: int) -> str:
    if seed >= 1000:
        return small(seed)
    r = SplitMix64(seed * 1000003 + 12345)
    if seed == 0:
        return (
            "5 6\n"
            "1 2 3 4 5\n"
            "1 0 0 5\n"
            "0 0 2 10\n"
            "1 1 0 5\n"
            "1 0 0 5\n"
            "0 1 0 -7\n"
            "1 2 0 3\n"
        )
    if seed == 1:
        n, q = 1, 40
        a = [r.range(-VMAX, VMAX)]
        ops = []
        cur = 0
        for i in range(q):
            if i % 2 == 0:
                ops.append((0, r.range(0, cur), 0, r.range(-VMAX, VMAX)))
                cur += 1
            else:
                ops.append((1, r.range(0, cur), 0, 1))
        return fmt(n, a, ops)
    if seed == 2:
        n, q = 200, 200
        a = [r.range(-VMAX, VMAX) for _ in range(n)]
        ops = []
        cur = 0
        for _ in range(q // 2):
            ops.append(rand_set(r, n, cur))
            cur += 1
        for _ in range(q - q // 2):
            ops.append(rand_sum(r, n, cur))
        return fmt(n, a, ops)
    if seed == 3:
        n, q = 1000, 1000
        a = [r.range(-VMAX, VMAX) for _ in range(n)]
        ops = []
        for _ in range(q):
            l = r.range(0, n - 2)
            ops.append((1, 0, l, r.range(l + 1, n - 1)))
        return fmt(n, a, ops)
    if seed == 4:
        # 鎖: 直前の版から作る。
        n, q = NMAX, QMAX
        a = [r.range(-VMAX, VMAX) for _ in range(n)]
        ops = []
        cur = 0
        for _ in range(q):
            if r.range(0, 99) < 50:
                ops.append((0, cur, r.range(0, n - 1), r.range(-VMAX, VMAX)))
                cur += 1
            else:
                ops.append(rand_sum(r, n, cur))
        return fmt(n, a, ops)
    if seed == 5:
        # 星: いつも版 0 から作る。
        n, q = NMAX, QMAX
        a = [r.range(-VMAX, VMAX) for _ in range(n)]
        ops = []
        cur = 0
        for _ in range(q):
            if r.range(0, 99) < 50:
                ops.append((0, 0, r.range(0, n - 1), r.range(-VMAX, VMAX)))
                cur += 1
            else:
                ops.append(rand_sum(r, n, cur))
        return fmt(n, a, ops)
    if seed == 6:
        n, q = NMAX, QMAX
        a = [r.range(-VMAX, VMAX) for _ in range(n)]
        return fmt(n, a, mixed(r, n, q, 50))
    if seed == 7:
        n, q = NMAX, QMAX
        a = [r.range(-VMAX, VMAX) for _ in range(n)]
        return fmt(n, a, mixed(r, n, q, 70))
    if seed == 8:
        n, q = NMAX, QMAX
        a = [r.range(-VMAX, VMAX) for _ in range(n)]
        ops = []
        cur = 0
        for _ in range(1000):
            ops.append(rand_set(r, n, cur))
            cur += 1
        for _ in range(q - 1000):
            ops.append(rand_sum(r, n, cur))
        return fmt(n, a, ops)
    if seed == 9:
        n, q = NMAX, QMAX
        a = [r.range(-VMAX, VMAX) for _ in range(n)]
        ops = []
        cur = 0
        for _ in range(q):
            if r.range(0, 1) == 0:
                i = 0 if r.range(0, 1) == 0 else n - 1
                ops.append((0, r.range(0, cur), i, r.range(-VMAX, VMAX)))
                cur += 1
            else:
                if r.range(0, 1) == 0:
                    ops.append((1, r.range(0, cur), 0, n))
                else:
                    l = r.range(0, n - 2)
                    ops.append((1, r.range(0, cur), l, l + 1))
        return fmt(n, a, ops)
    # seed 10, 11
    n = r.range(NMAX // 2, NMAX)
    q = r.range(QMAX // 2, QMAX)
    a = [r.range(-VMAX, VMAX) for _ in range(n)]
    return fmt(n, a, mixed(r, n, q, 40 + 20 * (seed % 2)))


def main() -> None:
    if len(sys.argv) < 2:
        raise SystemExit("usage: gen.py <seed>")
    sys.stdout.write(case(int(sys.argv[1])))


if __name__ == "__main__":
    main()
