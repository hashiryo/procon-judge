#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""線形漸化式の第 k 項の入力 (1 ケース 1 行の「d T kmax seed」) を作る。

seed 0〜11 は次数 d = 2^seed (1〜2048) で、k は 2^60 未満の乱数。T は T d^2 が 2^22 くらいになるように決め、2^16 で止める。
1000 以上の seed は愚直解 (brute.hpp) と突き合わせるための小さい入力 (次数 6 以下、k < 3000)。
"""
import sys


def main() -> None:
    seed = int(sys.argv[1])
    if seed < 12:
        d = 1 << seed
        t = max(1, min(1 << 16, (1 << 22) // (d * d)))
        kmax = 1 << 60
    else:  # 1000 以上: 小さい入力
        d, t, kmax = 1 + seed % 6, 100, 3000
    sys.stdout.write(f"{d} {t} {kmax} {seed * 1_000_003 + 7}\n")


if __name__ == "__main__":
    main()
