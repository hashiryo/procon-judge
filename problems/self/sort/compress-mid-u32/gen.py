#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""長さ 10^4 から 5 × 10^5 の u32 の座標圧縮の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。
self-sort-compress-u32 (10^6 個が中心) と同じ形で、競プロでよく出る長さの速さを見る。1 ケースに同じ長さの配列を何本か並べ、
合わせて 10^6 個ほどにして、ケースの時間が短くなりすぎないようにする。

入力は配列の作り方だけで、配列そのものはハーネスが作る (_shared/sort/_common.hpp)。
1 行目が配列の個数 T、続く T 行が「分布 N seed 引数」で、引数を使わない分布でも 0 を書く。
期待出力は参照実装 (problem.toml の reference) がハーネスと一緒に作るので、ここでは入力だけを書く。
seed 0 の小さい配列の選び方は、Python の random の版による違いを避けるため splitmix64 で決める。
"""
import sys

MASK64 = (1 << 64) - 1
U32_MAX = (1 << 32) - 1
M = 10**6


class SplitMix64:
    def __init__(self, seed: int) -> None:
        self.x = seed & MASK64

    def next(self) -> int:
        self.x = (self.x + 0x9E3779B97F4A7C15) & MASK64
        z = self.x
        z = ((z ^ (z >> 30)) * 0xBF58476D1CE4E5B9) & MASK64
        z = ((z ^ (z >> 27)) * 0x94D049BB133111EB) & MASK64
        return z ^ (z >> 31)

    def below(self, n: int) -> int:
        return (self.next() * n) >> 64

    def choice(self, xs: list):
        return xs[self.below(len(xs))]


# seed -> 配列の並び (分布, N, 引数)
def rows(dist: str, n: int, arg: int, k: int) -> list[tuple[str, int, int]]:
    return [(dist, n, arg)] * k


CASES = {
    0: rows("range", 1 << 14, U32_MAX, 64),
    1: rows("range", 30000, 10**9, 32),
    2: rows("range", 100000, U32_MAX, 10),
    3: rows("range", 100000, 10**9, 10),
    4: rows("range", 100000, 99999, 10),
    5: rows("range", 200000, 10**9, 5),
    6: rows("range", 200000, U32_MAX, 5),
    7: rows("range", 500000, 10**9, 2),
    8: rows("sorted", 100000, 0, 10),
    9: rows("few", 100000, 16, 10),
    10: rows("nearly", 100000, 1000, 10),
    11: rows("mask", 100000, 0xFFFF0000, 10),
}


def main() -> None:
    seed = int(sys.argv[1])
    if seed not in CASES:
        raise SystemExit(f"seed {seed} は {len(CASES)} 未満にしてください")
    rows = CASES[seed]
    out = [str(len(rows))]
    # 配列ごとの乱数の種は、ケースの seed と行の番号から決める。
    out += [f"{dist} {n} {seed * 1_000_003 + i} {arg}" for i, (dist, n, arg) in enumerate(rows)]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
