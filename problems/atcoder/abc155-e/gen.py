"""abc155-e (Payment) の入力を作る。N を 1 行で出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、ランダム (半分以上は 10^6 桁)。
角のケースは、1 桁の数、最大の N = 10^(10^6)、全部 9、全部 5 など。5 は払うかおつりにするかで
枚数が同じになる境目で、9 と 0 の並びは繰り上がりが長く続く。ランダムは、数字をランダムに
選んだもの、4 5 6 だけ、0 と 9 だけ、同じ数字が長く続くものを混ぜる。
seed が 1000 以上なら、払う額を全部試す愚直解で解ける 5 桁までの N を出す (pj testdata crosscheck 用)。
"""

import random
import sys

MAX_DIGITS = 10**6  # N < 10^(10^6) なら 10^6 桁まで。N = 10^(10^6) だけが 10^6 + 1 桁

SAMPLES = [
    "36",
    "91",
    "314159265358979323846264338327950288419716939937551058209749445923078164062862089986280348253421170",
]
FIXED = [
    "1",
    "5",
    "6",
    "9",
    "10",
    "55",
    "95",
    "4" * 1000,
    "6" * 1000,
    "5" + "4" * 999,
    "1" + "9" * 998 + "1",
    "1" + "0" * MAX_DIGITS,  # 最大の N。答えは 1
    "9" * MAX_DIGITS,  # 10^(10^6) - 1。答えは 2
    "5" * MAX_DIGITS,
    "9" * (MAX_DIGITS // 2) + "0" * (MAX_DIGITS // 2),
]
# ランダム。(桁数, 使う数字, 同じ数字が続く長さの上限)。
RANDOM = [
    (1000, "0123456789", 1),
    (10**5, "45", 1),
    (10**5, "5", 1),  # 下の random_digits で 5 のあいだに 4 と 6 を少し混ぜる
    (MAX_DIGITS, "0123456789", 1),
    (MAX_DIGITS, "0123456789", 1),
    (MAX_DIGITS, "456", 1),
    (MAX_DIGITS, "09", 1),
    (MAX_DIGITS, "0123456789", 50),
    (MAX_DIGITS, "059", 1000),
]
COUNT = len(SAMPLES) + len(FIXED) + len(RANDOM)


def random_digits(rng: random.Random, digits: str, run: int, length: int) -> str:
    out: list[str] = []
    while len(out) < length:
        c = rng.choice(digits)
        if digits == "5" and rng.random() < 0.01:
            c = rng.choice("46")
        out.extend(c * rng.randint(1, run))
    s = "".join(out[:length])
    if s[0] == "0":
        s = rng.choice("123456789") + s[1:]
    return s


def small_case(rng: random.Random) -> str:
    """愚直解が払う額を全部試せる大きさ (5 桁まで)。境目の数字を多めに混ぜる。"""
    kind = rng.randrange(3)
    if kind == 0:
        return str(rng.randint(1, 99999))
    if kind == 1:
        return str(rng.randint(1, 200))
    return random_digits(rng, rng.choice(["04569", "59", "09", "456"]), rng.randint(1, 3), rng.randint(1, 5))


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 1000:
        n = small_case(rng)
    elif seed < len(SAMPLES):
        n = SAMPLES[seed]
    elif seed < len(SAMPLES) + len(FIXED):
        n = FIXED[seed - len(SAMPLES)]
    else:
        length, digits, run = RANDOM[seed - len(SAMPLES) - len(FIXED)]
        n = random_digits(rng, digits, run, length)
    assert n.isdigit() and n[0] != "0"
    assert len(n) <= MAX_DIGITS or n == "1" + "0" * MAX_DIGITS
    print(n)


if __name__ == "__main__":
    main()
