#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""連続する点の値からの 1 点の値の入力 (1 ケース 1 行の「K T seed」) を作る。

seed 0〜22 は点の数 K = 2^seed (1〜2^22)。T は T K が 2^24 くらいになるように決め、2^16 で止める。
1000 以上の seed は愚直解 (brute.hpp) と突き合わせるための小さい入力 (点は 10 個以下)。
"""
import sys


def main() -> None:
    seed = int(sys.argv[1])
    if seed < 23:
        k = 1 << seed
        t = max(1, min(1 << 16, (1 << 24) // k))
    else:  # 1000 以上: 小さい入力
        k, t = 1 + seed % 10, 50
    sys.stdout.write(f"{k} {t} {seed * 1_000_003 + 19}\n")


if __name__ == "__main__":
    main()
