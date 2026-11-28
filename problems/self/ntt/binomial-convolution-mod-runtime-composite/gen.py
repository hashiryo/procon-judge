#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""二項畳み込み (実行時の合成数) の入力 (1 ケース 1 行の「n m t fill same mod seed」) を作る。

法 M は合成数 (10^9、998244352 = 2^23 7 17、999999999 = 3^4 37 333667、2^29、735134400 = 2^6 3^3 5^2 7 11 13 17、
12、1)。M の素因数が n + m - 1 未満だと階乗が法で割り切れるので、p を抜いた階乗で分けた和を
NTT の素数で求めて Garner で戻す道を通る (素因数の多い法ほど素数の本数が増える)。
seed 0〜18 は本番のケース。小さい大きさ、同じ長さの 2 冪、細長い形、2 冪を少し超える長さ、
a = b の 2 乗、全部 M - 1 の順に置く。
T は n m T が 2^23 くらい (道は NTT を数本回すので、普通の畳み込みの回より小さく取る)。
1000 以上の seed は愚直解 (brute.hpp) と突き合わせるための小さい入力。
"""
import sys

TARGET = 1 << 23
SIZES = [1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 64, 65, 127, 128, 129]
COMPOSITES = [1000000000, 998244352, 999999999, 536870912, 735134400, 12, 1]
CASES = [
    (1, 1, 12),
    (3, 5, 1000000000),
    (8, 8, 998244352),
    (16, 16, 999999999),
    (64, 64, 536870912),
    (1 << 10, 1 << 10, 1000000000),
    (1 << 12, 1 << 12, 998244352),
    (1 << 14, 1 << 14, 1000000000),
    (1 << 16, 1 << 16, 998244352),
    (1 << 18, 1 << 18, 536870912),
    (1 << 18, 1 << 18, 1000000000),
    (1, 1 << 16, 1000000000),
    (16, 1 << 14, 998244352),
    ((1 << 15) + 1, (1 << 15) + 1, 999999999),
    (1 << 12, 1 << 12, 735134400),
    (1 << 14, 1 << 14, 1000000000),
    (1 << 16, 1 << 16, 998244352),
    (1 << 12, 1 << 12, 12),
    (1, 1, 1),
]


def work_t(n: int, m: int) -> int:
    lg = max(1, (n + m - 1).bit_length() - 1)
    return max(
        1,
        min(
            1 << 16,
            TARGET // (n * m),
            (1 << 23) // ((n + m) * lg),
            (1 << 22) // (n + m),
        ),
    )


def case_list() -> list[tuple[int, int, int, int, int, int]]:
    out = [(n, m, work_t(n, m), 0, 0, mod) for n, m, mod in CASES]
    assert len(out) == 19
    out[15] = (out[15][0], out[15][1], out[15][2], 1, 0, out[15][5])
    out[16] = (out[16][0], out[16][1], out[16][2], 0, 1, out[16][5])
    return out


def main() -> None:
    seed = int(sys.argv[1])
    if seed < 1000:
        cases = case_list()
        if not 0 <= seed < len(cases):
            raise SystemExit(f"seed {seed} は {len(cases)} 未満にしてください")
        n, m, t, fill, same, mod = cases[seed]
    else:  # 1000 以上: 小さい入力
        n = SIZES[seed % len(SIZES)]
        m = SIZES[(seed // len(SIZES)) % len(SIZES)]
        t = 1 + seed % 3
        fill = (seed // 19) % 3
        same = 1 if n == m and (seed // 13) % 2 == 0 else 0
        mod = COMPOSITES[seed % len(COMPOSITES)]
    print(f"{n} {m} {t} {fill} {same} {mod} {seed * 1_000_003 + 17}")


if __name__ == "__main__":
    main()
