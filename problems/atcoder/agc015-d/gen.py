"""agc015-d (A or...or B Problem) の入力を作る。A と B を 1 行ずつ出す。

A と B の共通の上位ビットを除くと、最上位の違うビット T = 2^k について A < T <= B になる。答えは、A 以上 T 未満の数、
T 以上で B 以下の数どうしの OR (T から B の 2 番目に高いビットまでを全部 1 にした数まで)、両方を混ぜた T + A 以上
2T 未満の数、の 3 つの区間の和集合の大きさになる。seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、
2^60 未満のランダム。角のケースは、A = B (答え 1)、B = A + 1、A = 1 と B = 2^60 - 1、2 のべきの前後、B が 2 のべき
ちょうど、上位ビットが長く一致するもの、2 番目と 3 番目の区間がちょうど接するもの、1 つ離れるもの、重なるもの。
seed が 1000 以上なら、A から B までの数を 1 つずつ足して、OR で作れる数の集合を広げていく愚直解で解ける入力を出す
(pj testdata crosscheck 用)。B が 2^11 未満のものと、値は 2^60 近くまであるが B - A が 300 以下のものを半分ずつにする。
"""

import random
import sys

LIM = 2**60
T59 = 2**59
B40 = T59 + 2**40 + 12345  # 2 番目に高いビットが 2^40。T から T + 2^41 - 1 までが OR で作れる

SAMPLES = [(7, 9), (65, 98), (271828182845904523, 314159265358979323)]
FIXED = [
    (1, 1),  # 答え 1
    (LIM - 1, LIM - 1),
    (1, 2),  # 1, 2, 3
    (2, 3),
    (LIM - 2, LIM - 1),
    (1, LIM - 1),  # 全部作れる
    (T59, LIM - 1),
    (T59 - 1, T59),  # T - 1, T, 2T - 1
    (1, T59),
    (T59 - 1, LIM - 1),
    (T59, T59 + 2**58),  # 共通の上位ビットを除くと A が 0 になる
    (2**41, B40),  # 2 番目と 3 番目の区間がちょうど接する
    (2**41 + 1, B40),  # 1 つ離れる
    (2**41 - 1, B40),  # 1 つ重なる
    (T59 + 2**58 + 3, T59 + 2**58 + 5),  # 上位ビットが長く一致する
    (3 * 2**57 + 12345, 3 * 2**57 + 2**20 + 7),
]
RANDOM = 10
COUNT = len(SAMPLES) + len(FIXED) + RANDOM


def random_case(rng: random.Random) -> tuple[int, int]:
    kind = rng.randrange(4)
    if kind == 0:  # 一様
        a, b = sorted(rng.randrange(1, LIM) for _ in range(2))
    elif kind == 1:  # 同じ桁数
        top = 2 ** rng.randint(50, 59)
        a, b = sorted(rng.randrange(top, 2 * top) for _ in range(2))
    elif kind == 2:  # 上位ビットが一致する
        k = rng.randint(1, 58)
        prefix = rng.randrange(1, LIM >> (k + 1)) << (k + 1)
        a, b = prefix + rng.randrange(2**k), prefix + 2**k + rng.randrange(2**k)
    else:  # B が 2 のべきに近い
        b = 2 ** rng.randint(1, 59) + rng.randint(0, 3)
        a = rng.randint(1, b)
    return a, b


def small_case(rng: random.Random) -> tuple[int, int]:
    """愚直解が 1 つずつ足していける大きさ。"""
    if rng.random() < 0.5:
        b = rng.choice([rng.randint(1, 2**11 - 1)] * 2 + [2 ** rng.randint(0, 10), 2 ** rng.randint(1, 11) - 1])
        a = rng.choice([1] + [rng.randint(1, b)] * 3 + [max(1, b - rng.randint(1, 5)), 2 ** rng.randint(0, b.bit_length() - 1)])
        return a, b
    # 値は大きく、B - A は 300 以下。2^k をまたぐものを多めにする。
    k = rng.randint(1, 59)
    prefix = rng.randrange(LIM >> (k + 1)) << (k + 1) if k < 59 else 0
    t = prefix + 2**k
    if rng.random() < 0.7:
        a = max(1, t - rng.randint(1, min(150, 2**k)))
        b = t + rng.randint(0, min(150, 2**k - 1))
    else:
        a = rng.randrange(max(1, prefix), prefix + 2 ** (k + 1))
        b = min(prefix + 2 ** (k + 1) - 1, a + rng.randint(0, 300))
    return a, b


def case_for(seed: int, rng: random.Random) -> tuple[int, int]:
    if seed >= 1000:
        return small_case(rng)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    return random_case(rng)


def main() -> None:
    seed = int(sys.argv[1])
    a, b = case_for(seed, random.Random(seed))
    assert 1 <= a <= b < LIM
    print(a)
    print(b)


if __name__ == "__main__":
    main()
