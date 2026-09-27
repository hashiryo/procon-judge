"""abc276-g (Count Sequences) の入力を作る。N M を 1 行で出す。

seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース、ランダム。
角のケースは、N - 1 > M で答えが 0 のもの、N - 1 = M で答えが 1 のもの (0, 1, ..., M だけ)、N = M = 10^7、
N が小さくて M = 10^7 のもの。提出の計算量は M - N + 1 に比例するので、N が小さく M が大きいものが遅い。
seed が 1000 以上なら、値ごとの DP の愚直解で解ける小さい入力を出す (pj testdata crosscheck 用)。
2000 以上なら、N M <= 3 × 10^7 の入力を出す。
"""

import random
import sys

MAX = 10**7

FIXED = [
    (3, 4),  # 例 1
    (276, MAX),  # 例 2
    (2, 1),
    (3, 1),  # N - 1 > M なので 0
    (4, 3),  # N - 1 = M なので (0, 1, 2, 3) だけ
    (MAX, 1),
    (MAX, MAX - 2),
    (MAX, MAX - 1),
    (MAX, MAX),
    (100, 1000),
    (2, MAX),
    (3, MAX),
    (10, MAX),
    (1000, MAX),
    (MAX // 2, MAX),
]
RANDOM = 4


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 2000:
        n = rng.randint(2, 3000)
        m = rng.randint(max(1, n - 3), min(MAX, 3 * 10**7 // n))
    elif seed >= 1000:
        n = rng.randint(2, 30)
        m = rng.randint(max(1, n - 3), 60) if rng.random() < 0.8 else rng.randint(1, 60)
    elif seed < len(FIXED):
        n, m = FIXED[seed]
    elif seed % 2:
        n = rng.randint(2, 100)
        m = rng.randint(MAX // 2, MAX)
    else:
        n = rng.randint(2, MAX)
        m = rng.randint(n - 1, MAX)
    assert 2 <= n <= MAX and 1 <= m <= MAX
    print(n, m)


if __name__ == "__main__":
    main()
