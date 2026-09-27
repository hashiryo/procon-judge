"""abc141-e (Who Says a Pun?) の入力を作る。N と、英小文字の文字列 S を出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、N = 5000 のいろいろな形の文字列。
角のケースは、N = 2 と 3、26 文字が全部違う (答え 0)、全部 a (答えは N / 2 の切り捨てで、重なりの条件だけで決まる)。
N = 5000 は、X X や X Y X の形、周期 2、3、7、1667 のくり返し、a の並びの後に b の並び、ランダム (26 文字、4 文字、
2 文字)、1 文字だけ変えた X X'、Thue-Morse 列 (2^64 で割る rolling hash が衝突する列)、フィボナッチ文字列。
全部 a は、lib-rh.cpp の multiset の count が毎回全部を数えるので遅くなりやすい。
seed が 1000 以上なら、始まりの組を全部比べる愚直解で解ける短い文字列を出す (pj testdata crosscheck 用)。
"""

import random
import string
import sys

MAX_N = 5000
LETTERS = string.ascii_lowercase

SAMPLES = ["ababa", "xy", "strangeorange"]


def rand(rng: random.Random, n: int, k: int = 26) -> str:
    return "".join(rng.choice(LETTERS[:k]) for _ in range(n))


def thue_morse(n: int) -> str:
    return "".join("ab"[bin(i).count("1") % 2] for i in range(n))


def fibonacci(n: int) -> str:
    a, b = "a", "ab"
    while len(b) < n:
        a, b = b, b + a
    return b[:n]


def periodic(rng: random.Random, n: int, period: int, k: int = 26) -> str:
    block = rand(rng, period, k)
    return (block * (n // period + 1))[:n]


def xyx(rng: random.Random, n: int, x: int, k: int = 26) -> str:
    """X Y X の形。|X| = x で、Y は残り。"""
    head = rand(rng, x, k)
    return head + rand(rng, n - 2 * x, k) + head


def mutated_double(rng: random.Random, n: int, k: int = 26) -> str:
    """X X' の形。X' は X の真ん中あたりの 1 文字を変えたもの。"""
    half = rand(rng, n // 2, k)
    i = rng.randrange(len(half) // 4, 3 * len(half) // 4)
    other = rng.choice([c for c in LETTERS[:max(k, 2)] if c != half[i]])
    return (half + half[:i] + other + half[i + 1:] + rand(rng, n % 2, k))[:n]


FIXED = [
    *SAMPLES,
    "aa",
    "aaa",
    "aab",
    LETTERS,  # 26 文字が全部違うので 0
    "a" * MAX_N,
    "a" * (MAX_N - 1),
]
# 本番のケースのうち FIXED のあとに並べる、N = 5000 の作り方。
PLANS = [
    lambda rng: xyx(rng, MAX_N, MAX_N // 2),  # X X
    lambda rng: xyx(rng, MAX_N, 2000),
    lambda rng: xyx(rng, MAX_N, 1234, 2),
    lambda rng: periodic(rng, MAX_N, 2),
    lambda rng: periodic(rng, MAX_N, 3),
    lambda rng: periodic(rng, MAX_N, 7),
    lambda rng: periodic(rng, MAX_N, 1667),
    lambda rng: "a" * 2500 + "b" * 2500,
    lambda rng: "a" * 3001 + rand(rng, MAX_N - 3001),
    lambda rng: rand(rng, MAX_N),
    lambda rng: rand(rng, MAX_N, 4),
    lambda rng: rand(rng, MAX_N, 2),
    lambda rng: mutated_double(rng, MAX_N),
    lambda rng: mutated_double(rng, MAX_N, 2),
    lambda rng: thue_morse(MAX_N),
    lambda rng: fibonacci(MAX_N),
    lambda rng: "".join(c * rng.randint(1, 30) for c in rand(rng, MAX_N, 3))[:MAX_N],
]
COUNT = len(FIXED) + len(PLANS)


def small_case(rng: random.Random) -> str:
    """愚直解が始まりの組を全部比べても速い長さ。形は本番のケースと同じものから選ぶ。"""
    n = rng.randint(2, 120)
    k = rng.choice([1, 2, 2, 2, 3, 4, 5, 26, 26])
    kind = rng.randrange(7)
    if kind == 0:
        return rand(rng, n, k)
    if kind == 1:
        return periodic(rng, n, rng.randint(1, max(1, n // 2)), k)
    if kind == 2:
        return xyx(rng, n, rng.randint(0, n // 2), k)
    if kind == 3:
        return mutated_double(rng, max(n, 8), k)
    if kind == 4:
        start = rng.randrange(1000)
        return thue_morse(start + n)[start:]
    if kind == 5:
        return fibonacci(n + 50)[50:]
    return "".join(c * rng.randint(1, 8) for c in rand(rng, n, k))[:n]


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 1000:
        s = small_case(rng)
    elif seed < len(FIXED):
        s = FIXED[seed]
    else:
        s = PLANS[seed - len(FIXED)](rng)
    assert 2 <= len(s) <= MAX_N and set(s) <= set(LETTERS)
    print(len(s))
    print(s)


if __name__ == "__main__":
    main()
