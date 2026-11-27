#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""NTT の畳み込み (実行時の素数) の入力 (1 ケース 1 行の「n m t fill same mod seed」) を作る。

法 M は素数 (998244353、10^9+7、10^9+9、1004535809、469762049、1073741789、2、3 など)。
seed 0〜18 は本番のケース。小さい大きさ、同じ長さの 2 冪、細長い形、2 冪を少し超える長さ、a = b の
2 乗、全部 M - 1 の順に置く。T は n m T が 2^24 くらい (CRT は 3 本の NTT を回すので 998244353 の回より小さく取る)。
1000 以上の seed は愚直解 (brute.hpp) と突き合わせるための小さい入力。
"""
import sys

TARGET = 1 << 24
SIZES = [1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 64, 65, 127, 128, 129]
PRIMES = [998244353, 1000000007, 1000000009, 1004535809, 469762049, 1073741789, 2, 3]
SHAPES = [
    (1, 1), (3, 5), (8, 8), (16, 16), (64, 64),
    (1 << 10, 1 << 10), (1 << 12, 1 << 12), (1 << 14, 1 << 14),
    (1 << 16, 1 << 16), (1 << 18, 1 << 18), (1 << 20, 1 << 20),
    (1, 1 << 16), (16, 1 << 14), (1024, 1 << 18),
    ((1 << 15) + 1, (1 << 15) + 1), ((1 << 18) + 1, (1 << 18) + 1),
]


def work_t(n: int, m: int) -> int:
    # 細長い形は、積の項の数ではなく変換の長さで T を抑える (参照実装も核も NTT の長さで働く)。
    lg = max(1, (n + m - 1).bit_length() - 1)
    return max(
        1,
        min(
            1 << 16,
            TARGET // (n * m),
            (1 << 25) // ((n + m) * lg),
            (1 << 23) // (n + m),
        ),
    )


def case_list() -> list[tuple[int, int, int, int, int, int]]:
    out = [(n, m, work_t(n, m), 0, 0, PRIMES[i % len(PRIMES)]) for i, (n, m) in enumerate(SHAPES)]
    n = m = 1 << 14
    out.append((n, m, work_t(n, m), 1, 0, PRIMES[1]))
    n = m = 1 << 16
    out.append((n, m, work_t(n, m), 0, 1, PRIMES[1]))
    n = m = 1 << 12
    out.append((n, m, work_t(n, m), 0, 0, PRIMES[6]))
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
        mod = PRIMES[seed % len(PRIMES)]
    print(f"{n} {m} {t} {fill} {same} {mod} {seed * 1_000_003 + 17}")


if __name__ == "__main__":
    main()
