"""abc129-e (Sum Equals Xor) の入力を作る。L を 2 進数で 1 行に出す。

seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース、10 万桁あたりのランダム。
角のケースは、L = 1、全部 1 (答えは 3^桁数)、1 のあとに 0 だけ (答えは 3^(桁数 - 1) + 2)、
1 と 0 が交互、1 がまばらなものや詰まったものなど。桁数は 100001 まで (L < 2^100001)。
seed が 1000 以上なら、a + b <= L の組を全部調べる愚直解で解ける小さい L を出す
(pj testdata crosscheck 用)。
"""

import random
import sys

MAX_LEN = 100001

FIXED = [
    "10",  # 例 1
    "1111111111111111111",  # 例 2
    "1",
    "11",
    "100",
    "101",
    "1" * MAX_LEN,
    "1" + "0" * (MAX_LEN - 1),
    "1" + "0" * (MAX_LEN - 2) + "1",
    "1" * (MAX_LEN - 1) + "0",
    "10" * (MAX_LEN // 2) + "1",
    "1" + "0" * (MAX_LEN // 2) + "1" * (MAX_LEN // 2),
    "1" * (MAX_LEN // 2) + "0" * (MAX_LEN - MAX_LEN // 2),
]
# (桁数, 1 の割合)。本番のケースのうち角のケースのあとに並べる。
RANDOM = [
    (MAX_LEN, 0.5),
    (MAX_LEN, 0.01),
    (MAX_LEN, 0.99),
    (MAX_LEN - 1, 0.5),
    (12345, 0.5),
    (1000, 0.2),
    (1000, 0.8),
    (64, 0.5),
    (16, 0.5),
    (12, 0.5),
]
COUNT = len(FIXED) + len(RANDOM)


def random_bits(rng: random.Random, length: int, ones: float) -> str:
    """先頭が 1 の、length 桁の 2 進数。"""
    return "1" + "".join("1" if rng.random() < ones else "0" for _ in range(length - 1))


def case_for(seed: int, rng: random.Random) -> str:
    if seed >= 1000:
        # 愚直解が a + b <= L の組を全部調べられる大きさ (12 桁まで)。
        length = rng.randint(1, 12)
        kind = rng.randrange(4)
        if kind == 0:
            return "1" * length
        if kind == 1:
            return "1" + "0" * (length - 1)
        return random_bits(rng, length, rng.choice([0.2, 0.5, 0.8]))
    if seed < len(FIXED):
        return FIXED[seed]
    length, ones = RANDOM[seed - len(FIXED)]
    return random_bits(rng, length, ones)


def main() -> None:
    seed = int(sys.argv[1])
    bits = case_for(seed, random.Random(seed))
    assert 1 <= len(bits) <= MAX_LEN and bits[0] == "1" and set(bits) <= set("01")
    print(bits)


if __name__ == "__main__":
    main()
