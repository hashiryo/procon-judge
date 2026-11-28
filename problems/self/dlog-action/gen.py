#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""モノイド作用の離散対数の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

1 行目に Q。続く Q 行が次のいずれか。
  0 p a b s t N            f(x) = (a x + b) mod p を n 回作用させて s を t にする最小の n (0 <= n < N)
  1 p A0 A1 A2 A3 S0 S1 S2 S3 T0 T1 T2 T3 N   A^n S = T (2x2 行列の積 mod p) の最小の n
  2 L N p0..p_{L-1} s0..s_{L-1} t0..t_{L-1}   置換 p を列に n 回作用させて s を t にする最小の n
答えは無ければ -1。seed 1000 以上は愚直解と突き合わせる小さい入力。
乱数は Python の random の版による違いを避けるため splitmix64 で作る。
"""
import sys

MASK64 = (1 << 64) - 1
PRIME = 1000000007
COMPOSITE = 997 * 1000003  # 997002991


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


def affine_pow(a: int, b: int, n: int, p: int) -> tuple[int, int]:
    ra, rb = 1 % p, 0
    pa, pb = a % p, b % p
    while n:
        if n & 1:
            ra, rb = pa * ra % p, (pa * rb + pb) % p
        pa, pb = pa * pa % p, (pa * pb + pb) % p
        n >>= 1
    return ra, rb


def affine_apply(f: tuple[int, int], x: int, p: int) -> int:
    return (f[0] * x + f[1]) % p


def mat_mul(A, B, p):
    return (
        (A[0] * B[0] + A[1] * B[2]) % p,
        (A[0] * B[1] + A[1] * B[3]) % p,
        (A[2] * B[0] + A[3] * B[2]) % p,
        (A[2] * B[1] + A[3] * B[3]) % p,
    )


def mat_pow(A, n, p):
    R = (1, 0, 0, 1)
    B = A
    while n:
        if n & 1:
            R = mat_mul(B, R, p)
        B = mat_mul(B, B, p)
        n >>= 1
    return R


def perm_pow(perm, n):
    q = list(range(len(perm)))
    base = perm[:]
    while n:
        if n & 1:
            q = [base[q[i]] for i in range(len(perm))]
        base = [base[base[i]] for i in range(len(perm))]
        n >>= 1
    return q


def perm_apply(q, s):
    return [s[q[i]] for i in range(len(q))]


def perm_from_cycles(lengths):
    p = []
    start = 0
    for L in lengths:
        for i in range(L):
            p.append(start + (i + 1) % L)
        start += L
    return p


LONG_CYCLES = [16, 9, 5, 7, 11, 13, 17, 19, 3]  # 和 100、位数 232792560
LONG_ORDER = 232792560


def query_affine(p, a, b, s, t, N):
    return f"0 {p} {a} {b} {s} {t} {N}"


def query_matrix(p, A, S, T, N):
    return "1 " + " ".join(map(str, (p, *A, *S, *T, N)))


def query_perm(perm, s, t, N):
    return "2 " + " ".join(map(str, (len(perm), N, *perm, *s, *t)))


def main() -> None:
    seed = int(sys.argv[1])
    r = SplitMix64(seed * 1000003 + 12345)
    out = []

    def affine_solution(p):
        for _ in range(100):
            a = 2 + r.below(p - 3)
            b, s = r.below(p), r.below(p)
            n0 = 1 + r.below(p - 1)
            t = affine_apply(affine_pow(a, b, n0, p), s, p)
            if t != s:
                out.append(query_affine(p, a, b, s, t, p))
                return
        raise RuntimeError("affine solution not found")

    def affine_no_solution(p):
        a, b, s = p - 1, r.below(p), r.below(p)
        while True:
            t = r.below(p)
            if t != s and t != (b - s) % p:
                break
        out.append(query_affine(p, a, b, s, t, p))

    def matrix_solution(p):
        for _ in range(100):
            A = tuple(r.below(p) for _ in range(4))
            det = (A[0] * A[3] - A[1] * A[2]) % p
            if det == 0:
                continue
            n0 = 1 + r.below(2 * p)
            T = mat_pow(A, n0, p)
            if T != (1, 0, 0, 1):
                out.append(query_matrix(p, A, (1, 0, 0, 1), T, 2 * p + 1))
                return
        raise RuntimeError("matrix solution not found")

    def matrix_no_solution(p, q):
        A = (p - 1, 0, 0, p - 1)  # -I、位数 2
        while True:
            T = tuple(r.below(p) for _ in range(4))
            if T != (1, 0, 0, 1) and T != (p - 1, 0, 0, p - 1):
                break
        out.append(query_matrix(p, A, (1, 0, 0, 1), T, 2 * p + 1))

    def perm_solution(L, N, cycles, order):
        perm = perm_from_cycles(cycles)
        for _ in range(100):
            s = list(range(L))
            for i in range(L - 1, 0, -1):
                j = r.below(i + 1)
                s[i], s[j] = s[j], s[i]
            n0 = 1 + r.below(order - 1)
            t = perm_apply(perm_pow(perm, n0), s)
            if t != s:
                out.append(query_perm(perm, s, t, N))
                return
        raise RuntimeError("perm solution not found")

    def perm_no_solution(L, N, cycles):
        perm = perm_from_cycles(cycles)
        while True:
            s = [r.below(L) for _ in range(L)]
            t = [r.below(L) for _ in range(L)]
            if s != t:
                break
        out.append(query_perm(perm, s, t, N))

    if seed >= 1000:
        q = 1 + r.below(8)
        for _ in range(q):
            typ = r.below(3)
            if typ == 0:
                p = 2 + r.below(49)
                out.append(query_affine(p, r.below(p), r.below(p), r.below(p), r.below(p), 1 + r.below(200)))
            elif typ == 1:
                p = [2, 3, 5, 7, 11, 13, 17, 19, 23][r.below(9)]
                out.append(query_matrix(p, tuple(r.below(p) for _ in range(4)), tuple(r.below(p) for _ in range(4)),
                                         tuple(r.below(p) for _ in range(4)), 1 + r.below(200)))
            else:
                L = 1 + r.below(6)
                perm = list(range(L))
                for i in range(L - 1, 0, -1):
                    j = r.below(i + 1)
                    perm[i], perm[j] = perm[j], perm[i]
                out.append(query_perm(perm, [r.below(L) for _ in range(L)], [r.below(L) for _ in range(L)], 1 + r.below(200)))
    elif seed == 0:
        out.append(query_affine(11, 2, 1, 1, 2, 11))
        out.append(query_affine(11, 2, 1, 1, 4, 11))
        out.append(query_matrix(7, (2, 3, 1, 4), (1, 0, 0, 1), mat_pow((2, 3, 1, 4), 5, 7), 15))
        out.append(query_perm(perm_from_cycles([3, 2]), [1, 2, 0, 4, 3], [0, 1, 2, 3, 4], 10))
    elif seed == 1:
        for _ in range(200):
            affine_solution(PRIME)
    elif seed == 2:
        for _ in range(100):
            affine_no_solution(PRIME)
    elif seed == 3:
        for _ in range(4):
            matrix_solution(PRIME)
    elif seed == 4:
        for _ in range(4):
            matrix_no_solution(PRIME, None)
    elif seed == 5:
        for _ in range(4):
            perm_solution(100, 10**9, LONG_CYCLES, LONG_ORDER)
    elif seed == 6:
        for _ in range(2):
            perm_no_solution(100, 10**9, LONG_CYCLES)
    elif seed == 7:
        for _ in range(100):
            affine_solution(COMPOSITE)
    elif seed == 8:
        out.append(query_affine(2, 0, 1, 1, 1, 1))       # N = 1, s = t
        out.append(query_affine(2, 0, 1, 0, 1, 1))       # N = 1, s != t
        out.append(query_affine(2, 0, 1, 0, 1, 2))       # 定数写像、n = 1
        out.append(query_affine(2, 1, 1, 0, 1, 2))       # x + 1
        out.append(query_affine(3, 0, 0, 2, 0, 3))       # 0 に落ちる
        out.append(query_matrix(5, (0, 0, 0, 0), (1, 0, 0, 1), (1, 0, 0, 1), 1))  # A^0 = I
        out.append(query_matrix(5, (0, 0, 0, 0), (1, 0, 0, 1), (0, 0, 0, 0), 2))  # A^1 = 0
        out.append(query_matrix(5, (0, 0, 0, 0), (1, 0, 0, 1), (1, 2, 3, 4), 10))  # 解なし
        out.append(query_perm([0], [0], [0], 1))         # L = 1, s = t
        out.append(query_perm([1, 0], [1, 0], [0, 1], 2))  # 互換
        out.append(query_perm([1, 0], [0, 1], [0, 1], 1))  # N = 1, s != t
    else:
        for _ in range(10):
            affine_solution(PRIME)
        for _ in range(3):
            affine_no_solution(PRIME)
        for _ in range(2):
            matrix_solution(PRIME)
        for _ in range(2):
            matrix_no_solution(PRIME, None)
        for _ in range(2):
            perm_solution(100, 10**9, LONG_CYCLES, LONG_ORDER)
        for _ in range(10):
            affine_solution(COMPOSITE)

    print(len(out))
    print("\n".join(out))


if __name__ == "__main__":
    main()
