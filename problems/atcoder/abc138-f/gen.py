"""abc138-f (Coincidence) の入力を作る。L R を 1 行で出す。
答えは、L ≤ x ≤ y ≤ R で y mod x = y xor x になる組 (x, y) の数を 10^9 + 7 で割った余り。

条件は、x の 1 のビットが y でも 1 で、x と y の最上位のビットが同じことと同じ。
seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、ランダム。角のケースは、
L = R (答えは 1)、L = 1 と R = 10^18、2 の冪の境目をまたぐ区間 (2^k - 1 と 2^k)、最上位のビットが
同じ区間 [2^k, 2^(k+1) - 1]、2^59 から 10^18 まで (10^18 は 60 ビット)、10^18 の近く、1 のビットが
並ぶ L や R。ランダムは、全体から一様なもの、ビット数が一様なもの、L と R の最上位のビットが同じもの。
seed が 1000 以上なら、区間の中の組を全部調べる愚直解で解ける、幅の狭い区間を出す (pj testdata crosscheck 用)。
値は、小さいものと、2 の冪の近くや 10^18 の近くの大きいものを混ぜる。2000 以上なら、幅 10^4 までの区間を出す。
"""

import random
import sys

MAX = 10**18

FIXED = [
    (2, 3),  # 例 1
    (10, 100),  # 例 2
    (1, MAX),  # 例 3
    (1, 1),
    (MAX, MAX),
    (1, 2),
    (1, 3),
    (2**59 - 1, 2**59),  # 最上位のビットが違うので、答えは x = y の 2 組
    (2**59, MAX),
    (1, 2**59 - 1),
    (2**59 - 1, MAX),
    (2**30, 2**31 - 1),
    (2**40 + 1, 2**41 - 2),
    (MAX - 1000, MAX),
    (3, MAX),
    (2**58 - 1, 2**59 + 2**58 - 1),  # 1 のビットが 58 個並ぶ L
    (MAX // 2, MAX),
    (123456789012345678, 987654321098765432),
    (576460752303423487, 576460752303423487),  # L = R = 2^59 - 1
]
RANDOM = 9


def random_case(rng: random.Random, i: int) -> tuple[int, int]:
    kind = i % 3
    if kind == 0:
        l, r = sorted(rng.randint(1, MAX) for _ in range(2))
    elif kind == 1:
        l, r = sorted(rng.randint(1, 2 ** rng.randint(1, 59)) for _ in range(2))
    else:
        # 最上位のビットが同じ 2 つ。
        k = rng.randint(1, 59)
        top = min(MAX, 2 ** (k + 1) - 1)
        l, r = sorted(rng.randint(2**k, top) for _ in range(2))
    return l, r


def small_case(rng: random.Random, width: int) -> tuple[int, int]:
    """幅が width までの区間。"""
    kind = rng.randrange(4)
    if kind == 0:
        l = rng.randint(1, 2 * width)
    elif kind == 1:
        # 2 の冪の境目の近く。
        k = rng.randint(1, 59)
        l = max(1, 2**k - rng.randint(0, width))
    elif kind == 2:
        l = max(1, MAX - rng.randint(0, 2 * width))
    else:
        l = rng.randint(1, MAX)
    r = min(MAX, l + rng.randint(0, width))
    return l, r


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 2000:
        l, r = small_case(rng, 10**4)
    elif seed >= 1000:
        l, r = small_case(rng, rng.choice([10, 100, 1000]))
    elif seed < len(FIXED):
        l, r = FIXED[seed]
    else:
        l, r = random_case(rng, seed - len(FIXED))
    assert 1 <= l <= r <= MAX
    print(l, r)


if __name__ == "__main__":
    main()
