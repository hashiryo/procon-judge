"""abc121-d (XOR World) の入力を作る。A B を 1 行で出す。答えは A から B までの排他的論理和。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、10^12 までのランダム。
角のケースは、A = 0、A = B、最上位のビット (2^39) をまたぐ区間、答えが 0 になる区間
(0 から、または 4 の倍数から 4 の倍数 - 1 まで) など。
seed が 1000 以上なら、A から B まで 1 つずつ排他的論理和を取る愚直解で解ける、幅の狭い区間を出す
(pj testdata crosscheck 用)。
"""

import random
import sys

MAX = 10**12
TOP = 2**39  # MAX 以下で最大の 2 のべき

FIXED = [
    (2, 4),  # 例 1
    (123, 456),  # 例 2
    (123456789012, 123456789012),  # 例 3
    (0, 0),
    (0, 1),
    (1, 1),
    (0, 3),  # 答えは 0
    (1, 4),
    (0, MAX),
    (1, MAX),
    (MAX, MAX),
    (MAX - 1, MAX),
    (MAX - 4, MAX - 1),  # 4 の倍数から 4 個なので 0
    (TOP - 1, TOP),  # 最上位のビットをまたぐ。答えは 2^40 - 1
    (0, TOP - 1),  # 答えは 0
    (TOP, MAX),
    (TOP - 2, TOP + 1),
]
RANDOM = 8


def random_case(rng: random.Random, kind: int) -> tuple[int, int]:
    if kind == 0:
        a, b = sorted(rng.randint(0, MAX) for _ in range(2))
    elif kind == 1:
        a, b = 0, rng.randint(0, MAX)
    elif kind == 2:
        a = rng.randint(0, MAX)
        b = min(MAX, a + rng.randint(0, 10))
    elif kind == 3:
        a, b = rng.randint(0, MAX), MAX
    else:
        # A と B の上位のビットが揃っている区間。
        a, b = sorted(rng.randint(TOP, MAX) for _ in range(2))
    return a, b


def small_case(rng: random.Random) -> tuple[int, int]:
    """愚直解が B - A + 1 個を順に xor できる幅。A は 0 の近く、2 のべきの近く、10^12 の近くを混ぜる。"""
    width = rng.choice([rng.randint(0, 8), rng.randint(0, 1000), rng.randint(0, 10**6)])
    base = rng.choice([
        rng.randint(0, 20),
        2 ** rng.randint(1, 39) - rng.randint(0, 5),
        rng.randint(0, MAX),
        MAX - rng.randint(0, 20),
    ])
    a = max(0, min(base, MAX - width))
    return a, a + width


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 1000:
        a, b = small_case(rng)
    elif seed < len(FIXED):
        a, b = FIXED[seed]
    else:
        a, b = random_case(rng, (seed - len(FIXED)) % 5)
    assert 0 <= a <= b <= MAX
    print(a, b)


if __name__ == "__main__":
    main()
