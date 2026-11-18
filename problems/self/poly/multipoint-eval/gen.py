#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""多点評価の入力 (1 ケース 1 行の「n m T seed」) を作る。

seed 0〜12 は n = m = 2^seed (1〜4096)、13 と 14 は (n, m) = (16, 65536) と (65536, 16)。T は n m T が 2^26 くらい、
(n + m) T が 2^23 以下になるように決め、2^16 で止める。1000 以上の seed は愚直解 (brute.hpp) と突き合わせるための小さい入力。
"""
import sys

SHAPES = [(1 << s, 1 << s) for s in range(13)] + [(16, 1 << 16), (1 << 16, 16)]


def main() -> None:
    seed = int(sys.argv[1])
    if seed < len(SHAPES):
        n, m = SHAPES[seed]
        t = max(1, min(1 << 16, (1 << 26) // (n * m), (1 << 23) // (n + m)))
    else:  # 1000 以上: 小さい入力
        n, m, t = 1 + seed % 9, 1 + seed % 37, 50
    sys.stdout.write(f"{n} {m} {t} {seed * 1_000_003 + 37}\n")


if __name__ == "__main__":
    main()
