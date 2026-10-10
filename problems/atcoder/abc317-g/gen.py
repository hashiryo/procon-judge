#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""ABC 317 G (Rearranging) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

N 行 M 列のグリッドに、1 から N がちょうど M 個ずつ書かれている。
seed 0 から 11 は本番のケース。seed 1000 以上は、行ごとの並べ替えを全部試す愚直解 (brute.cpp) で解ける N, M ≤ 3 の入力。
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

    def shuffle(self, xs: list) -> None:
        for i in range(len(xs) - 1, 0, -1):
            j = self.range(0, i)
            xs[i], xs[j] = xs[j], xs[i]


def fmt(n: int, m: int, cells: list, r: SplitMix64, shuffle_rows: bool = True) -> str:
    rows = [cells[i * m : (i + 1) * m] for i in range(n)]
    if shuffle_rows:
        for row in rows:
            r.shuffle(row)
    return f"{n} {m}\n" + "".join(" ".join(map(str, row)) + "\n" for row in rows)


def uniform(r: SplitMix64, n: int, m: int) -> list:
    cells = [v for v in range(1, n + 1) for _ in range(m)]
    r.shuffle(cells)
    return cells


def case(seed: int) -> str:
    r = SplitMix64(seed * 1000003 + 317)
    if seed == 0:
        return "3 2\n1 1\n2 3\n2 3\n"
    if seed == 1:
        return "4 4\n1 2 3 4\n1 1 1 2\n3 2 2 4\n4 4 3 3\n"
    if seed == 2:  # 一様乱択
        return fmt(100, 100, uniform(r, 100, 100), r)
    if seed == 3:  # 行 i は数 i だけ
        return fmt(100, 100, [i + 1 for i in range(100) for _ in range(100)], r)
    if seed == 4:  # 行 i は数 i と i + 1 を 50 個ずつ (行と数のグラフが重みの付いた 1 本の閉路になる)
        return fmt(100, 100, [(i + k % 2) % 100 + 1 for i in range(100) for k in range(100)], r)
    if seed == 5:  # M = 1
        cells = list(range(1, 101))
        r.shuffle(cells)
        return fmt(100, 1, cells, r)
    if seed == 6:  # N = 1
        return fmt(1, 100, [1] * 100, r)
    if seed == 7:  # N = 2
        return fmt(2, 100, uniform(r, 2, 100), r)
    if seed == 8:  # M = 2
        return fmt(100, 2, uniform(r, 100, 2), r)
    if seed == 9:  # 前半の行は前半の数だけ、後半の行は後半の数だけを使う
        a = [v for v in range(1, 51) for _ in range(100)]
        b = [v for v in range(51, 101) for _ in range(100)]
        r.shuffle(a), r.shuffle(b)
        return fmt(100, 100, a + b, r)
    if seed == 10:  # 行ごとに数の種類が少ない (行 i は i から i + 4 の 5 種類を 20 個ずつ)
        return fmt(100, 100, [(i + k % 5) % 100 + 1 for i in range(100) for k in range(100)], r)
    if seed == 11:  # N = 100、M = 37 の一様乱択
        return fmt(100, 37, uniform(r, 100, 37), r)
    # seed 1000 以上: 愚直解で解ける小さい入力
    n, m = r.range(1, 3), r.range(1, 3)
    return fmt(n, m, uniform(r, n, m), r)


def main() -> None:
    seed = int(sys.argv[1])
    if not (0 <= seed <= 11 or seed >= 1000):
        raise SystemExit(f"seed {seed} は 0 から 11 か、1000 以上にしてください")
    sys.stdout.write(case(seed))


if __name__ == "__main__":
    main()
