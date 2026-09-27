"""abc335-g (Discrete Logarithm Problems) の入力を作る。N P と A を出す。答えは A_i^k ≡ A_j となる k がある組 (i, j) の数。

A_j が A_i のべきになるのは、A_j の位数が A_i の位数を割り切るときである。
seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、N が 5 × 10^4 か 2 × 10^5 のケース。
P は、2 と 3、10^13 以下で最大の素数、2^30 の前後 (提出は 2^30 を境に 32 ビットと 64 ビットの剰余の計算を
使い分ける)、P - 1 の約数が最も多いもの (10080 個)、P - 1 = 2q の安全素数、P - 1 が 2 × 10^6 前後の素数を
2 つ持つもの、P - 1 = 3 × 2^41 のもの、約数の多い小さい素数 (55441 と 2521) を使う。
A は、一様なランダム、原始根だけ、位数の小さい元 (P - 1 の約数 d を選んで g^((P - 1) / d × r))、
少ない種類の値の繰り返し、全部同じ値、全部 1、全部 P - 1 にする。
値が 10^13 近くのケースは 1 行が長いので、N = 2 × 10^5 のものは 8 つにして、残りは N = 5 × 10^4 にする。
2^30 の少し下の P では、32 ビットの Montgomery 表現が P 以上のまま残りやすく、1 と比べるところで間違えやすい。
そこで、P = 1073741789 で位数が (P - 1) / 4 と (P - 1) / 2 の 2 つだけの入力 (答えは 3) も入れる。
seed が 1000 以上なら、A_i のべきを 1 周するまで掛けていく愚直解で解ける P <= 3000 の入力を出す
(pj testdata crosscheck 用)。2000 以上なら、P を 10^5 まで、N を 1000 までにした入力を出す。
"""

import math
import random
import sys

MAX_N = 2 * 10**5
MAX_P = 10**13

LARGEST = 9999999999971  # 10^13 以下で最大の素数。P - 1 = 2 × 5 × 5507 × 181587071
BELOW_2_30 = 1073741789  # 2^30 より小さい最大の素数
ABOVE_2_30 = 1073741827  # 2^30 より大きい最小の素数
MANY_DIVISORS = 6746328388801  # P - 1 = 2^6 3^4 5^2 7^2 11 13 17 19 23 (約数 10080 個)
SAFE = 9999999999659  # P - 1 = 2 × 4999999999829
TWO_LARGE = 8312037969239  # P - 1 = 2 × 2027537 × 2049787
POW2 = 6597069766657  # P - 1 = 3 × 2^41

SAMPLES = [
    (13, [2, 3, 5]),
    (2, [1, 1, 1, 1, 1]),
    (LARGEST, [
        141592653589, 793238462643, 383279502884, 197169399375, 105820974944,
        592307816406, 286208998628, 34825342117, 67982148086, 513282306647,
    ]),
]
# 例のあとに並べる、値をそのまま決めたケース。
FIXED = [
    (BELOW_2_30, [278009743, 1036660031]),  # 位数は (P - 1) / 4 と (P - 1) / 2。答えは 3
]
# (P, N, A の出し方)。本番のケースのうち FIXED のあとに並べる。
PLANS = [
    (7, 6, "all"),  # 1 から 6 を 1 つずつ
    (LARGEST, 2, "one_minus"),  # 1 と P - 1。答えは 3
    (2521, 1000, "random"),
    (2, MAX_N, "one"),  # 答えは 4 × 10^10
    (3, MAX_N, "random"),
    (55441, MAX_N, "random"),
    (LARGEST, MAX_N, "random"),
    (LARGEST, MAX_N, "primitive"),  # 答えは N^2
    (BELOW_2_30, MAX_N, "random"),
    (ABOVE_2_30, MAX_N, "small_order"),
    (MANY_DIVISORS, MAX_N, "small_order"),
    (MANY_DIVISORS, MAX_N, "random"),
    (MANY_DIVISORS, MAX_N, "pool"),
    (TWO_LARGE, MAX_N, "small_order"),
    (SAFE, 50000, "random"),
    (POW2, 50000, "small_order"),
    (LARGEST, 50000, "same"),
    (SAFE, 50000, "minus"),
]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)
HOWS = ["random", "primitive", "small_order", "pool", "same", "one", "minus"]


def is_prime(n: int) -> bool:
    if n < 2:
        return False
    bases = (2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37)
    for p in bases:
        if n % p == 0:
            return n == p
    d, s = n - 1, 0
    while d % 2 == 0:
        d, s = d // 2, s + 1
    for a in bases:
        x = pow(a, d, n)
        if x in (1, n - 1):
            continue
        for _ in range(s - 1):
            x = x * x % n
            if x == n - 1:
                break
        else:
            return False
    return True


def factorize(n: int) -> dict[int, int]:
    """割り算を順に試して素因数分解する。n <= 10^13 なら 3 × 10^6 までで済む。"""
    f: dict[int, int] = {}
    d = 2
    while d * d <= n:
        while n % d == 0:
            f[d] = f.get(d, 0) + 1
            n //= d
        d += 1 if d == 2 else 2
    if n > 1:
        f[n] = f.get(n, 0) + 1
    return f


def divisors(f: dict[int, int]) -> list[int]:
    ds = [1]
    for q, e in f.items():
        ds = [d * q**k for d in ds for k in range(e + 1)]
    return sorted(ds)


def primitive_root(p: int, f: dict[int, int]) -> int:
    if p == 2:
        return 1
    return next(g for g in range(2, p) if all(pow(g, (p - 1) // q, p) != 1 for q in f))


def pick_a(rng: random.Random, p: int, n: int, how: str) -> list[int]:
    if how == "random":
        return [rng.randint(1, p - 1) for _ in range(n)]
    if how == "one":
        return [1] * n
    if how == "minus":
        return [p - 1] * n
    if how == "same":
        return [rng.randint(1, p - 1)] * n
    if how == "all":
        return list(range(1, p))
    if how == "one_minus":
        return [1, p - 1]
    f = factorize(p - 1)
    g = primitive_root(p, f)
    if how == "primitive":
        exps = [rng.randrange(p - 1) for _ in range(n)]
        return [pow(g, e if math.gcd(e, p - 1) == 1 else 1, p) for e in exps]
    ds = divisors(f)

    def small_order() -> int:
        d = rng.choice(ds)
        return pow(g, (p - 1) // d * rng.randrange(d), p)

    if how == "small_order":
        return [small_order() for _ in range(n)]
    assert how == "pool"
    pool = [small_order() for _ in range(20)] + [rng.randint(1, p - 1) for _ in range(10)] + [1, p - 1]
    return [rng.choice(pool) for _ in range(n)]


def random_prime(rng: random.Random, lo: int, hi: int) -> int:
    while True:
        p = rng.randint(lo, hi)
        if is_prime(p):
            return p


def case_for(seed: int, rng: random.Random) -> tuple[int, list[int]]:
    if seed >= 1000:
        # 愚直解が値ごとに部分群を 1 周できる大きさ。約数の多い P と、2 と 3 を多めにする。
        big = seed >= 2000
        p = rng.choice(
            [55441, 45361, 30241, random_prime(rng, 2, 10**5)] if big
            else [2, 3, 2521, 2161, 1801, random_prime(rng, 2, 50), random_prime(rng, 2, 3000)]
        )
        n = rng.randint(2, 1000 if big else 60)
        return p, pick_a(rng, p, n, rng.choice(HOWS))
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    p, n, how = PLANS[seed - len(FIXED)]
    return p, pick_a(rng, p, n, how)


def main() -> None:
    seed = int(sys.argv[1])
    p, a = case_for(seed, random.Random(seed))
    n = len(a)
    assert 2 <= n <= MAX_N and 2 <= p <= MAX_P and is_prime(p) and all(1 <= v < p for v in a)
    sys.stdout.write(f"{n} {p}\n" + " ".join(map(str, a)) + "\n")


if __name__ == "__main__":
    main()
