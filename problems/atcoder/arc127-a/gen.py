"""arc127-a (Leading 1s) の入力を作る。N を 1 行で出す。答えは f(1) + ... + f(N) で、f(x) は x の先頭に並ぶ 1 の数。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、ランダム。
角のケースは、1 桁の N、10^k とその前後、1 が並ぶ N (1 が 15 個の 111111111111111 で f が最大)、
1 が並んだあとに 0 や 2 が来る N、2 × 10^k の前後、9 が並ぶ N、N = 10^15 (16 桁)。
ランダムは、一様なもの、桁数が一様なもの、1 と 0 と 2 が多いもの。1 と 0 と 2 が多いと、先頭の 1 の
並びが長くなり、同じ長さの 11...1 で始まる数と N との大小がきわどくなる。
seed が 1000 以上なら、1 から N まで 1 ずつ数えて先頭の 1 を数える愚直解で解ける小さい N を出す
(pj testdata crosscheck 用)。2000 以上なら、愚直解でまだ解ける 2 × 10^7 までの N を出す。
"""

import random
import sys

MAX = 10**15
ONES = 111111111111111  # 1 が 15 個

FIXED = [
    11,  # 例 1
    120,  # 例 2
    987654321,  # 例 3
    1,
    2,
    9,
    10,
    19,
    100,
    111,
    MAX,  # 16 桁。f(N) = 1
    MAX - 1,  # 9 が 15 個
    ONES,
    ONES - 1,  # 111...10
    ONES + 1,  # 111...12
    ONES - 2,  # 111...109
    11111111111111,  # 1 が 14 個
    2 * 10**14 - 1,  # 1 のあとに 9 が 14 個
    2 * 10**14,
    10**14,
    10**14 + 1,
    110 * 10**12,
    101010101010101,
    123456789012345,
]
RANDOM = 8


def random_case(rng: random.Random, i: int) -> int:
    kind = i % 3
    if kind == 0:
        return rng.randint(1, MAX)
    if kind == 1:
        digits = rng.randint(1, 15)
        return rng.randint(10 ** (digits - 1), 10**digits - 1)
    # 1 と 0 と 2 が多い 15 桁。
    return int("1" + "".join(rng.choice("1111102") for _ in range(14)))


def small_case(rng: random.Random, limit: int) -> int:
    kind = rng.randrange(4)
    if kind == 0:
        return rng.randint(1, min(limit, 1000))
    if kind == 1:
        return rng.randint(1, limit)
    if kind == 2:
        # 10^k や 2 × 10^k の前後。
        k = rng.randint(1, len(str(limit)) - 1)
        return max(1, min(limit, rng.choice([1, 2]) * 10**k + rng.randint(-3, 3)))
    # 1 と 0 と 2 が多い。
    digits = rng.randint(1, len(str(limit)) - 1)
    return max(1, min(limit, int("".join(rng.choice("1111102") for _ in range(digits)).lstrip("0") or "1")))


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 2000:
        n = small_case(rng, 2 * 10**7) if rng.random() < 0.5 else rng.randint(10**5, 2 * 10**7)
    elif seed >= 1000:
        n = small_case(rng, 10**5)
    elif seed < len(FIXED):
        n = FIXED[seed]
    else:
        n = random_case(rng, seed - len(FIXED))
    assert 1 <= n <= MAX
    print(n)


if __name__ == "__main__":
    main()
