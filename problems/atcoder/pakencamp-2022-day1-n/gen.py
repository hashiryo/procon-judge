#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""パ研マシン (pakencamp 2022 day1 N) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

1 行目に T。各テストケースは
  s t P
  A_r B_r A_b B_b A_y B_y
の 2 行。色は赤 (0) → 青 (1) → 黄 (2) → 赤 と移り、色 c のとき x = (A_c x + B_c) mod P。
表示を t にする最小の回数を出す (無ければ -1)。seed 1000 以上は愚直解と突き合わせる小さい入力。
乱数は Python の random の版による違いを避けるため splitmix64 で作る。
"""
import sys

MASK64 = (1 << 64) - 1
SMALL_PRIMES = [2, 3, 5, 7, 11, 13, 17, 19]
MID_PRIMES = [1000003, 999983]
BIG_PRIMES = [1000000007, 998244353, 999999937, 1000000009]


class SplitMix64:
    def __init__(self, seed: int) -> None:
        self.x = seed & MASK64

    def next(self) -> int:
        self.x = (self.x + 0x9E3779B97F4A7C15) & MASK64
        z = self.x
        z = ((z ^ (z >> 30)) * 0xBF58476D1CE4E5B9) & MASK64
        z = ((z ^ (z >> 27)) * 0x94D049BB133111EB) & MASK64
        return z ^ (z >> 31)

    def below(self, n: int) -> int:
        return (self.next() * n) >> 64


def compose(m1, m2, P):
    return ((m1[0] * m2[0]) % P, (m1[0] * m2[1] + m1[1]) % P)


def aff_pow(m, e, P):
    ra, rb = 1 % P, 0
    pa, pb = m[0] % P, m[1] % P
    while e:
        if e & 1:
            ra, rb = pa * ra % P, (pa * rb + pb) % P
        pa, pb = pa * pa % P, (pa * pb + pb) % P
        e >>= 1
    return ra, rb


def state_after(maps, s, n, P):
    r = n % 3
    x = s
    for i in range(r):
        x = (maps[i][0] * x + maps[i][1]) % P
    g = compose(maps[(r + 2) % 3], compose(maps[(r + 1) % 3], maps[r], P), P)
    a, b = aff_pow(g, n // 3, P)
    return (a * x + b) % P


def main() -> None:
    seed = int(sys.argv[1])
    r = SplitMix64(seed * 1000003 + 12345)
    out = []

    def push(maps, s, t, P):
        out.append(f"{s} {t} {P}")
        out.append(" ".join(map(str, (maps[0][0], maps[0][1], maps[1][0], maps[1][1], maps[2][0], maps[2][1]))))

    def random_maps(P, A_max=None):
        hi = P if A_max is None else min(P, A_max)
        return [(r.below(hi), r.below(P)) for _ in range(3)]

    def solution_case(P):
        maps = random_maps(P)
        s = r.below(P)
        n0 = r.below(3 * P)
        t = state_after(maps, s, n0, P)
        push(maps, s, t, P)

    def no_solution_case(P):
        # A = 0 の定数写像にして、軌道を小さくする (0 に落ちるので、t を 0 以外にする)。
        maps = [(0, r.below(P)) for _ in range(3)]
        s = r.below(P)
        t = 1 + r.below(P - 1)
        push(maps, s, t, P)

    if seed >= 1000:
        T = 1 + r.below(5)
        for _ in range(T):
            P = SMALL_PRIMES[r.below(len(SMALL_PRIMES))]
            push(random_maps(P), r.below(P), r.below(P), P)
    else:
        cases = {
            0: "sample",
            1: (10, SMALL_PRIMES, "sol"),
            2: (10, BIG_PRIMES, "sol"),
            3: (10, BIG_PRIMES, "nosol"),
            4: (10, SMALL_PRIMES, "mixed"),
            5: (10, BIG_PRIMES, "mixed"),
            6: (10, MID_PRIMES, "sol"),
            7: (10, BIG_PRIMES, "nosol"),
            8: (10, BIG_PRIMES, "sol"),
            9: (10, MID_PRIMES, "mixed"),
        }[seed]
        if cases == "sample":
            out = [
                "1 8 13", "1 2 3 4 5 6",
                "0 1 3", "1 0 1 0 1 0",
                "123456789 634 998244353", "1 23 456 7890 123456789 987654321",
            ]
        else:
            T, primes, kind = cases
            for _ in range(T):
                P = primes[r.below(len(primes))]
                if kind == "sol":
                    solution_case(P)
                elif kind == "nosol":
                    no_solution_case(P)
                else:
                    if r.below(2):
                        solution_case(P)
                    else:
                        no_solution_case(P)
    print(len(out) // 2)
    print("\n".join(out))


if __name__ == "__main__":
    main()
