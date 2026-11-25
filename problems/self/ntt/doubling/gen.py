#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""NTT の変換の倍化 (mod 998244353) の入力 (1 ケース 1 行の「n t fill seed」) を作る。

seed 0〜20 は本番のケース。n が小さいものから始め、2^10〜2^22 を 1 つずつ、最後に値の端のケースを置く。
T は dft(n) + dft_doubling + idft(2n) の作業が 2^25 くらいになるように決め、2^14 で止める。
1000 以上の seed は愚直解 (brute.hpp) と突き合わせるための小さい入力。
"""
import sys

TARGET = 1 << 25
TCAP = 1 << 14


def case_list() -> list[tuple[int, int, int]]:
    out = [(1 << k, TCAP, 0) for k in range(5)]  # n = 1, 2, 4, 8, 16
    for k in range(10, 23):
        n = 1 << k
        out.append((n, max(1, min(TCAP, TARGET // (5 * n * k))), 0))
    for k in (12, 16, 18):
        n = 1 << k
        out.append((n, max(1, min(TCAP, TARGET // (5 * n * k))), 1))
    return out


def main() -> None:
    seed = int(sys.argv[1])
    if seed < 1000:
        cases = case_list()
        if not 0 <= seed < len(cases):
            raise SystemExit(f"seed {seed} は {len(cases)} 未満にしてください")
        n, t, fill = cases[seed]
    else:  # 1000 以上: 小さい入力
        n = 1 << (seed % 5)
        t = 1 + seed % 3
        fill = seed % 3
    print(f"{n} {t} {fill} {seed * 1_000_003 + 17}")


if __name__ == "__main__":
    main()
