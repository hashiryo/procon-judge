#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""係数の補間の入力 (1 ケース 1 行の「n T seed」) を作る。

seed 0〜12 は点の数 n = 2^seed (1〜4096)。T は T n^2 が 2^24 くらいになるように決め、2^16 で止める。
1000 以上の seed は愚直解 (brute.hpp) と突き合わせるための小さい入力 (点は 7 個以下)。
"""
import sys


def main() -> None:
    seed = int(sys.argv[1])
    if seed < 13:
        n = 1 << seed
        t = max(1, min(1 << 16, (1 << 24) // (n * n)))
    else:  # 1000 以上: 小さい入力
        n, t = 1 + seed % 7, 50
    sys.stdout.write(f"{n} {t} {seed * 1_000_003 + 13}\n")


if __name__ == "__main__":
    main()
