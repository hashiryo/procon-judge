"""abc222-h (Beautiful Binary Tree) の入力を作る。N を 1 行で出す。

seed が 0 から count - 1 までは本番のケース。例の 4 つ、角のケース (N が小さいもの、10^7 とその近く、
10^7 以下で最大の素数、2 のべき)、ランダム。計算量は N に比例するので、大きい N を多めにする。
seed が 1000 以上なら、部分木の数を数える O(N^2) の DP の愚直解で解ける N <= 60 を出す (pj testdata crosscheck 用)。
2000 以上なら、N <= 5000 を出す。
"""

import random
import sys

MAX = 10**7

FIXED = [
    1,  # 例 1
    2,  # 例 2
    222,  # 例 3
    222222,  # 例 4
    3,
    4,
    5,
    10,
    1000,
    20000,
    MAX,
    MAX - 1,
    9999991,  # 10^7 以下で最大の素数
    2**23,
    5 * 10**6,
]
RANDOM = 4


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 2000:
        n = rng.randint(61, 5000)
    elif seed >= 1000:
        n = rng.randint(1, 60)
    elif seed < len(FIXED):
        n = FIXED[seed]
    else:
        n = rng.randint(MAX // 2, MAX) if seed % 2 else rng.randint(1, MAX)
    assert 1 <= n <= MAX
    print(n)


if __name__ == "__main__":
    main()
