"""abc235-f (Variety of Digits) の入力を作る。N と M と C_1 ... C_M を出す。N は 10^4 桁まで。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、10^4 桁の N。
角のケースは、1 桁の N、C が 0 だけのもの (先頭の 0 は数えない)、10 個の数字を全部含む最小の数
1023456789 とその 1 つ手前 (答えが 0)、9 が並ぶ N、10^9999、同じ数字が並ぶ N。
10^4 桁の N は、ランダムな数字のものと、9876543210 のくり返しで、M は 1 から 10 まで散らす。
seed が 1000 以上なら、1 から N まで全部調べる愚直解で解ける 10^5 までの N を出す (pj testdata crosscheck 用)。
2000 以上なら、愚直解でまだ解ける 10^7 までの N を出す。
"""

import random
import sys

MAX_DIGITS = 10**4
ALL = list(range(10))

FIXED = [
    ("104", [0, 1]),  # 例 1
    ("999", [1, 2, 3, 4]),  # 例 2
    ("1234567890" * 10, [0, 2, 4, 6, 8]),  # 例 3
    ("1", [1]),
    ("1", [0]),  # 答えは 0
    ("9", [9]),
    ("10", [0]),
    ("10", [0, 1]),
    ("1023456788", ALL),  # 答えは 0
    ("1023456789", ALL),  # 答えは N
    ("9" * MAX_DIGITS, ALL),
    ("9" * MAX_DIGITS, [0]),
    ("9" * MAX_DIGITS, [9]),
    ("1" + "0" * (MAX_DIGITS - 1), [0]),
    ("1" + "0" * (MAX_DIGITS - 1), ALL),
    ("1" * MAX_DIGITS, [0, 1]),
    ("5" * MAX_DIGITS, [4, 6]),
    ("9876543210" * (MAX_DIGITS // 10), ALL),
]
# 10^4 桁のランダムな N に付ける M。
RANDOM_M = [1, 2, 3, 5, 7, 9, 10]
RANDOM_SHORT = 3  # 桁数もランダム


def pick_c(rng: random.Random, m: int) -> list[int]:
    return sorted(rng.sample(ALL, m))


def random_number(rng: random.Random, digits: int) -> str:
    return str(rng.randint(1, 9)) + "".join(rng.choice("0123456789") for _ in range(digits - 1))


def small_case(rng: random.Random, limit: int) -> tuple[str, list[int]]:
    kind = rng.randrange(4)
    if kind == 0:
        n = rng.randint(1, limit)
    elif kind == 1:
        n = int(10 ** rng.uniform(0, len(str(limit)) - 1))
    elif kind == 2:
        # 10^k の前後と、同じ数字の並び。
        k = rng.randint(1, len(str(limit)) - 1)
        n = rng.choice([10**k - 1, 10**k, 10**k + 1, int(str(rng.randint(1, 9)) * k)])
    else:
        n = rng.randint(limit // 10, limit)
    m = rng.choice([1, 1, 2, 2, 3, 3, 4, 5, 6, 10])
    return str(max(1, min(n, limit))), pick_c(rng, m)


def case_for(seed: int, rng: random.Random) -> tuple[str, list[int]]:
    if seed >= 2000:
        return small_case(rng, 10**7)
    if seed >= 1000:
        return small_case(rng, 10**5)
    if seed < len(FIXED):
        return FIXED[seed]
    seed -= len(FIXED)
    if seed < len(RANDOM_M):
        return random_number(rng, MAX_DIGITS), pick_c(rng, RANDOM_M[seed])
    return random_number(rng, rng.randint(1, MAX_DIGITS)), pick_c(rng, rng.randint(1, 10))


def main() -> None:
    seed = int(sys.argv[1])
    n, c = case_for(seed, random.Random(seed))
    assert 1 <= len(n) <= MAX_DIGITS and n.isdigit() and n[0] != "0"
    assert 1 <= len(c) <= 10 and c == sorted(set(c)) and all(0 <= x <= 9 for x in c)
    print(n)
    print(len(c))
    print(*c)


if __name__ == "__main__":
    main()
