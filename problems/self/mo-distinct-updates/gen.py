#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""点更新つき区間 distinct の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

書式 (1-indexed):
  n q
  a_1 ... a_n
  続く q 行は 0 p x (a_p を x に変える) か、1 l r (閉区間 [l, r] の distinct 数を出す)。

seed 0 から 11 は本番のケース。小さい例、n = 1、全部同じ、全部異なる、更新なし、
更新だけのあとにクエリ、交互、値が 5 種類だけ、同じ位置への連続更新、値が 1e9 付近、
n = q = 1e5 のランダム、クエリが全部 [1, n] で更新だけ動く形を並べる。

seed 1000 以上は、クエリごとに区間を走査する愚直解 (brute.cpp) で解ける n, q <= 40 の
小さい入力を出す。形 (全部クエリ、全部更新、交互、前半だけ更新、ランダム) と大きさを
splitmix64 で選ぶ。
"""
import sys

MASK64 = (1 << 64) - 1
NMAX = 100000
QMAX = 100000
VMAX = 1000000000
# 本番のケースの数。seed 0 から COUNT-1 がテストデータになる。
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
        return lo + (self.next() * (hi - lo + 1) >> 64)


def fmt(n: int, a: list, ops: list) -> str:
    lines = [f"{n} {len(ops)}", " ".join(map(str, a))]
    lines += [f"{t} {x} {y}" for t, x, y in ops]
    return "\n".join(lines) + "\n"


def rand_query(r: SplitMix64, n: int) -> tuple:
    """1 <= l <= r <= n のクエリ。"""
    l = r.range(1, n)
    return (1, l, r.range(l, n))


def rand_update(r: SplitMix64, n: int, vmax: int) -> tuple:
    return (0, r.range(1, n), r.range(1, vmax))


def mixed(r: SplitMix64, n: int, q: int, vmax: int) -> list:
    """更新とクエリを半々に混ぜる。"""
    return [rand_update(r, n, vmax) if r.range(0, 1) == 0 else rand_query(r, n) for _ in range(q)]


def small(seed: int) -> str:
    """愚直解で解ける n, q <= 40 の入力。形と大きさを seed から決める。"""
    r = SplitMix64(seed * 7919 + 17)
    n = r.range(1, 40)
    q = r.range(1, 40)
    vmax = (1, 2, 3, 5, 10)[r.range(0, 4)]
    a = [r.range(1, vmax) for _ in range(n)]
    shape = r.range(0, 4)
    ops = []
    for i in range(q):
        if shape == 0:
            is_update = False
        elif shape == 1:
            is_update = True
        elif shape == 2:
            is_update = i % 2 == 0
        elif shape == 3:
            is_update = i < q // 2
        else:
            is_update = r.range(0, 1) == 0
        if is_update:
            ops.append((0, r.range(1, n), r.range(1, vmax)))
        else:
            ops.append(rand_query(r, n))
    return fmt(n, a, ops)


def case(seed: int) -> str:
    r = SplitMix64(seed * 1000003 + 12345)
    if seed == 0:
        # 例。更新の前後で答えが変わる小さな形。
        return (
            "5 6\n"
            "1 2 1 3 2\n"
            "1 1 5\n"
            "0 2 3\n"
            "1 2 4\n"
            "0 5 3\n"
            "1 1 5\n"
            "1 5 5\n"
        )
    if seed == 1:
        # n = 1。1 要素の書き換えと、同じ値への書き換え (何も変わらない更新)。
        return fmt(
            1,
            [VMAX],
            [
                (1, 1, 1),
                (0, 1, VMAX - 1),
                (1, 1, 1),
                (0, 1, VMAX - 1),
                (1, 1, 1),
                (0, 1, 1),
                (1, 1, 1),
            ],
        )
    if seed == 2:
        # 全部同じ値。更新も同じ値へ書き換えるので、答えはいつも 1。
        n = NMAX
        a = [5] * n
        ops = [(0, r.range(1, n), 5) if r.range(0, 1) == 0 else rand_query(r, n) for _ in range(QMAX)]
        return fmt(n, a, ops)
    if seed == 3:
        # 全部異なる値。更新先も初期値と重ならない値にして、distinct が大きいまま動く。
        n = NMAX
        a = list(range(1, n + 1))
        ops = []
        for k in range(QMAX):
            if r.range(0, 1) == 0:
                ops.append((0, r.range(1, n), VMAX - k))
            else:
                ops.append(rand_query(r, n))
        return fmt(n, a, ops)
    if seed == 4:
        # 更新なし。時間軸が動かない形。
        n = NMAX
        a = [r.range(1, VMAX) for _ in range(n)]
        return fmt(n, a, [rand_query(r, n) for _ in range(QMAX)])
    if seed == 5:
        # 更新だけを並べ、最後にクエリ。どのクエリも t が同じで、t は 1 回だけ動く。
        n = NMAX
        a = [r.range(1, 1000) for _ in range(n)]
        ops = [rand_update(r, n, VMAX) for _ in range(QMAX // 2)]
        ops += [rand_query(r, n) for _ in range(QMAX // 2)]
        return fmt(n, a, ops)
    if seed == 6:
        # 更新とクエリの交互。
        n = NMAX
        a = [r.range(1, 1000) for _ in range(n)]
        ops = []
        for _ in range(QMAX // 2):
            ops.append(rand_update(r, n, VMAX))
            ops.append(rand_query(r, n))
        return fmt(n, a, ops)
    if seed == 7:
        # 値が 5 種類だけ。distinct の数が小さいまま、位置の分布だけが動く。
        n = NMAX
        a = [r.range(1, 5) for _ in range(n)]
        return fmt(n, a, mixed(r, n, QMAX, 5))
    if seed == 8:
        # 同じ位置への連続更新。古い値と新しい値の対応が続けて起きる形。
        n = NMAX
        a = [r.range(1, 100) for _ in range(n)]
        ops = []
        while len(ops) < QMAX:
            p = r.range(1, n)
            for _ in range(r.range(5, 40)):
                if len(ops) >= QMAX:
                    break
                ops.append((0, p, r.range(1, VMAX)))
            for _ in range(r.range(1, 4)):
                if len(ops) >= QMAX:
                    break
                ops.append(rand_query(r, n))
        return fmt(n, a, ops)
    if seed == 9:
        # 値が 1e9 付近。座標圧縮と int の扱いを見る。
        n = NMAX
        a = [VMAX - r.range(0, 1000000) for _ in range(n)]
        ops = []
        for _ in range(QMAX):
            if r.range(0, 1) == 0:
                ops.append((0, r.range(1, n), VMAX - r.range(0, 1000000)))
            else:
                ops.append(rand_query(r, n))
        return fmt(n, a, ops)
    if seed == 10:
        # n = q = 1e5 のランダム。制約いっぱい。
        n = NMAX
        a = [r.range(1, VMAX) for _ in range(n)]
        return fmt(n, a, mixed(r, n, QMAX, VMAX))
    if seed == 11:
        # クエリが全部 [1, n]。動くのは時間軸だけ。
        n = NMAX
        a = [r.range(1, 1000) for _ in range(n)]
        ops = []
        for _ in range(QMAX // 2):
            ops.append(rand_update(r, n, VMAX))
            ops.append((1, 1, n))
        return fmt(n, a, ops)
    raise AssertionError(f"本番のケース {seed} がありません")


def main() -> None:
    seed = int(sys.argv[1])
    if 0 <= seed < COUNT:
        sys.stdout.write(case(seed))
    elif seed >= 1000:
        sys.stdout.write(small(seed))
    else:
        raise SystemExit(f"seed {seed} は 0 から {COUNT - 1} か、1000 以上にしてください")


if __name__ == "__main__":
    main()
