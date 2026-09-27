#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""GF(2^64) の任意の底の離散対数 の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

期待出力は参照実装 (problem.toml の reference) がハーネスと一緒に作るので、ここでは入力だけを書く。
1 行が 1 組の (a, b) で、a^k = b となる k を求める。a も b も 0 は出さない。

一様ランダムな (a, b) は 4 割ほどが解なしになる (素数 p ごとに解ける確率が 1 - 1/p + 1/p^2 で、
その積が 0.61 ほど)。解なしは部分群へ移すだけで判定でき、BSGS を回さずに済むので、多すぎると
早く抜ける実装だけが有利になる。解けるケースは b = a^k で作り、解なしを混ぜる割合は種類ごとに決める。
"""

import random
import sys

MASK64 = (1 << 64) - 1
ORDER = MASK64  # 乗法群の位数 2^64 - 1
PRIMES = (3, 5, 17, 257, 641, 65537, 6700417)  # ORDER の素因数。どれも 1 乗
IRRED_LOW = 0x1B  # x^64 ≡ x^4 + x^3 + x + 1
G = 2  # 多項式基底の x。原始元

# seed -> (種類, 件数)
CASES = {
    0: ("sample", 0),  # 件数は並べた例の数で決まる
    1: ("small", 50),
    2: ("edge", 200),
    3: ("solvable", 1000),
    4: ("solvable", 10000),
    5: ("random", 10000),
}


def gf_mul(a: int, b: int) -> int:
    """GF(2)[x] / (x^64 + x^4 + x^3 + x + 1) の積。素朴でよい (入力を作るだけ)。"""
    r = 0
    while b:
        if b & 1:
            r ^= a
        b >>= 1
        a <<= 1
        if a >> 64:
            a = (a & MASK64) ^ IRRED_LOW
    return r


def gf_pow(a: int, e: int) -> int:
    r = 1
    while e:
        if e & 1:
            r = gf_mul(r, a)
        a = gf_mul(a, a)
        e >>= 1
    return r


def random_element(rng: random.Random) -> int:
    return rng.randint(1, MASK64)


def random_divisor(rng: random.Random) -> int:
    """ORDER の約数を、素因数を半々の確率で選んで作る。"""
    d = 1
    for p in PRIMES:
        if rng.random() < 0.5:
            d *= p
    return d


def subgroup_element(rng: random.Random, d: int) -> int:
    """位数が d を割る元 (d は ORDER の約数)。G^((ORDER / d)·j) の j をランダムに取る。"""
    return gf_pow(G, (ORDER // d) * rng.randrange(d))


def solvable(rng: random.Random, a: int) -> tuple[int, int]:
    return a, gf_pow(a, rng.randrange(ORDER))


def edge_pair(rng: random.Random) -> tuple[int, int]:
    """位数の小さい底や、部分群の端に当たる組。解なしもここで混ぜる。"""
    kind = rng.randrange(8)
    if kind == 0:  # 位数の小さい底と、その冪
        return solvable(rng, subgroup_element(rng, random_divisor(rng)))
    if kind == 1:  # 位数の小さい底と、ランダムな元 (ほとんどが解なし)
        return subgroup_element(rng, random_divisor(rng)), random_element(rng)
    if kind == 2:  # ランダムな底と、小さい部分群の元
        return random_element(rng), subgroup_element(rng, random_divisor(rng))
    if kind == 3:  # 素因数を 1 つだけ欠く底。その素数の成分が 1 なので、そこの BSGS が要らない
        p = rng.choice(PRIMES)
        a = gf_pow(random_element(rng), p)
        return (a, gf_pow(a, rng.randrange(ORDER))) if rng.random() < 0.5 else (a, random_element(rng))
    if kind == 4:  # a = 1
        return 1, (1 if rng.random() < 0.5 else random_element(rng))
    if kind == 5:  # b = 1
        return random_element(rng), 1
    if kind == 6:  # a = b
        a = random_element(rng)
        return a, a
    # b = a^-1 (k = ord(a) - 1 が最小の解)
    a = random_element(rng)
    return a, gf_pow(a, ORDER - 1)


def make(kind: str, t: int, rng: random.Random) -> list[tuple[int, int]]:
    if kind == "sample":
        x = gf_pow(G, 12345)
        return [
            (2, 2),  # k = 1
            (2, 1),  # k = 0
            (1, 1),  # k = 0 (a = 1 でも b = 1 なら解がある)
            (1, 2),  # 解なし
            (2, 4),  # k = 2
            (gf_pow(G, 3), G),  # 解なし (log a が 3 の倍数で、log b = 1 は違う)
            (gf_pow(G, 3), gf_pow(G, 6)),  # k ≡ 2 (mod (2^64-1)/3)
            (gf_pow(G, ORDER // 3), gf_pow(G, 2 * (ORDER // 3))),  # 位数 3 の底、k ≡ 2 (mod 3)
            (gf_pow(G, ORDER // 6700417), gf_pow(G, 5 * (ORDER // 6700417))),  # k ≡ 5 (mod 6700417)
            (x, gf_pow(x, ORDER - 1)),  # b = a^-1
        ]
    if kind == "small":
        return [(a, gf_pow(a, rng.randint(0, 1000))) for a in (random_element(rng) for _ in range(t))]
    if kind == "edge":
        return [edge_pair(rng) for _ in range(t)]
    if kind == "solvable":
        return [solvable(rng, random_element(rng)) for _ in range(t)]
    if kind == "random":
        return [(random_element(rng), random_element(rng)) for _ in range(t)]
    raise ValueError(kind)


def main() -> None:
    seed = int(sys.argv[1])
    if seed not in CASES:
        raise SystemExit(f"seed {seed} は {len(CASES)} 未満にしてください")
    kind, t = CASES[seed]
    rng = random.Random(seed * 1_000_003 + 12345)
    rows = make(kind, t, rng)
    out = [str(len(rows))] + [f"{a} {b}" for a, b in rows]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
