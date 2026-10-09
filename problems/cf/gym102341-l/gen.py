"""cf-gym102341-l (Lati@s) の入力を作る。n と、n × n の行列 M (0 <= M_ij < 2^64) を出す。

答えは、nimber の体での M のパーマネント (標数 2 なので行列式) が 0 でなければ先手の勝ち。0 になる行列は、
ある行を別の 2 行の nimber 倍の和にして作る (nimber の積はここで素朴に計算する。8 bit の表から組む)。
seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース (n = 1、0 の行、全部同じ値、0 と 1 だけ、
下三角)、n = 150 のランダム、階数を 1 つ落としたもの、2^32 未満の値 (部分体) で階数を落としたもの。
seed が 1000 以上なら、全部の置換を数える愚直解 brute.cpp で解ける n <= 7 の入力を出す (半分ほどは階数を落とす)。
"""

import random
import sys

MAX_N = 150
M64 = (1 << 64) - 1


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


assert mul(4, 4) == 6 and mul(8, 8) == 13 and mul(32, 64) == 141 and mul(5, 6) == 8

Matrix = list[list[int]]


def random_matrix(rng: random.Random, n: int, hi: int) -> Matrix:
    return [[rng.randint(0, hi) for _ in range(n)] for _ in range(n)]


def drop_rank(rng: random.Random, m: Matrix, hi: int) -> Matrix:
    """ある行を、別の 2 行の nimber 倍の和で置き換えて、行列式を 0 にする。"""
    n = len(m)
    if n == 1:
        return [[0]]
    k = rng.randrange(n)
    i, j = rng.sample([r for r in range(n) if r != k], 2) if n >= 3 else (1 - k, 1 - k)
    c, d = rng.randint(0, hi), (rng.randint(0, hi) if i != j else 0)
    m[k] = [mul(c, x) ^ mul(d, y) for x, y in zip(m[i], m[j])]
    return m


SAMPLES = [
    [[0, 1, 2], [1, 2, 3], [1, 2, 1]],
    [[1, 2], [2, 3]],
]


def plans(seed: int, rng: random.Random) -> Matrix:
    n = MAX_N
    if seed == 2:
        return [[0]]
    if seed == 3:
        return [[M64]]
    if seed == 4:  # 0 だけの行
        m = random_matrix(rng, n, M64)
        m[rng.randrange(n)] = [0] * n
        return m
    if seed == 5:  # 全部同じ値 (階数 1)
        v = rng.randint(1, M64)
        return [[v] * n for _ in range(n)]
    if seed == 6:  # 0 と 1 だけ (F_2 の行列。行列式は F_2 の行列式と同じ)
        return random_matrix(rng, n, 1)
    if seed == 7:  # 下三角で対角が 0 でない (行列式は対角の積)
        return [[rng.randint(1, M64) if j == i else (rng.randint(0, M64) if j < i else 0) for j in range(n)] for i in range(n)]
    if seed == 8:  # 下三角で対角に 0 が 1 つ (行列式は 0)
        m = [[rng.randint(1, M64) if j == i else (rng.randint(0, M64) if j < i else 0) for j in range(n)] for i in range(n)]
        k = rng.randrange(n)
        m[k][k] = 0
        return m
    if seed == 9:
        return random_matrix(rng, n, M64)
    if seed == 10:
        return drop_rank(rng, random_matrix(rng, n, M64), M64)
    if seed == 11:  # 2^32 未満の値は部分体をなすので、その中で階数を落とす
        return drop_rank(rng, random_matrix(rng, n, (1 << 32) - 1), (1 << 32) - 1)
    if seed == 12:  # 置換行列の形 (行列式は 0 でない値の積)
        perm = list(range(n))
        rng.shuffle(perm)
        return [[rng.randint(1, M64) if perm[i] == j else 0 for j in range(n)] for i in range(n)]
    if seed == 13:  # 小さい値で階数を落とす
        return drop_rank(rng, random_matrix(rng, n, 3), 3)
    assert False


COUNT = 14


def case_for(seed: int) -> Matrix:
    rng = random.Random(seed)
    if seed >= 1000:
        n = rng.randint(1, 7)
        hi = rng.choice([1, 3, 15, 255, M64])
        m = random_matrix(rng, n, hi)
        return drop_rank(rng, m, hi) if rng.random() < 0.5 else m
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    return plans(seed, rng)


def main() -> None:
    seed = int(sys.argv[1])
    m = case_for(seed)
    n = len(m)
    assert 1 <= n <= MAX_N and all(len(r) == n and all(0 <= x <= M64 for x in r) for r in m)
    sys.stdout.write(f"{n}\n" + "".join(" ".join(map(str, r)) + "\n" for r in m))


if __name__ == "__main__":
    main()
