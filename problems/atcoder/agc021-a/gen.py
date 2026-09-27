"""agc021-a (Digit Sum 2) の入力を作る。N を 1 行で出す。答えは N 以下の正の整数の桁和の最大。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、ランダム。
答えは N そのものか、N のある桁を 1 減らして下の桁を 9 で埋めたものの桁和になる。角のケースは、
1 桁の N、10^k とその前後、9 が並ぶ N、先頭の桁だけ小さい N、N = 10^16 (17 桁) と、
両方の候補が同じ桁和になる N。ランダムは、一様なもの、桁数が一様なもの、9 と 8 と 0 が多いもの。
seed が 1000 以上なら、1 から N まで桁和を数える愚直解で解ける小さい N を出す (pj testdata crosscheck 用)。
2000 以上なら、愚直解でまだ解ける 2 × 10^7 までの N を出す。
"""

import random
import sys

MAX = 10**16

FIXED = [
    100,  # 例 1
    9995,  # 例 2
    3141592653589793,  # 例 3
    1,
    9,
    10,
    19,  # N そのもの (10) が最大
    99,
    MAX,  # 17 桁。答えは 9 が 16 個
    MAX - 1,
    MAX - 2,  # N そのものと 8999...9 が同じ桁和
    9 * 10**15,
    2 * 10**15 - 1,  # 1 のあとに 9 が 15 個。N そのものが最大
    2 * 10**15,
    10**15,
    10**15 + 1,
    11 * 10**14 - 1,  # 1099...9。先頭の 1 を 0 にしたほうが大きい
    98 * 10**14 - 1,  # 9799...9
    99 * 10**14 - 1,  # 9899...9。N そのものと 8999...9 が同じ桁和
    1234567890123456,
    5 * 10**15,
]
RANDOM = 9


def random_case(rng: random.Random, i: int) -> int:
    kind = i % 3
    if kind == 0:
        return rng.randint(1, MAX)
    if kind == 1:
        digits = rng.randint(1, 16)
        return rng.randint(10 ** (digits - 1), 10**digits - 1)
    # 9 と 8 と 0 が多い 16 桁。どの桁を 1 減らすかで同点が出やすい。
    return int("".join(rng.choice("9998810") for _ in range(16)).lstrip("0") or "1")


def small_case(rng: random.Random, limit: int) -> int:
    kind = rng.randrange(4)
    if kind == 0:
        return rng.randint(1, min(limit, 1000))
    if kind == 1:
        return rng.randint(1, limit)
    if kind == 2:
        # 10^k の前後。
        k = rng.randint(1, len(str(limit)) - 1)
        return max(1, min(limit, 10**k + rng.randint(-3, 3)))
    # 9 と 8 と 0 が多い。
    digits = rng.randint(1, len(str(limit)) - 1)
    return max(1, min(limit, int("".join(rng.choice("998810") for _ in range(digits)).lstrip("0") or "1")))


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
