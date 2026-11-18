#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""素朴な畳み込みの入力 (1 ケース 1 行の「n m T seed」) を作る。

seed 0〜13 は n = m = 2^seed (1〜8192)、14〜16 は片方だけ長い形。T は n m T が 2^26 くらいになるように決め、2^18 と
(入力と答えの大きさを抑えるため) 2^23 / (n + m) で止める。
1000 以上の seed は愚直解 (brute.hpp) と突き合わせるための小さい入力。
"""
import sys

SHAPES = [(1 << s, 1 << s) for s in range(14)] + [(1, 1 << 16), (16, 1 << 14), (128, 4096)]


def main() -> None:
    seed = int(sys.argv[1])
    if seed < len(SHAPES):
        n, m = SHAPES[seed]
        t = max(1, min(1 << 18, (1 << 26) // (n * m), (1 << 23) // (n + m)))
    else:  # 1000 以上: 小さい入力
        n, m, t = 1 + seed % 7, 1 + seed % 11, 50
    sys.stdout.write(f"{n} {m} {t} {seed * 1_000_003 + 17}\n")


if __name__ == "__main__":
    main()
