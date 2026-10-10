#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""32 bit 整数の拡張 gcd の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

期待出力は参照実装 (problem.toml の reference) がハーネスと一緒に作るので、ここでは入力だけを書く。

入力: N amode bmode gmode seed の 1 行。(a_i, b_i) はハーネスが計測の前に splitmix64 で作り (値の幅は 32 bit)、
b = 0 の組は b = 1 にする。mode の意味は problems/_shared/gcd-test/_pairs.hpp にある。
self-gcd-test-inv-gcd-64 を 32 bit に縮めた形で、法が 2^30 未満の ModInt の逆元もここに入る。
"""
import sys

CASES = [
    ("small_00", 0, "u32", "u32", "1"),
    ("small_01", 1, "u32", "u32", "1"),
    ("small_02", 5, "u32", "u32", "1"),
    ("edge_00", 100_000, "e", "e", "1"),
    ("edge_01", 100_000, "u32", "e", "1"),
    ("edge_02", 100_000, "e", "l", "1"),
    ("rand_00", 100_000, "u32", "u32", "1"),
    ("rand_01", 1_000_000, "u32", "u32", "1"),
    ("heavy_00", 10_000_000, "u32", "u32", "1"),
    ("unbal_00", 5_000_000, "u32", "u16", "1"),
    ("unbal_01", 5_000_000, "u8", "u32", "1"),
    ("logu_00", 10_000_000, "l", "l", "1"),
    ("prime_00", 5_000_000, "r", "p", "1"),
    ("common_00", 5_000_000, "u16", "u16", "u16"),
]


def main() -> None:
    seed = int(sys.argv[1])
    if not 0 <= seed < len(CASES):
        raise SystemExit(f"seed {seed} は {len(CASES)} 未満にしてください")
    _, n, am, bm, gm = CASES[seed]
    print(f"{n} {am} {bm} {gm} {1_000_003 * seed + 65432}")


if __name__ == "__main__":
    main()
