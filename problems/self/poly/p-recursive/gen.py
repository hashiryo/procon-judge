#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""多項式係数の線形漸化式の第 N 項の入力 (1 ケース 1 行の「d e t nmax kind seed」) を作る。

seed 0〜12 は階数 d と次数 e と N の大きさの組。t は (d + 1) e と N の平方根で決まる手間に合わせて、
1 ケースの時間がそろうように置いた。kind 0 は p_d = 1、1 は p_d = 0 でない定数、2 は p_d = n + 1 (nmax が小さい)。
1000 以上の seed は愚直解 (brute.hpp) と突き合わせるための小さい入力。
"""
import sys

# d, e, t, nmax, kind
CASES = [
    (1, 3, 1, 10**10, 1),
    (2, 2, 1, 10**10, 0),
    (3, 2, 1, 10**9, 1),
    (4, 1, 1, 10**9, 0),
    (2, 0, 2, 10**9, 0),
    (1, 1, 4, 10**8, 0),
    (2, 3, 4, 10**7, 1),
    (3, 3, 8, 10**6, 0),
    (2, 2, 32, 10**5, 0),
    (4, 3, 128, 10**4, 1),
    (1, 0, 2000, 10**3, 0),
    (2, 2, 100000, 10, 0),
    (3, 1, 1000, 100, 2),
]


def main() -> None:
    seed = int(sys.argv[1])
    if seed < len(CASES):
        d, e, t, nmax, kind = CASES[seed]
    else:  # 1000 以上: 小さい入力
        d = 1 + seed % 4
        e = seed % 3
        t = 20
        nmax = 3000
        kind = seed % 3
    sys.stdout.write(f"{d} {e} {t} {nmax} {kind} {seed * 1_000_003 + 7}\n")


if __name__ == "__main__":
    main()
