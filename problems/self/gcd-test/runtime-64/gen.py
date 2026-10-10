#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""64 bit 整数の gcd の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

期待出力は参照実装 (problem.toml の reference) がハーネスと一緒に作るので、ここでは入力だけを書く。

入力: N amode bmode gmode seed の 1 行。(a_i, b_i) はハーネスが計測の前に splitmix64 で作る。
mode の意味は problems/_shared/gcd-test/_pairs.hpp にある。uB は B bit 以下の一様、l は桁数が一様、
e は角の値、gmode は a と b に掛ける共通因数 (1 なら掛けない)。

2026-10-10 に作り直した。旧 judge から移したときの入力は LCG の下位ビットを使っていて、
16 bit のケースは周期 65536 で同じ組を繰り返し、edge の 3 ケースは a と b の種が同じで a = b の組しかなかった。
大きさの違う組 (Stein の方法が苦手な、大きい数と小さい数の組) も足した。
"""
import sys

CASES = [
    ("small_00", 0, "u64", "u64", "1"),
    ("small_01", 1, "u64", "u64", "1"),
    ("small_02", 5, "u64", "u64", "1"),
    ("edge_00", 100_000, "e", "e", "1"),
    ("edge_01", 100_000, "e", "u64", "1"),
    ("edge_02", 100_000, "l", "e", "1"),
    ("rand_00", 100_000, "u64", "u64", "1"),
    ("rand_01", 1_000_000, "u64", "u64", "1"),
    ("bits16_00", 5_000_000, "u16", "u16", "1"),
    ("bits32_00", 5_000_000, "u32", "u32", "1"),
    ("heavy_00", 10_000_000, "u64", "u64", "1"),
    ("unbal_00", 5_000_000, "u64", "u32", "1"),
    ("unbal_01", 5_000_000, "u16", "u64", "1"),
    ("unbal_02", 5_000_000, "u64", "u8", "1"),
    ("logu_00", 10_000_000, "l", "l", "1"),
    ("common_00", 5_000_000, "u32", "u32", "u32"),
]


def main() -> None:
    seed = int(sys.argv[1])
    if not 0 <= seed < len(CASES):
        raise SystemExit(f"seed {seed} は {len(CASES)} 未満にしてください")
    _, n, am, bm, gm = CASES[seed]
    print(f"{n} {am} {bm} {gm} {1_000_003 * seed + 12345}")


if __name__ == "__main__":
    main()
