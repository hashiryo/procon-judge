#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""多項式の商と余りの入力 (1 ケース 1 行の「n m T seed」) を作る。

seed 0〜12 は m = 2^seed、n = 2m (1〜4096 次で割る)。13 と 14 は割る式が短い形と長い形。T は (n - m + 1)(m - 1) T が 2^24
くらいになるように決め、2^16 で止める。1000 以上の seed は愚直解 (brute.hpp) と突き合わせるための小さい入力。
"""
import sys

SHAPES = [(2 << s, 1 << s) for s in range(13)] + [(1 << 16, 3), (8192, 8000)]


def main() -> None:
    seed = int(sys.argv[1])
    if seed < len(SHAPES):
        n, m = SHAPES[seed]
        work = max(1, (n - m + 1) * max(1, m - 1))
        t = max(1, min(1 << 16, (1 << 24) // work))
    else:  # 1000 以上: 小さい入力
        m = 1 + seed % 6
        n, t = m + seed % 9, 50
    sys.stdout.write(f"{n} {m} {t} {seed * 1_000_003 + 29}\n")


if __name__ == "__main__":
    main()
