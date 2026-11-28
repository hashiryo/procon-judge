#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""relaxed (オンライン) の畳み込みの入力 (1 ケース 1 行の「n m t fill same seed」) を作る。

seed 0 以降は本番のケース。小さい大きさ、同じ長さの 2 冪 (2^10〜2^18)、細長い形、2 冪を少し超える
長さ、n と m が違う組、a = b、全部 P - 1、全部 0 の順に置く。
T は n m T が 2^24 くらい、小さい n でも 2^10 で止める。ただし relaxed の費用は n m ではなく
(n + m) log^2 (n + m) に比例するので、細長い形や大きい形は (n + m) log^2 の見積もりでも抑える
(そうしないと (1, 2^16) の T が 2^8 になり、C++ が 1 ケースで数十秒かかる)。
1000 以上の seed は愚直解 (brute.hpp) と突き合わせるための小さい入力。
"""
import sys

TARGET = 1 << 24       # n m T の上限
ONLINE = 1 << 26       # (n + m) log2^2 (n + m) の T 倍の上限 (片方のオンラインの実測が 4 ns くらい)
SIZES = [1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 64, 65, 127, 128, 129]
SHAPES = [
    (1, 1), (3, 5), (5, 3), (8, 8), (16, 16), (64, 64), (100, 64),
    (1 << 10, 1 << 10), (1 << 12, 1 << 12), (1 << 14, 1 << 14),
    (1 << 16, 1 << 16), (1 << 18, 1 << 18),
    (1, 1 << 16), (1 << 16, 1), (16, 1 << 14), (1 << 14, 16),
    (1 << 10, 1 << 18), (1 << 18, 1 << 10),
    ((1 << 15) + 1, (1 << 15) + 1), ((1 << 18) + 1, (1 << 18) + 1),
    ((1 << 10) + 1, 1 << 10), (1 << 12, (1 << 12) + 1),
    (1 << 15, 1 << 10), (1 << 10, 1 << 15), (1 << 13, 3), (3, 1 << 13),
]


def work_t(n: int, m: int) -> int:
    s = n + m - 1
    lg = max(1, s.bit_length())
    return max(1, min(1 << 10, TARGET // (n * m), ONLINE // (s * lg * lg)))


def case_list() -> list[tuple[int, int, int, int, int]]:
    out = [(n, m, work_t(n, m), 0, 0) for n, m in SHAPES]
    out.append((1 << 12, 1 << 12, work_t(1 << 12, 1 << 12), 0, 1))  # a = b
    out.append((1 << 10, 1 << 10, work_t(1 << 10, 1 << 10), 0, 1))  # a = b (小さい)
    out.append((1 << 12, 1 << 12, work_t(1 << 12, 1 << 12), 1, 0))  # 全部 P - 1
    out.append((1 << 14, 1 << 14, work_t(1 << 14, 1 << 14), 1, 0))  # 全部 P - 1
    out.append((1 << 10, 1 << 10, work_t(1 << 10, 1 << 10), 2, 0))  # 全部 0
    return out


def main() -> None:
    seed = int(sys.argv[1])
    if seed < 1000:
        cases = case_list()
        if not 0 <= seed < len(cases):
            raise SystemExit(f"seed {seed} は {len(cases)} 未満にしてください")
        n, m, t, fill, same = cases[seed]
    else:  # 1000 以上: 小さい入力
        n = SIZES[seed % len(SIZES)]
        m = SIZES[(seed // len(SIZES)) % len(SIZES)]
        t = 1 + seed % 3
        fill = (seed // 19) % 3
        same = 1 if n == m and (seed // 13) % 2 == 0 else 0
    print(f"{n} {m} {t} {fill} {same} {seed * 1_000_003 + 17}")


if __name__ == "__main__":
    main()
