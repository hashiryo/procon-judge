"""abc179-e (Sequence Sum) の入力を作る。N X M を 1 行で出す。答えは A_1 = X、A_(n+1) = A_n^2 mod M の最初の N 項の和。

A は尻尾のあとに輪に入る形 (rho) になる。seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、
N を尻尾や輪の区切りに合わせたケース、N = 10^10 のランダム。角のケースは、M = 1、X = 0 (答え 0)、X = 1 (答え N)、
X = M - 1 (次から 1 が続く)、M = 2^16 (偶数の X はすぐ 0 になる)。M = 99707 と X = 2 は輪の長さが 49852 で、
M <= 10^5 で最も長い部類。M = 65537 は尻尾が 16 項で、輪は 1 だけ。区切りのケースは、N が尻尾ちょうど、
尻尾 + 1、輪を 1 周、1 周 + 1、2 周 - 1 のもの。
seed が 1000 以上なら、2^j 項ぶんの行き先と和を倍々で表にする愚直解で解ける入力を出す (pj testdata crosscheck 用)。
愚直解は N = 10^10 でも速いので、本番のケースも全部突き合わせられる。M を小さくして、輪の形が色々になるようにする。
"""

import random
import sys

MAX_N = 10**10
MAX_M = 10**5

SAMPLES = [(6, 2, 1001), (1000, 2, 16), (10**10, 10, 99959)]
FIXED = [
    (1, 0, 1),  # 最小
    (MAX_N, 0, 1),
    (MAX_N, 0, MAX_M),  # 答え 0
    (1, MAX_M - 1, MAX_M),  # 答えは X だけ
    (MAX_N, 1, 2),  # 答え N
    (MAX_N, 1, MAX_M),
    (MAX_N, MAX_M - 1, MAX_M),  # X の次から 1 が続く
    (MAX_N, 2, 2**16),  # 2, 4, 16, 256, 0, 0, ...
    (MAX_N, 2**16 - 1, 2**16),
    (MAX_N, 3, 2**16),  # 奇数は 1 に行き着く
    (MAX_N, 2, 99707),  # 輪の長さ 49852
    (MAX_N, 3, 65537),  # 尻尾 16 項
]
# (X, M, N の決め方)。N は尻尾の項数 t と輪の長さ c から決める。
BOUNDARY = [
    (2, 99707, "t"),
    (2, 99707, "t+1"),
    (2, 99707, "t+c"),
    (2, 99707, "t+c+1"),
    (2, 99707, "t+2c-1"),
    (3, 65537, "t"),
    (3, 65537, "t+1"),
    (10, 99959, "t+c"),
    (10, 99959, "t+c+1"),
]
RANDOM = 6
COUNT = len(SAMPLES) + len(FIXED) + len(BOUNDARY) + RANDOM


def rho(x: int, m: int) -> tuple[int, int]:
    """X = x から始めたときの (尻尾の項数, 輪の長さ)。"""
    seen: dict[int, int] = {}
    while x not in seen:
        seen[x] = len(seen)
        x = x * x % m
    return seen[x], len(seen) - seen[x]


def boundary(x: int, m: int, how: str) -> int:
    t, c = rho(x, m)
    return max(1, {"t": t, "t+1": t + 1, "t+c": t + c, "t+c+1": t + c + 1, "t+2c-1": t + 2 * c - 1}[how])


def random_case(rng: random.Random) -> tuple[int, int, int]:
    m = rng.choice([rng.randint(1, MAX_M), rng.randint(MAX_M // 2, MAX_M), 99707, 99991, 2**16])
    return rng.choice([MAX_N, rng.randint(1, MAX_N)]), rng.randrange(m), m


def small_case(rng: random.Random) -> tuple[int, int, int]:
    """M を小さめにして輪の形を色々にする。X と N は角の値を多めに混ぜる。"""
    m = rng.choice([rng.randint(1, 30), rng.randint(1, 1000), rng.randint(1, MAX_M), 2 ** rng.randint(0, 16)])
    x = rng.choice([0, 1, m - 1, rng.randrange(m)]) % m
    t, c = rho(x, m)
    n = rng.choice([
        rng.randint(1, 50),
        rng.randint(1, MAX_N),
        MAX_N,
        max(1, t + rng.randint(-2, 2)),
        max(1, t + c + rng.randint(-2, 2)),
        max(1, t + rng.randint(1, 5) * c + rng.randint(-1, 1)),
    ])
    return min(n, MAX_N), x, m


def case_for(seed: int, rng: random.Random) -> tuple[int, int, int]:
    if seed >= 1000:
        return small_case(rng)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    seed -= len(FIXED)
    if seed < len(BOUNDARY):
        x, m, how = BOUNDARY[seed]
        return boundary(x, m, how), x, m
    return random_case(rng)


def main() -> None:
    seed = int(sys.argv[1])
    n, x, m = case_for(seed, random.Random(seed))
    assert 1 <= n <= MAX_N and 0 <= x < m <= MAX_M
    print(n, x, m)


if __name__ == "__main__":
    main()
