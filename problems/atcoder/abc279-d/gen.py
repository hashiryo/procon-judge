"""abc279-d (Freefall) の入力を作る。A B を 1 行で出す。答えは整数 n >= 0 での B n + A / √(n + 1) の最小。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、10^18 までのランダム。
実数での最小は n + 1 = (A / 2B)^(2/3) で、角のケースには、そこがちょうど整数になるもの、n = 0 が最小に
なるもの (B が A の 0.29 倍より大きい)、n = 0 と n = 1 の境目の近く、A = 10^18 と B = 1 で n が
6 * 10^11 あたりになるものなどを入れる。ランダムは、A / B を 10^0 から 10^18 まで散らす。
seed が 1000 以上なら、愚直解ですぐ解ける入力を出す (pj testdata crosscheck 用)。多くは n を 0 から
1 つずつ試せる A / B が 10^7 までのもので、残りは愚直解が実数の最小の前後を試す A / B の大きいもの。
愚直解はどの入力も数秒で解けるので、本番のケースもそのまま突き合わせられる。
"""

import random
import sys

MAX = 10**18

FIXED = [
    (10, 1),  # 例 1
    (5, 10),  # 例 2
    (10**18, 100),  # 例 3
    (1, 1),
    (MAX, MAX),
    (1, MAX),
    (MAX, 1),
    (MAX, 2),
    (16, 1),  # 実数の最小がちょうど n = 3
    (2 * 700000**3, 1),  # 実数の最小がちょうど n = 7 * 10^5 の 2 乗 - 1
    (2 * 1000**3 * 12345, 12345),  # 同じく n = 10^6 - 1
    (7, 2),  # n = 1 がわずかに良い
    (7, 3),  # n = 0 が良い
    (2, 1),
    (MAX, MAX - 1),
    (MAX - 1, 3),
]
RANDOM = 10


def random_case(rng: random.Random, kind: int) -> tuple[int, int]:
    if kind == 0:
        return rng.randint(1, MAX), rng.randint(1, MAX)
    if kind == 1:
        return rng.randint(MAX // 10, MAX), rng.randint(1, 1000)
    # A / B を 10^e のあたりにする。
    e = rng.randint(0, 18)
    b = rng.randint(1, MAX // 10**e)
    a = min(MAX, b * 10**e + rng.randint(0, 10**e))
    return a, b


def small_case(rng: random.Random) -> tuple[int, int]:
    """A / B が 10^7 まで (B は小さい値から 10^11 まで散らす) か、A / B が大きいもの。"""
    if rng.random() < 0.2:
        return random_case(rng, rng.choice([1, 2]))
    ratio = rng.choice([rng.randint(0, 10), rng.randint(0, 1000), rng.randint(0, 10**7)])
    b = rng.choice([rng.randint(1, 10), rng.randint(1, 10**6), rng.randint(1, 10**11)])
    a = max(1, min(MAX, b * ratio + rng.randint(0, b)))
    return a, b


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 1000:
        a, b = small_case(rng)
    elif seed < len(FIXED):
        a, b = FIXED[seed]
    else:
        a, b = random_case(rng, (seed - len(FIXED)) % 4)
    assert 1 <= a <= MAX and 1 <= b <= MAX
    print(a, b)


if __name__ == "__main__":
    main()
