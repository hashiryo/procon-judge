#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""GF(2^64) の元の位数 の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

期待出力は参照実装 (problem.toml の reference) がハーネスと一緒に作るので、ここでは入力だけを書く。
1 行が 1 個の a (0 でない) で、a^k = 1 となる最小の k ≥ 1 を求める。

一様ランダムな a は半分ほどが最大の位数 2^64-1 を持つ (素数 p ごとに p 成分が 1 でない確率が 1 - 1/p で、
その積が 0.5 ほど)。大きい素数の成分が 1 になる元はほとんど出ないので、位数を指定した元を混ぜる。
素数ごとに成分を 1 にするかを半々に選び、1 にしない成分は位数 p の部分群の元を一様に取って掛ける。
部分群の元は位数 p の生成元の冪の表を引いて作る (65537 と 6700417 は 2 段の表)。

時間を比べるのは最後のケース (10^5 個のうち 8 割が一様ランダム、2 割が位数を指定した元)。
"""

import random
import sys

MASK64 = (1 << 64) - 1
ORDER = MASK64  # 乗法群の位数 2^64 - 1
PRIMES = (3, 5, 17, 257, 641, 65537, 6700417)  # ORDER の素因数。どれも 1 乗
G = 2  # 多項式基底の x。原始元 (Library の GF2p64::generator() と同じ)

# seed -> (種類, 件数)
CASES = {
    0: ("sample", 0),  # 件数は並べた例の数で決まる
    1: ("random", 0),
    2: ("edge", 0),
    3: ("order", 1000),
    4: ("random", 10000),
    5: ("mixed", 100000),
}


def gf_mul(a: int, b: int) -> int:
    """GF(2)[x] / (x^64 + x^4 + x^3 + x + 1) の積。b の立っている bit だけ a をずらして足し、最後に畳む。"""
    r = 0
    while b:
        low = b & -b
        r ^= a * low
        b ^= low
    while r >> 64:
        h = r >> 64
        r = (r & MASK64) ^ h ^ (h << 1) ^ (h << 3) ^ (h << 4)
    return r


def gf_pow(a: int, e: int) -> int:
    r = 1
    while e:
        if e & 1:
            r = gf_mul(r, a)
        a = gf_mul(a, a)
        e >>= 1
    return r


class Subgroups:
    """位数 p の部分群の元 g_p^j を表で作る。641 以下は全部の冪を、65537 と 6700417 は j を上と下に分けた 2 段の表を持つ。"""

    def __init__(self) -> None:
        self.tab = {}
        for p in PRIMES:
            g = gf_pow(G, ORDER // p)
            step = 1 if p <= 641 else (256 if p == 65537 else 4096)
            lo = [1]
            for _ in range((step if step > 1 else p) - 1):
                lo.append(gf_mul(lo[-1], g))
            if step == 1:
                self.tab[p] = (1, lo, None)
                continue
            gs = gf_mul(lo[-1], g)  # g^step
            hi = [1]
            for _ in range((p - 1) // step):
                hi.append(gf_mul(hi[-1], gs))
            self.tab[p] = (step, lo, hi)

    def power(self, p: int, j: int) -> int:
        step, lo, hi = self.tab[p]
        return lo[j] if step == 1 else gf_mul(hi[j // step], lo[j % step])

    def element(self, rng: random.Random) -> int:
        """素数ごとに半々で成分を 1 でなくした元。位数はその素数の積になる。"""
        x = 1
        for p in PRIMES:
            if rng.random() < 0.5:
                x = gf_mul(x, self.power(p, rng.randrange(1, p)))
        return x


def random_element(rng: random.Random) -> int:
    return rng.randint(1, MASK64)


def make(kind: str, t: int, rng: random.Random) -> list:
    if kind == "sample":
        g3 = gf_pow(G, ORDER // 3)
        return [1, G, MASK64, g3, gf_pow(G, ORDER // 65537), gf_pow(G, ORDER // 6700417), gf_mul(g3, gf_pow(G, ORDER // 641)), 0x12345678ABCDEF00]
    if kind == "edge":
        rows = [1, G, gf_pow(G, ORDER - 1), MASK64, 1 << 63, 1 << 62, 3 << 62, 0x8000000000000001, 0xFFFFFFFF, 0xFFFFFFFF00000000]
        # 部分体 GF(2^k) の生成元 (位数 2^k - 1)
        rows += [gf_pow(G, ORDER // ((1 << k) - 1)) for k in (2, 4, 8, 16, 32)]
        # 位数 p の元とその逆元、位数 (2^64-1)/p の元、2 つの素数の積の位数の元
        gp = {p: gf_pow(G, ORDER // p) for p in PRIMES}
        rows += [gp[p] for p in PRIMES] + [gf_pow(gp[p], p - 1) for p in PRIMES] + [gf_pow(G, p) for p in PRIMES]
        rows += [gf_mul(gp[p], gp[q]) for i, p in enumerate(PRIMES) for q in PRIMES[i + 1:]]
        return rows
    if kind == "order":
        sg = Subgroups()
        return [sg.element(rng) for _ in range(t)]
    if kind == "random":
        return [random_element(rng) for _ in range(t)]
    if kind == "mixed":
        sg = Subgroups()
        rows = [random_element(rng) for _ in range(t * 4 // 5)]
        rows += [sg.element(rng) for _ in range(t - len(rows))]
        rng.shuffle(rows)
        return rows
    raise ValueError(kind)


def main() -> None:
    seed = int(sys.argv[1])
    if seed not in CASES:
        raise SystemExit(f"seed {seed} は {len(CASES)} 未満にしてください")
    kind, t = CASES[seed]
    rng = random.Random(seed * 1_000_003 + 12345)
    rows = make(kind, t, rng)
    out = [str(len(rows))]
    out += [str(a) for a in rows]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
