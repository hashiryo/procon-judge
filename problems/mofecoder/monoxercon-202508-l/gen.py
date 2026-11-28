#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""Monoxer L FF?DLP の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

1 行に A B C D p。(A + Bi)^k = C + Di (mod p) となる最小の k >= 0 を出す (無ければ -1)。i^2 = -1、p は素数。
seed 1000 以上は愚直解と突き合わせる小さい入力。
乱数は Python の random の版による違いを避けるため splitmix64 で作る。
"""
import sys

MASK64 = (1 << 64) - 1
P1 = [5, 13, 17, 29, 37, 41, 53, 61, 73, 89, 97, 998244353, 1000000009]  # p = 1 (mod 4)
P3 = [3, 7, 11, 19, 23, 31, 43, 47, 59, 67, 71, 79, 83, 1000000007]  # p = 3 (mod 4)
SMALL = [3, 5, 7, 11, 13, 17, 19, 23, 29, 31]


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


def cmul(x, y, p):
    return ((x[0] * y[0] - x[1] * y[1]) % p, (x[0] * y[1] + x[1] * y[0]) % p)


def cpow(z, e, p):
    r = (1 % p, 0)
    while e:
        if e & 1:
            r = cmul(r, z, p)
        z = cmul(z, z, p)
        e >>= 1
    return r


def main() -> None:
    seed = int(sys.argv[1])
    r = SplitMix64(seed * 1000003 + 12345)

    def norm(z, p):
        return (z[0] * z[0] + z[1] * z[1]) % p

    def solution_case(p):
        while True:
            A, B = r.below(p), r.below(p)
            if A or B:
                break
        k0 = r.below(p * p)
        C, D = cpow((A, B), k0, p)
        return A, B, C, D, p

    def no_solution_case(p):
        while True:
            A, B = r.below(p), r.below(p)
            if norm((A, B), p):
                break
        return A, B, 0, 0, p  # ノルムが可逆な元の累乗は 0 にならない

    if seed >= 1000:
        p = SMALL[r.below(len(SMALL))]
        print(*[r.below(p) for _ in range(4)], p)
        return

    if seed == 0:
        print(2, 3, 6, 1, 11)
    elif seed == 1:
        p = SMALL[r.below(len(SMALL))]
        print(*solution_case(p))
    elif seed == 2:
        print(*solution_case(P1[r.below(len(P1))]))
    elif seed == 3:
        print(*solution_case(P3[r.below(len(P3))]))
    elif seed == 4:
        print(*no_solution_case(P1[r.below(len(P1))]))
    elif seed == 5:
        print(0, 0, 0, 0, 5)  # 0^k = 0 (k >= 1)
    elif seed == 6:
        print(1, 0, 3, 0, 5)  # 1^k = 1 だけ
    elif seed == 7:
        print(*no_solution_case(P1[r.below(len(P1))]))
    elif seed == 8:
        print(*no_solution_case(P3[r.below(len(P3))]))
    else:
        p = SMALL[r.below(len(SMALL))]
        if r.below(2):
            print(*solution_case(p))
        else:
            print(*[r.below(p) for _ in range(4)], p)


if __name__ == "__main__":
    main()
