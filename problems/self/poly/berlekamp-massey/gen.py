#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""Berlekamp–Massey の入力 (1 ケース 1 行の「N T seed mode」) を作る。

seed 0〜13 は長さ N = 2^(seed+1) (2〜16384)。T は T N^2 が 2^26 くらいになるように決め、2^16 で止める。
1000 以上の seed は愚直解 (brute.hpp) と突き合わせるための小さい入力で、最短の漸化式が 1 つに決まる列だけにする (mode = 1)。
"""
import sys


def main() -> None:
    seed = int(sys.argv[1])
    if seed < 14:
        n = 2 << seed
        t = max(1, min(1 << 16, (1 << 26) // (n * n)))
        mode = 0
    else:  # 1000 以上: 小さい入力
        n, t, mode = 4 + seed % 12, 50, 1
    sys.stdout.write(f"{n} {t} {seed * 1_000_003 + 23} {mode}\n")


if __name__ == "__main__":
    main()
