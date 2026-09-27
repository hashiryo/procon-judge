"""abc212-g (Power Pair) の入力を作る。素数 P を 1 行に出す。答えは 1 + (P - 1 の約数 d についての φ(d) d の和)。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、10^12 以下のランダムな素数。
角のケースは、P = 2、P - 1 が 2 の冪 (フェルマー素数)、10^12 以下で最大の素数、P - 1 = 2q (q は素数)、
P - 1 の約数が多いもの、P - 1 が 98 より大きい素数の 2 乗や 3 乗や 2 つの積を持つもの
(Factors は 98 未満を割り算で取り、残りを Pollard の rho で割る)、P - 1 が 998244353 の倍数のもの
(約数 d が法の倍数になる)。
seed が 1000 以上なら、x ごとに x^n を全部たどる愚直解で解ける小さい P を出す (pj testdata crosscheck 用)。
2000 以上なら、愚直解でまだ解ける 20000 までの P を出す。
"""

import random
import sys

MAX_P = 10**12
MOD = 998244353

FIXED = [
    3,  # 例 1
    11,  # 例 2
    998244353,  # 例 3
    2,
    5,
    7,
    17,
    65537,
    1000000007,
    999999999989,  # 10^12 以下で最大の素数
    999999999959,  # P - 1 = 2 * 499999999979
    838053216001,  # P - 1 の約数が 5760 個
    806626220401,  # 5400 個
    293318625601,  # 5040 個
    999892002917,  # P - 1 = 4 * 499973^2
    999807526087,  # P - 1 = 6 * 408209^2
    999998011907,  # P - 1 = 2 * 7937^3
    999777185567,  # P - 1 = 2 * 707027 * 707029
    23957864473,  # P - 1 = 24 * 998244353
    972289999823,  # P - 1 = 974 * 998244353
    206158430209,  # P - 1 = 3 * 2^36
]
# (下限, 上限)。この範囲のランダムな数から上へ探した最初の素数。
RANDOM = [
    (2, MAX_P),
    (2, MAX_P),
    (MAX_P - 10**6, MAX_P),
    (10**11, MAX_P),
    (10**6, 10**7),
    (2, 1000),
]
COUNT = len(FIXED) + len(RANDOM)


def is_prime(n: int) -> bool:
    """決定的な Miller-Rabin (n < 3.3 * 10^24 で正しい底)。"""
    bases = (2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37)
    if n < 2:
        return False
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


def random_prime(rng: random.Random, low: int, high: int) -> int:
    while True:
        p = rng.randint(low, high)
        while p <= high and not is_prime(p):
            p += 1
        if p <= high:
            return p


def case_for(seed: int, rng: random.Random) -> int:
    if seed >= 2000:
        return random_prime(rng, 2, 20000)
    if seed >= 1000:
        # 愚直解が O(P^2) で解ける大きさ。小さい素数を多めに混ぜる。
        return random_prime(rng, 2, rng.choice([20, 200, 2000]))
    if seed < len(FIXED):
        return FIXED[seed]
    low, high = RANDOM[seed - len(FIXED)]
    return random_prime(rng, low, high)


def main() -> None:
    seed = int(sys.argv[1])
    p = case_for(seed, random.Random(seed))
    assert 2 <= p <= MAX_P and is_prime(p)
    print(p)


if __name__ == "__main__":
    main()
