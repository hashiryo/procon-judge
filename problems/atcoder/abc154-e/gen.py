"""abc154-e (Almost Everywhere Zero) の入力を作る。N と K を 1 行ずつ出す。

seed が 0 から count - 1 までは本番のケース。例の 4 つ、角のケース、N が 100 桁のランダム。
角のケースは、N = 1、K が N の桁数より大きい (答えが 0)、N = 10^99、N = 10^100 - 1、
N 自身がちょうど K 個の 0 でない桁を持つもの (N を数え忘れやすい)、0 が多い N など。
答えの最大は N = 10^100 - 1、K = 3 のときの 117879300 (例 4)。
seed が 1000 以上なら、1 から N まで数えて桁を調べる愚直解で解ける N ≤ 10^6 の入力を出す
(pj testdata crosscheck 用)。2000 以上なら、愚直解でまだ解ける N ≤ 2 × 10^7 の入力を出す。
"""

import random
import sys

MAX_DIGITS = 100

SAMPLES = [
    ("100", 1),
    ("25", 2),
    ("314159", 2),
    ("9" * 100, 3),
]
FIXED = [
    ("1", 1),
    ("1", 2),  # 0 でない桁が 2 個の数は無い
    ("9", 1),
    ("11", 2),  # N 自身だけ
    ("99", 3),  # K が桁数より大きい
    ("100", 3),
    ("111", 3),
    ("1" + "0" * 99, 1),  # 10^99
    ("1" + "0" * 99, 2),
    ("1" + "0" * 99, 3),
    ("9" * 100, 1),
    ("9" * 100, 2),
    ("9" * 99, 3),
    ("1" + "0" * 98 + "1", 2),  # N 自身がちょうど 2 個
    ("2" + "0" * 49 + "5" + "0" * 48 + "9", 3),  # N 自身がちょうど 3 個
    ("2" + "0" * 49 + "5" + "0" * 48 + "9", 2),
    ("1" + "0" * 97 + "10", 3),  # 0 でない桁が 2 個しかない N
    ("5" * 100, 3),
    ("10" * 50, 3),
    ("1" * 100, 2),
    ("18446744073709551616", 3),  # 2^64
]
RANDOM_FULL = 5  # 100 桁のランダム
RANDOM_LENGTH = 3  # 桁数もランダム
COUNT = len(SAMPLES) + len(FIXED) + RANDOM_FULL + RANDOM_LENGTH


def random_number(rng: random.Random, digits: int) -> str:
    """digits 桁のランダムな数。0 の多いものも混ぜる。"""
    zero = rng.choice([0.1, 0.5, 0.9])
    rest = "".join("0" if rng.random() < zero else str(rng.randint(1, 9)) for _ in range(digits - 1))
    return str(rng.randint(1, 9)) + rest


def small_number(rng: random.Random, limit: int) -> str:
    """1 以上 limit 以下の数。10^k、10^k - 1、10^k + 1、0 の多い数を多めに混ぜる。"""
    kind = rng.randrange(5)
    if kind == 0:
        return str(rng.randint(1, limit))
    k = rng.randint(0, len(str(limit)) - 1)
    if kind == 1:
        return str(10**k)
    if kind == 2:
        return str(max(1, 10**k - 1))
    if kind == 3:
        return str(min(limit, 10**k + 1))
    return random_number(rng, rng.randint(1, len(str(limit)) - 1))


def case_for(seed: int, rng: random.Random) -> tuple[str, int]:
    if seed >= 1000:
        limit = 2 * 10**7 if seed >= 2000 else 10**6
        return small_number(rng, limit), rng.randint(1, 3)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    seed -= len(FIXED)
    digits = MAX_DIGITS if seed < RANDOM_FULL else rng.randint(1, MAX_DIGITS)
    return random_number(rng, digits), rng.randint(1, 3)


def main() -> None:
    seed = int(sys.argv[1])
    n, k = case_for(seed, random.Random(seed))
    assert n.isdigit() and n[0] != "0" and len(n) <= MAX_DIGITS and 1 <= k <= 3
    print(n)
    print(k)


if __name__ == "__main__":
    main()
