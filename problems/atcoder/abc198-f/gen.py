"""abc198-f (Cube) の入力を作る。S を 1 行で出す。

seed が 0 から count - 1 までは本番のケース。例の 4 つ、角のケース、ランダム。
答えは S - 6 を 12 で割った余りごとに、商 t の 5 次式になる。角のケースは、S = 6 と 7、
提出の持つ表の端 (S - 6 が 29 と 30、71 と 72)、S = 998244353、10^18 から下の 12 個 (余りを全部通る)、
t を 998244353 で割った余りが 0 や 5 や 6 や -1 になるもの (t の余りが 6 未満なら補間せずに表を引く)。
seed が 1000 以上なら、6 面の数の組を全部たどる愚直解で解ける S (300 まで) を出す (pj testdata crosscheck 用)。
"""

import random
import sys

P = 998244353
MIN = 6
MAX = 10**18
SMALL_MAX = 300


def with_t(rng: random.Random, j: int) -> int:
    """t = (S - 6) // 12 が P で割って j 余る大きい S。"""
    k = rng.randint(MAX // (12 * P) // 2, (MAX - 6 - 11) // (12 * P) - 1)
    return 6 + 12 * (P * k + j) + rng.randrange(12)


FIXED = [
    8,  # 例 1
    9,  # 例 2
    50,  # 例 3
    10**10,  # 例 4
    6,
    7,
    35,  # S - 6 = 29 (lib-oeis の持つ 30 項の最後)
    36,
    77,  # S - 6 = 71 (lib-interpolate の持つ 72 項の最後)
    78,
    P,
    12 * P + 6,  # t = P
] + [MAX - k for k in range(12)]
T_RESIDUES = [0, 5, 6, P - 1]
RANDOM = 4
COUNT = len(FIXED) + len(T_RESIDUES) + RANDOM


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 1000:
        kind = rng.randrange(3)
        if kind == 0:
            s = rng.randint(MIN, 40)
        elif kind == 1:
            s = rng.choice([6, 7, 35, 36, 77, 78, 79, 89, 90, 6 + 12 * 12, 6 + 12 * 20]) + rng.randint(0, 11)
        else:
            s = rng.randint(MIN, SMALL_MAX)
    elif seed < len(FIXED):
        s = FIXED[seed]
    elif seed < len(FIXED) + len(T_RESIDUES):
        s = with_t(rng, T_RESIDUES[seed - len(FIXED)])
    else:
        s = rng.randint(MIN, MAX)
    assert MIN <= s <= MAX
    print(s)


if __name__ == "__main__":
    main()
