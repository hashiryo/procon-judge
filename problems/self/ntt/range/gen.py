#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""NTT の範囲の積 (mod 998244353) の入力 (1 ケース 1 行の「n m lo hi t fill mode seed」) を作る。

seed 0〜17 は本番のケース。範囲の端の小さいケース、Newton 法の inv の段 (n = 2m、[m, 2m))、先頭、
末尾、中ほど、b を反転する middle product の順に置く。T は全体の積の n m が 2^26 くらいになるように
決める。1000 以上の seed は愚直解 (brute.hpp) と突き合わせるための小さい入力。
"""
import sys

TARGET = 1 << 26
SIZES = [1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 64, 65]


def work_t(n: int, m: int) -> int:
    return max(1, min(1 << 16, TARGET // (n * m), (1 << 23) // (n + m)))


def case_list() -> list[tuple[int, int, int, int, int, int]]:
    out = [
        (3, 5, 0, 1, 0, 0), (3, 5, 1, 4, 0, 0), (3, 5, 2, 7, 0, 0),
        (8, 8, 0, 8, 0, 0), (8, 8, 8, 15, 0, 0),
    ]
    for k in (10, 12, 14, 16, 18, 20):
        m = 1 << k
        out.append((2 * m, m, m, 2 * m, 0, 0))
    out += [
        (1 << 16, 1 << 16, 0, 64, 0, 0),
        (1 << 18, 1 << 18, 0, 1 << 17, 0, 0),
        (1 << 16, 1 << 16, (1 << 16) - 1, (1 << 17) - 1, 0, 0),
        (1 << 16, 1 << 16, 1 << 15, (1 << 15) + (1 << 14), 0, 0),
        (1 << 17, 1 << 16, (1 << 16) - 1, 1 << 17, 0, 1),
        (1 << 19, 1 << 18, (1 << 18) - 1, 1 << 19, 0, 1),
        (1 << 17, 1 << 16, 1 << 16, 1 << 17, 1, 0),
    ]
    return out


def main() -> None:
    seed = int(sys.argv[1])
    if seed < 1000:
        cases = case_list()
        if not 0 <= seed < len(cases):
            raise SystemExit(f"seed {seed} は {len(cases)} 未満にしてください")
        n, m, lo, hi, fill, mode = cases[seed]
        t = work_t(n, m)
    else:  # 1000 以上: 小さい入力
        n = SIZES[seed % len(SIZES)]
        m = SIZES[(seed // len(SIZES)) % len(SIZES)]
        top = n + m - 1
        lo = min([0, 1, max(0, n - 1), n // 2, max(0, top - 1)][seed % 5], top - 1)
        hi = [lo + 1, min(lo + 7, top), top, min(lo + max(1, n // 2), top)][(seed // 5) % 4]
        if hi <= lo:
            hi = lo + 1
        t = 1 + seed % 3
        fill = (seed // 11) % 3
        mode = (seed // 7) % 2
    print(f"{n} {m} {lo} {hi} {t} {fill} {mode} {seed * 1_000_003 + 17}")


if __name__ == "__main__":
    main()
