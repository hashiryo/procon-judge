#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""永続 Li Chao 木 (線分追加・最小) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

書式:
  q
  続く q 行は 0 v l r a b (版 v から新しい版を作り、[l,r) に y = a x + b を足す) か、
  1 v x (版 v の x での最小。無ければ INFINITY)。版 0 は空。

座標は -1000000 以上 1000000 以下、a, b は -10^9 以上 10^9 以下。
seed 0 から 11 は本番のケース。例、q = 1、追加だけ、鎖、星、ランダム、長い線分、短い線分、
クエリだけ、端、中くらい。
seed 1000 以上は、愚直解で解ける q <= 20 の小さい入力を出す。
"""
import sys

MASK64 = (1 << 64) - 1
QMAX = 20000
CMAX = 1000000
AMAX = 10**9
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


def fmt(ops: list) -> str:
    out = [str(len(ops))]
    out += [" ".join(map(str, op)) for op in ops]
    return "\n".join(out) + "\n"


def rand_seg(r: SplitMix64, cmax: int, amax: int) -> tuple:
    l = r.range(-cmax, cmax - 1)
    return (l, r.range(l + 1, cmax), r.range(-amax, amax), r.range(-amax, amax))


def rand_query(r: SplitMix64, cur: int, cmax: int) -> tuple:
    return (1, r.range(0, cur), r.range(-cmax, cmax))


def small(seed: int) -> str:
    """愚直解で解ける q <= 20 の入力。"""
    r = SplitMix64(seed * 7919 + 17)
    q = r.range(1, 20)
    cur = 0
    ops = []
    for i in range(q):
        is_add = i % 2 == 0 if q > 1 else True
        if is_add:
            l = r.range(-8, 7)
            rr = r.range(l + 1, 8)
            ops.append((0, r.range(0, cur), l, rr, r.range(-10, 10), r.range(-10, 10)))
            cur += 1
        else:
            ops.append((1, r.range(0, cur), r.range(-8, 8)))
    return fmt(ops)


def case(seed: int) -> str:
    if seed >= 1000:
        return small(seed)
    r = SplitMix64(seed * 1000003 + 12345)
    if seed == 0:
        return (
            "8\n"
            "1 0 0\n"
            "0 0 0 5 1 0\n"
            "1 1 3\n"
            "0 1 2 4 10 0\n"
            "1 2 3\n"
            "1 2 1\n"
            "0 0 0 5 0 5\n"
            "1 3 0\n"
        )
    if seed == 1:
        q = 1
        ops = [(1, 0, 0)]
        return fmt(ops)
    if seed == 2:
        q = 2000
        cur = 0
        ops = []
        for _ in range(q // 2):
            ops.append((0, r.range(0, cur), *rand_seg(r, CMAX, AMAX)))
            cur += 1
        for _ in range(q - q // 2):
            ops.append(rand_query(r, cur, CMAX))
        return fmt(ops)
    if seed == 3:
        q = QMAX
        cur = 0
        ops = []
        for _ in range(q):
            if r.range(0, 1) == 0:
                ops.append((0, cur, *rand_seg(r, CMAX, AMAX)))
                cur += 1
            else:
                ops.append(rand_query(r, cur, CMAX))
        return fmt(ops)
    if seed == 4:
        q = QMAX
        cur = 0
        ops = []
        for _ in range(q):
            if r.range(0, 1) == 0:
                ops.append((0, 0, *rand_seg(r, CMAX, AMAX)))
                cur += 1
            else:
                ops.append(rand_query(r, cur, CMAX))
        return fmt(ops)
    if seed == 5:
        q = QMAX
        cur = 0
        ops = []
        for _ in range(q):
            if r.range(0, 1) == 0:
                ops.append((0, r.range(0, cur), *rand_seg(r, CMAX, AMAX)))
                cur += 1
            else:
                ops.append(rand_query(r, cur, CMAX))
        return fmt(ops)
    if seed == 6:
        q = QMAX
        cur = 0
        ops = []
        for _ in range(q):
            if r.range(0, 1) == 0:
                ops.append((0, r.range(0, cur), -CMAX, CMAX, r.range(-AMAX, AMAX), r.range(-AMAX, AMAX)))
                cur += 1
            else:
                ops.append(rand_query(r, cur, CMAX))
        return fmt(ops)
    if seed == 7:
        q = QMAX
        cur = 0
        ops = []
        for _ in range(q):
            if r.range(0, 1) == 0:
                l = r.range(-CMAX, CMAX - 1)
                ops.append((0, r.range(0, cur), l, l + 1, r.range(-AMAX, AMAX), r.range(-AMAX, AMAX)))
                cur += 1
            else:
                ops.append(rand_query(r, cur, CMAX))
        return fmt(ops)
    if seed == 8:
        q = QMAX
        cur = 0
        ops = []
        for _ in range(q // 10):
            ops.append((0, r.range(0, cur), *rand_seg(r, CMAX, AMAX)))
            cur += 1
        for _ in range(q - q // 10):
            ops.append(rand_query(r, cur, CMAX))
        return fmt(ops)
    if seed == 9:
        q = QMAX
        cur = 0
        ops = []
        for i in range(q):
            if i % 2 == 0:
                l = -CMAX if i % 4 == 0 else CMAX - 1
                rr = l + 1 if l == CMAX - 1 else CMAX
                ops.append((0, r.range(0, cur), l, rr, r.range(-AMAX, AMAX), r.range(-AMAX, AMAX)))
                cur += 1
            else:
                x = -CMAX if i % 4 == 1 else CMAX
                ops.append((1, r.range(0, cur), x))
        return fmt(ops)
    # seed 10, 11
    q = r.range(QMAX // 2, QMAX)
    cur = 0
    ops = []
    for _ in range(q):
        if r.range(0, 1) == 0:
            ops.append((0, r.range(0, cur), *rand_seg(r, CMAX, AMAX)))
            cur += 1
        else:
            ops.append(rand_query(r, cur, CMAX))
    return fmt(ops)


def main() -> None:
    if len(sys.argv) < 2:
        raise SystemExit("usage: gen.py <seed>")
    sys.stdout.write(case(int(sys.argv[1])))


if __name__ == "__main__":
    main()
