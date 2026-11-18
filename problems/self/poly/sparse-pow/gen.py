#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""低い次数の多項式の冪の入力 (1 ケース 1 行の「N D T seed」) を作る。

seed 0〜6 は N = 2^20 で次数 D = 1, 2, 4, 8, 16, 32, 64、7〜14 は D = 4 で N = 2^4, 2^6, ..., 2^18。T は N (D + 1) T が 2^24
くらいになるように決め、2^16 で止める。1000 以上の seed は愚直解 (brute.hpp) と突き合わせるための小さい入力。
"""
import sys

SHAPES = [(1 << 20, 1 << s) for s in range(7)] + [(1 << (4 + 2 * s), 4) for s in range(8)]


def main() -> None:
    seed = int(sys.argv[1])
    if seed < len(SHAPES):
        n, d = SHAPES[seed]
        t = max(1, min(1 << 16, (1 << 24) // (n * (d + 1))))
    else:  # 1000 以上: 小さい入力
        n, t = 1 + seed % 23, 50
        d = min(seed % 6, n - 1)  # 項の次数は n 未満 (yosupo と同じ)
    sys.stdout.write(f"{n} {d} {t} {seed * 1_000_003 + 31}\n")


if __name__ == "__main__":
    main()
