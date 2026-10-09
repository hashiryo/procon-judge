"""cf-1310-f (Bad Cryptography) の入力を作る。t と、t 個の組 a b (1 <= a, b < 2^64、nimber) を出す。

答えは a^x = b となる x (どれでもよい) か -1 で、checker.cpp が入力と出力だけで判定する。
乗法群の位数 2^64 - 1 = 3·5·17·257·641·65537·6700417 は平方因子を持たないので、元の位数を
r^((2^64-1)/m) の形で m の約数に決められる。nimber の積はここで素朴に計算する (8 bit の表から組む)。
seed が 0 から count - 1 までは本番のケース。例、角のケース 2 つ (a = 1、部分体の元、素数の位数の元)、
ランダムな a と b = a^k、ランダムな組、原始元の a (BSGS などでいちばん重い形)、答えが 0 や 2^64 - 2 に近いもの、
位数の大きい原始元でない a。seed が 1000 以上なら、位数が 2 × 10^6 以下の a だけを出す
(冪を順に並べる愚直解 brute.cpp で解ける。pj testdata crosscheck 用)。
"""

import random
import sys

M = (1 << 64) - 1
PRIMES = [3, 5, 17, 257, 641, 65537, 6700417]
MAX_T = 100


def _mul_rec(a: int, b: int, width: int) -> int:
    """width bit (2 の冪) の nimber の積。上下に分けて、X = 2^(width/2) について X^2 = X + X/2 を使う。"""
    if width == 1:
        return a & b
    h = width >> 1
    m = (1 << h) - 1
    a1, a0, b1, b0 = a >> h, a & m, b >> h, b & m
    c, d, e = _mul_rec(a0, b0, h), _mul_rec(a1 ^ a0, b1 ^ b0, h), _mul_rec(a1, b1, h)
    return ((d ^ c) << h) | (c ^ _mul_rec(e, 1 << (h - 1), h))


# 8 bit どうしの積の表。積は双線形なので、2 の冪どうしの積から xor で組む。
_Q = [[_mul_rec(1 << i, 1 << j, 8) for j in range(8)] for i in range(8)]
_P = [[0] * 256 for _ in range(8)]
for _i in range(8):
    for _b in range(1, 256):
        _P[_i][_b] = _P[_i][_b & (_b - 1)] ^ _Q[_i][(_b & -_b).bit_length() - 1]
_T8 = [0] * 65536
for _a in range(1, 256):
    _base, _row = (_a & (_a - 1)) << 8, _P[(_a & -_a).bit_length() - 1]
    for _b in range(256):
        _T8[(_a << 8) | _b] = _T8[_base | _b] ^ _row[_b]


def mul(a: int, b: int, width: int = 64) -> int:
    if width == 8:
        return _T8[(a << 8) | b]
    if a <= 1 or b <= 1:
        return a * b
    h = width >> 1
    m = (1 << h) - 1
    a1, a0, b1, b0 = a >> h, a & m, b >> h, b & m
    c, d, e = mul(a0, b0, h), mul(a1 ^ a0, b1 ^ b0, h), mul(a1, b1, h)
    return ((d ^ c) << h) | (c ^ mul(e, 1 << (h - 1), h))


def power(a: int, e: int) -> int:
    r = 1
    while e:
        if e & 1:
            r = mul(r, a)
        a = mul(a, a)
        e >>= 1
    return r


assert mul(4, 4) == 6 and mul(8, 8) == 13 and mul(32, 64) == 141 and mul(5, 6) == 8


def order(a: int) -> int:
    o = 1
    for p in PRIMES:
        if power(a, M // p) != 1:
            o *= p
    return o


def nonzero(rng: random.Random, bits: int = 64) -> int:
    return rng.randint(1, (1 << bits) - 1)


def with_order_dividing(rng: random.Random, m: int) -> int:
    """位数が m の約数の元。m が素数なら、ほとんど位数 m になる。"""
    return power(nonzero(rng), M // m)


def primitive(rng: random.Random) -> int:
    while True:
        g = nonzero(rng)
        if order(g) == M:
            return g


SAMPLE = [
    (2, 2), (1, 1), (2, 3), (8, 10), (8, 2), (321321321321, 2), (123214213213, 4356903202345442785),
]


def corners_subfield(rng: random.Random) -> list[tuple[int, int]]:
    out = [(1, 1), (1, 2), (1, M), (2, 1), (M, 1), (2, 2), (M, M), (2, 3), (3, 2), (3, 3), (2, M), (M, 2)]
    # 2^(2^k) 未満の nimber は部分体をなす。a がその中にあると、b がその外なら解が無い。
    for bits in (2, 4, 8, 16, 32):
        for _ in range(6):
            a = nonzero(rng, bits)
            k = rng.randint(0, (1 << bits) - 2)
            out += [(a, power(a, k)), (a, nonzero(rng, bits)), (a, nonzero(rng, min(64, bits * 2)))]
    return out[:MAX_T]


def corners_prime_order(rng: random.Random) -> list[tuple[int, int]]:
    out = []
    for p in PRIMES:
        a = with_order_dividing(rng, p)
        while a == 1:
            a = with_order_dividing(rng, p)
        out += [(a, power(a, rng.randint(0, p - 1))), (a, nonzero(rng)), (a, 1), (a, power(a, p - 1))]
        # 位数 (2^64-1)/p の元。b の位数が p で割れると解が無い。
        c = power(nonzero(rng), p)
        out += [(c, power(c, rng.randint(0, M - 1))), (c, with_order_dividing(rng, p)), (c, nonzero(rng))]
    for _ in range(MAX_T - len(out)):
        m = 1
        for p in PRIMES:
            if rng.random() < 0.4:
                m *= p
        a = with_order_dividing(rng, m)
        out.append((a, power(a, rng.randint(0, M)) if rng.random() < 0.5 else with_order_dividing(rng, rng.choice(PRIMES))))
    return out


def random_power(rng: random.Random) -> list[tuple[int, int]]:
    out = []
    for _ in range(MAX_T):
        a = nonzero(rng)
        out.append((a, power(a, rng.randint(0, M))))
    return out


def random_pairs(rng: random.Random) -> list[tuple[int, int]]:
    return [(nonzero(rng), nonzero(rng)) for _ in range(MAX_T)]


def primitive_power(rng: random.Random) -> list[tuple[int, int]]:
    out = []
    for _ in range(MAX_T):
        g = primitive(rng)
        out.append((g, power(g, rng.randint(M // 2, M - 1))))
    return out


def primitive_random(rng: random.Random) -> list[tuple[int, int]]:
    return [(primitive(rng), nonzero(rng)) for _ in range(MAX_T)]


def extreme_exponents(rng: random.Random) -> list[tuple[int, int]]:
    out = []
    for _ in range(MAX_T):
        g = primitive(rng)
        k = rng.choice([0, 1, 2, 3, M - 1, M - 2, M - 3, rng.randint(0, 1000), rng.randint(M - 1000, M - 1)])
        out.append((g, power(g, k)))
    return out


def large_order(rng: random.Random) -> list[tuple[int, int]]:
    out = []
    for _ in range(MAX_T):
        p = rng.choice(PRIMES)
        a = power(nonzero(rng), p)  # 位数は (2^64-1)/p の約数
        b = power(a, rng.randint(0, M)) if rng.random() < 0.5 else nonzero(rng)
        out.append((a, b))
    return out


PLANS = [corners_subfield, corners_prime_order, random_power, random_pairs, primitive_power, primitive_random, extreme_exponents, large_order]
COUNT = 1 + len(PLANS)
SMALL = [3, 5, 17, 257, 641, 65537]


def small_case(rng: random.Random) -> list[tuple[int, int]]:
    out = []
    for _ in range(rng.randint(1, 8)):
        m = 1
        while True:
            m = 1
            for p in SMALL:
                if rng.random() < 0.35:
                    m *= p
            if m <= 2 * 10**6:
                break
        a = 1 if rng.random() < 0.05 else with_order_dividing(rng, m)
        r = rng.random()
        if r < 0.5:
            b = power(a, rng.randint(0, M))
        elif r < 0.8:
            b = with_order_dividing(rng, rng.choice(SMALL))
        else:
            b = rng.choice([1, a, nonzero(rng)])
        out.append((a, b))
    return out


def case_for(seed: int) -> list[tuple[int, int]]:
    rng = random.Random(seed)
    if seed >= 1000:
        return small_case(rng)
    if seed == 0:
        return SAMPLE
    return PLANS[seed - 1](rng)


def main() -> None:
    seed = int(sys.argv[1])
    pairs = case_for(seed)
    assert 1 <= len(pairs) <= MAX_T and all(1 <= a <= M and 1 <= b <= M for a, b in pairs)
    sys.stdout.write(f"{len(pairs)}\n" + "".join(f"{a} {b}\n" for a, b in pairs))


if __name__ == "__main__":
    main()
