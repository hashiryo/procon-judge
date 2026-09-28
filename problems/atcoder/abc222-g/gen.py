"""abc222-g (222) の入力を作る。T と T 個の K を出す。

seed が 0 から count - 1 までは本番のケース。例、角のケース、T = 200 のいろいろな K。
答えは、M = 9K / gcd(K, 2) を法とした 10 の位数 (M が 10 と互いに素でなければ -1)。
K が 4 か 5 の倍数なら -1。答えが K に近いほど、提出の離散対数 (Baby-step Giant-step) は長く探す。
K は、10^8 の近く、10 が原始根になる素数 p (答えは p - 1)、その 2 倍 3 倍 9 倍 27 倍 (答えは K に近い)、
2 × 奇数 (漸化式 x -> 10x + 2 が単射でない)、4 か 5 の倍数、素数のべき、10^k - 1 の約数 (K が大きくても答えが小さい)、ランダム。
seed が 1000 以上なら、漸化式を K 回まわす愚直解で解ける小さい K を出す (pj testdata crosscheck 用)。
2000 以上なら、愚直解でまだ解ける 3 × 10^7 までの K を少しだけ出す。
"""

import random
import sys

MAX_T = 200
MAX_K = 10**8

SAMPLE = [1, 7, 10, 999983]


def is_prime(n: int) -> bool:
    if n < 2:
        return False
    for p in (2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37):
        if n % p == 0:
            return n == p
    d, s = n - 1, 0
    while d % 2 == 0:
        d //= 2
        s += 1
    for a in (2, 3, 5, 7):  # n < 3.2 × 10^9 ならこの 4 つで足りる
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


def prime_factors(n: int) -> list[int]:
    out, d = [], 2
    while d * d <= n:
        if n % d == 0:
            out.append(d)
            while n % d == 0:
                n //= d
        d += 1
    return out + ([n] if n > 1 else [])


def full_reptend(p: int) -> bool:
    """p を法として 10 が原始根か (p は 2, 5 以外の素数)。"""
    return all(pow(10, (p - 1) // q, p) != 1 for q in prime_factors(p - 1))


def full_reptend_below(limit: int, count: int, mod3: int | None = None) -> list[int]:
    """limit 以下で 10 が原始根になる素数を、大きい方から count 個。mod3 を渡せば p ≡ mod3 (mod 3) のものだけ。"""
    out, p = [], limit
    while len(out) < count:
        if (mod3 is None or p % 3 == mod3) and p not in (2, 5) and is_prime(p) and full_reptend(p):
            out.append(p)
        p -= 1
    return out


def rep_divisors() -> list[int]:
    """10^k - 1 (k <= 16) の 10^8 以下の約数。K が大きくても答えは k 以下になる。"""
    out = set()
    for k in range(1, 17):
        n = 10**k - 1
        ds = [1]
        for p in prime_factors(n):
            e, m = 0, n
            while m % p == 0:
                m //= p
                e += 1
            ds = [d * p**i for d in ds for i in range(e + 1) if d * p**i <= MAX_K]
        out.update(ds)
    return sorted(out)


def plan_ks(plan: str, rng: random.Random) -> list[int]:
    if plan == "one":
        return [1]
    if plan == "max":
        return [MAX_K]
    if plan == "first":
        return list(range(1, MAX_T + 1))
    if plan == "last":
        return list(range(MAX_K - MAX_T + 1, MAX_K + 1))
    if plan == "reptend":
        return full_reptend_below(MAX_K, MAX_T)
    if plan == "reptend_times":
        ks = [2 * p for p in full_reptend_below(MAX_K // 2, 40)]
        ks += [3 * p for p in full_reptend_below(MAX_K // 3, 40, mod3=2)]
        ks += [6 * p for p in full_reptend_below(MAX_K // 6, 40, mod3=2)]
        ks += [9 * p for p in full_reptend_below(MAX_K // 9, 40, mod3=2)]
        ks += [27 * p for p in full_reptend_below(MAX_K // 27, 40, mod3=2)]
        return ks
    if plan == "even":
        # 2 × 奇数 (5 の倍数でない)。
        ks = []
        while len(ks) < MAX_T:
            k = 2 * rng.randrange(1, MAX_K // 2, 2)
            if k % 5:
                ks.append(k)
        return ks
    if plan == "none":
        # 4 か 5 の倍数。2 と 5 のべきも入れる。
        ks = [2**e for e in range(2, 27)] + [5**e for e in range(1, 12)] + [10**e for e in range(1, 9)]
        while len(ks) < MAX_T:
            ks.append(rng.choice([4, 5, 20, 25, 8, 40]) * rng.randint(1, MAX_K // 40))
        return ks
    if plan == "prime_powers":
        ks = []
        for p in (3, 7, 11, 13, 17, 37, 41, 73, 101, 137, 9091, 9901):
            q = p
            while q <= MAX_K:
                ks += [q, 2 * q] if 2 * q <= MAX_K else [q]
                q *= p
        ks += [3**a * p for a in range(1, 6) for p in full_reptend_below(MAX_K // 3**a, 4)]
        return ks[:MAX_T]
    if plan == "rep_divisors":
        ds = rep_divisors()
        big = [d for d in ds if d > 10**6]
        return sorted(rng.sample(big, min(len(big), 150)) + rng.sample(ds, 50))
    if plan == "random":
        return [rng.randint(1, MAX_K) for _ in range(MAX_T)]
    if plan == "coprime":
        ks = []
        while len(ks) < MAX_T:
            k = rng.randint(1, MAX_K)
            if k % 2 and k % 5:
                ks.append(k)
        return ks
    if plan == "small":
        return [rng.randint(1, 10**4) for _ in range(MAX_T)]
    assert plan == "near_max"
    ks = [MAX_K - 1, MAX_K - 2, MAX_K - 3, 99999989, 3**16, 2 * 3**16, 7**9, 2 * 7**9]
    ks += [rng.randint(9 * 10**7, MAX_K) for _ in range(MAX_T - len(ks))]
    return ks


PLANS = [
    "one",
    "max",
    "first",
    "last",
    "reptend",
    "reptend_times",
    "even",
    "none",
    "prime_powers",
    "rep_divisors",
    "random",
    "coprime",
    "small",
    "near_max",
]
COUNT = 1 + len(PLANS)


def small_ks(rng: random.Random, medium: bool) -> list[int]:
    """愚直解用。4 と 5 の倍数、2 × 奇数、3 のべきを混ぜる。"""
    if medium:
        return [rng.randint(1, 3 * 10**7) for _ in range(rng.randint(1, 3))]
    ks = []
    for _ in range(rng.randint(1, 10)):
        kind = rng.randrange(4)
        if kind == 0:
            ks.append(rng.randint(1, 5000))
        elif kind == 1:
            ks.append(rng.choice([4, 5, 8, 10, 20, 25]) * rng.randint(1, 200))
        elif kind == 2:
            ks.append(2 * rng.randrange(1, 2500, 2))
        else:
            ks.append(3 ** rng.randint(0, 7) * rng.choice([1, 2, 7, 11, 37]))
    return ks


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 1000:
        ks = small_ks(rng, seed >= 2000)
    elif seed == 0:
        ks = SAMPLE
    else:
        ks = plan_ks(PLANS[seed - 1], rng)
    assert 1 <= len(ks) <= MAX_T and all(1 <= k <= MAX_K for k in ks)
    sys.stdout.write("\n".join(map(str, [len(ks)] + ks)) + "\n")


if __name__ == "__main__":
    main()
