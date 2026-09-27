"""abc177-e (Coprime) の入力を作る。N と A_1 ... A_N を出す。答えは pairwise coprime、setwise coprime、not coprime のどれか。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、N = 2 と 3 の角のケース、大きいケース。
大きいケースは、全部 1、1 と 10^6 以下の素数全部 (pairwise)、それに素数を 1 つ重ねたもの (setwise)、
素数ごとに 10^6 以下で最大の冪と 1 (pairwise)、500 未満の素数と大きい素数の積と残りの素数 (pairwise で、
1 を含まない)、ランダム (setwise)、全部偶数、全部 997 の倍数、全部 999983 (not coprime)、小さい素数 2 つの
積だけ (setwise)、1 と素数の列で 499979 と 2 × 499979 の 1 組だけが coprime でないもの (setwise)。
N = 10^6 は入力が 7 MB になるので、半分ほどは N = 3 × 10^5 にする。
seed が 1000 以上なら、全部の組の最大公約数を計算する愚直解で解ける小さい入力を出す (pj testdata crosscheck 用)。
2000 以上なら、同じ愚直解でまだ解ける N = 2000 までの入力を出す。
"""

import bisect
import random
import sys

MAX_N = 10**6
MAX_A = 10**6

SAMPLES = [
    [3, 4, 5],
    [6, 10, 15],
    [6, 10, 16],
]
FIXED = [
    [1, 1],
    [1, MAX_A],
    [MAX_A, MAX_A],
    [2, 3],
    [4, 6],
    [999983, 999979],
    [999983, 999983],
    [1, 1, 1],
    [2, 3, 4],
    [30, 1, 7],
]
# (N, 値の出し方)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    (MAX_N, "ones"),
    (MAX_N, "primes"),
    (MAX_N, "primes_dup"),
    (300000, "prime_powers"),
    (300000, "semiprimes"),
    (MAX_N, "random"),
    (300000, "even"),
    (300000, "mult997"),
    (300000, "all_999983"),
    (300000, "small_semiprimes"),
    (300000, "shared_big_prime"),
]


def primes_upto(n: int) -> list[int]:
    sieve = bytearray([1]) * (n + 1)
    sieve[0:2] = b"\x00\x00"
    for p in range(2, int(n**0.5) + 1):
        if sieve[p]:
            sieve[p * p :: p] = bytearray(len(range(p * p, n + 1, p)))
    return [p for p in range(n + 1) if sieve[p]]


def pad_ones(rng: random.Random, n: int, values: list[int]) -> list[int]:
    """values に 1 を足して長さ n にし、混ぜる。"""
    assert len(values) <= n
    out = values + [1] * (n - len(values))
    rng.shuffle(out)
    return out


def values_for(rng: random.Random, n: int, how: str) -> list[int]:
    if how == "ones":
        return [1] * n
    if how in ("primes", "primes_dup"):
        ps = primes_upto(MAX_A)
        if how == "primes_dup":
            ps.append(rng.choice(ps))
        return pad_ones(rng, n, ps)
    if how == "prime_powers":
        out = []
        for p in primes_upto(MAX_A):
            q = p
            while q * p <= MAX_A:
                q *= p
            out.append(q)
        return pad_ones(rng, n, out)
    if how == "semiprimes":
        # 500 未満の素数 p を、まだ使っていない 500 より大きい素数のうち p q <= 10^6 で最大の q と組にした
        # 積と、残りの素数。1 を含まない。素数はどれも 1 回しか使わないので pairwise。
        ps = primes_upto(MAX_A)
        large = [q for q in ps if q > 500]
        used, out = set(), []
        for p in (p for p in ps if p < 500):
            i = bisect.bisect_right(large, MAX_A // p) - 1
            while large[i] in used:
                i -= 1
            used.add(large[i])
            out.append(p * large[i])
        out += [q for q in large if q not in used]
        rng.shuffle(out)
        return out[:n]
    if how == "random":
        return [rng.randint(1, MAX_A) for _ in range(n)]
    if how == "even":
        return [2 * rng.randint(1, MAX_A // 2) for _ in range(n)]
    if how == "mult997":
        return [997 * rng.randint(1, MAX_A // 997) for _ in range(n)]
    if how == "all_999983":
        return [999983] * n
    if how == "small_semiprimes":
        ps = [2, 3, 5, 7, 11, 13]
        return [rng.choice(ps) * rng.choice(ps) for _ in range(n)]
    assert how == "shared_big_prime"
    # 1 と素数 (2 と 499979 を除く) の pairwise の列に、499979 と 2 × 499979 を入れる。
    # coprime でない組はこの 1 組だけ。
    ps = [p for p in primes_upto(MAX_A) if p not in (2, 499979)][: n - 2]
    return pad_ones(rng, n, ps + [499979, 2 * 499979])


def small_case(rng: random.Random, n: int) -> list[int]:
    """愚直解向け。答えの 3 通りが同じくらい出るように作り方を選ぶ。"""
    how = rng.randrange(6)
    if how == 0:
        return [rng.randint(1, 30) for _ in range(n)]
    if how == 1:
        # 1 と相異なる素数、ときどき 1 つ重ねる。
        ps = rng.sample([2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 999983, 999979, 499979], min(n, 13))
        if rng.random() < 0.5:
            ps[-1] = ps[0]
        return pad_ones(rng, n, ps[: rng.randint(0, len(ps))])
    if how == 2:
        g = rng.choice([2, 3, 997, 499979])
        return [g * rng.randint(1, MAX_A // g) for _ in range(n)]
    if how == 3:
        return [rng.randint(1, MAX_A) for _ in range(n)]
    if how == 4:
        ps = [2, 3, 5, 7]
        return [rng.choice(ps) * rng.choice(ps) for _ in range(n)]
    return [rng.choice([1, 2, MAX_A - 1, MAX_A, 999983]) for _ in range(n)]


def case_for(seed: int, rng: random.Random) -> list[int]:
    if seed >= 1000:
        n = rng.randint(13, 2000) if seed >= 2000 else rng.randint(2, 12)
        return small_case(rng, n)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    n, how = PLANS[seed - len(FIXED)]
    return values_for(rng, n, how)


def main() -> None:
    seed = int(sys.argv[1])
    a = case_for(seed, random.Random(seed))
    assert 2 <= len(a) <= MAX_N and all(1 <= v <= MAX_A for v in a)
    sys.stdout.write(f"{len(a)}\n{' '.join(map(str, a))}\n")


if __name__ == "__main__":
    main()
