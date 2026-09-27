"""arc066-b (Xor Sum) の入力を作る。N を 1 行で出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、10^18 までのランダム。
提出は下の桁から 60 桁を読むオートマトンで数えるので、2 の冪とその前後、1 が並んだ数、
1 と 0 が交互に並んだ数のような、桁の形が偏った N を入れる。
seed が 1000 以上なら、a + b <= N の組を全部試す愚直解で解ける小さい N を出す (pj testdata crosscheck 用)。
2000 以上なら、愚直解でまだ解ける 10^4 までの N を出す。
"""

import random
import sys

MAX_N = 10**18

SAMPLES = [3, 1422, MAX_N]
FIXED = [
    1,
    2,
    4,
    5,
    7,
    MAX_N - 1,
    2**59 - 1,  # 1 が 59 個並ぶ
    2**59,
    2**59 + 1,
    0xAAAAAAAAAAAAAAA,  # 1010...10 (60 桁)
    0x555555555555555,  # 0101...01
    2**31 - 1,
    2**32,
    10**9 + 7,
]
RANDOM = 6  # 10^18 までのランダム (半分は桁数もランダム)
SMALL = 3  # 3000 までのランダム


def case_for(seed: int, rng: random.Random) -> int:
    if seed >= 1000:
        # 愚直解が a と b を全部試せる大きさ。2 の冪の前後を多めに混ぜる。
        hi = 10**4 if seed >= 2000 else 300
        if rng.random() < 0.3:
            k = rng.randint(0, hi.bit_length() - 1)
            return max(1, min(hi, 2**k + rng.randint(-2, 2)))
        return rng.randint(1, hi)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    seed -= len(FIXED)
    if seed < RANDOM:
        if seed % 2 == 0:
            return rng.randint(1, MAX_N)
        return rng.randint(1, 2 ** rng.randint(1, 59))
    return rng.randint(1, 3000)


def main() -> None:
    seed = int(sys.argv[1])
    n = case_for(seed, random.Random(seed))
    assert 1 <= n <= MAX_N
    print(n)


if __name__ == "__main__":
    main()
