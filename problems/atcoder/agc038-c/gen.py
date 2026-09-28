"""agc038-c (LCMs) の入力を作る。N と A_0 ... A_{N-1} を出す。答えは lcm(A_i, A_j) (i < j) の和を 998244353 で割った余り。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、N = 2 × 10^5 のケース。
角のケースは、N = 1 (答えは 0)、N = 2、10^6 以下で最大の 2 つの素数 (lcm が 10^12 近くになる)、全部 1、全部 10^6。
大きいケースの値は、一様なランダム、全部 10^6、全部 1、10^6 から下へ N 個の相異なる値、1 から N の相異なる値、
10^6 の近くの素数、720720 の約数 (240 個で、公約数が多い)、2 の冪、少ない種類の値、2 3 5 7 だけを素因数に持つ数、
大きい数 d の倍数。提出は 10^6 までの倍数についてのゼータ変換をするので、時間は値によらない。
seed が 1000 以上なら、組を全部たどって lcm を足す愚直解で解ける小さい入力を出す (pj testdata crosscheck 用)。
2000 以上なら、愚直解でまだ解ける N = 5000 までの入力を出す。
"""

import functools
import random
import sys

MAX_N = 200000
MAX_A = 10**6

SAMPLES = [
    [2, 4, 6],
    [1, 2, 3, 4, 6, 8, 12, 12],
    [356822, 296174, 484500, 710640, 518322, 888250, 259161, 609120, 592348, 713644],
]
FIXED = [
    [1],
    [MAX_A],
    [1, 1],
    [MAX_A, MAX_A],
    [999983, 999979],  # 10^6 以下で最大の 2 つの素数
    [1, MAX_A, 999983, 720720, 524288, 531441, 2, 3],
    list(range(1, 31)),
]
# (N, 値の作り方)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    (MAX_N, "random"),
    (MAX_N, "max"),
    (MAX_N, "ones"),
    (MAX_N, "top"),
    (MAX_N, "range"),
    (MAX_N, "primes"),
    (MAX_N, "divisors"),
    (MAX_N, "pow2"),
    (MAX_N, "few"),
    (MAX_N, "smooth"),
    (MAX_N, "multiples"),
    (1000, "random"),
]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)
HOWS = ["random", "max", "ones", "top", "range", "primes", "divisors", "pow2", "few", "smooth", "multiples"]


@functools.cache
def big_primes() -> list[int]:
    """9 × 10^5 から 10^6 までの素数。"""
    sieve = bytearray([1]) * (MAX_A + 1)
    sieve[0:2] = b"\0\0"
    for p in range(2, int(MAX_A**0.5) + 1):
        if sieve[p]:
            sieve[p * p :: p] = bytearray(len(sieve[p * p :: p]))
    return [p for p in range(900000, MAX_A + 1) if sieve[p]]


def products(primes: list[int]) -> list[int]:
    """primes の冪の積で 10^6 以下のもの。"""
    out = [1]
    for p in primes:
        out = [x * p**e for x in out for e in range(20) if x * p**e <= MAX_A]
    return sorted(out)


DIVISORS = [d for d in products([2, 3, 5, 7, 11, 13]) if 720720 % d == 0]
SMOOTH = products([2, 3, 5, 7])


def values(rng: random.Random, n: int, how: str) -> list[int]:
    if how == "random":
        return [rng.randint(1, MAX_A) for _ in range(n)]
    if how == "max":
        return [MAX_A] * n
    if how == "ones":
        return [1] * n
    if how in ("top", "range"):
        a = list(range(MAX_A - n + 1, MAX_A + 1)) if how == "top" else list(range(1, n + 1))
        rng.shuffle(a)
        return a
    if how == "primes":
        return [rng.choice(big_primes()) for _ in range(n)]
    if how == "divisors":
        return [rng.choice(DIVISORS) for _ in range(n)]
    if how == "pow2":
        return [2 ** rng.randint(0, 19) for _ in range(n)]
    if how == "few":
        pool = [rng.randint(1, MAX_A) for _ in range(rng.randint(2, 50))]
        return [rng.choice(pool) for _ in range(n)]
    if how == "smooth":
        return [rng.choice(SMOOTH) for _ in range(n)]
    assert how == "multiples"
    d = rng.randint(1000, 10000)
    return [d * rng.randint(1, MAX_A // d) for _ in range(n)]


def case_for(seed: int, rng: random.Random) -> list[int]:
    if seed >= 1000:
        # 組を全部たどれる大きさ。
        n = rng.randint(1000, 5000) if seed >= 2000 else rng.randint(1, rng.choice([5, 60]))
        return values(rng, n, rng.choice(HOWS))
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    return values(rng, *PLANS[seed - len(FIXED)])


def main() -> None:
    seed = int(sys.argv[1])
    a = case_for(seed, random.Random(seed))
    assert 1 <= len(a) <= MAX_N and all(1 <= v <= MAX_A for v in a)
    sys.stdout.write(f"{len(a)}\n" + " ".join(map(str, a)) + "\n")


if __name__ == "__main__":
    main()
