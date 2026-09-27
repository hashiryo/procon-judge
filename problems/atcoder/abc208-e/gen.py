"""abc208-e (Digit Products) の入力を作る。N K を 1 行で出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、制約いっぱいのランダム。
角のケースは、N = 1、10 のべきとその 1 つ手前 (N = 10^18 だけ 19 桁になる)、1 が並んだ N、
途中に 0 が多い N、K = 1 (積が 0 か 1 の数だけ)、K が 9^9 や 2^29 や 9! のように桁の積で出る値など。
桁の積として出る値 (7-smooth な数) は 10^9 以下に 5194 個あり、K が大きいほど状態が増えて遅くなる。
seed が 1000 以上なら、1 から N まで数ごとに桁の積を求める愚直解で解ける小さい N を出す
(pj testdata crosscheck 用)。
"""

import random
import sys

MAX_N = 10**18
MAX_K = 10**9


def smooth_numbers() -> list[int]:
    """MAX_K 以下の 7-smooth な数 (桁の積として出る値)。"""
    out = [1]
    for p in (2, 3, 5, 7):
        out = [v * p**e for v in out for e in range(40) if v * p**e <= MAX_K]
    return sorted(set(out))


SMOOTH = smooth_numbers()

FIXED = [
    (13, 2),  # 例 1
    (100, 80),  # 例 2
    (10**18, 10**9),  # 例 3
    (1, 1),
    (1, MAX_K),
    (9, 1),
    (10, 1),
    (MAX_N, 1),
    (MAX_N - 1, 1),
    (MAX_N - 1, MAX_K),
    (111111111111111111, 1),  # 1 が 18 個
    (MAX_N, 9**9),  # 9 桁の 9 の積
    (MAX_N, 2**29),
    (123456789123456789, 362880),  # 9!
    (987654321987654321, MAX_K),
    (100000000000000001, MAX_K),  # 途中が 0 ばかり
    (10**17, MAX_K - 1),
    (99, 81),
]
RANDOM = 8


def random_case(rng: random.Random, kind: int) -> tuple[int, int]:
    n = rng.randint(10**17, MAX_N - 1) if kind != 3 else rng.randint(1, MAX_N)
    if kind == 0:
        return n, rng.randint(1, MAX_K)
    if kind == 1:
        return n, rng.randint(1, 100)
    if kind == 2:
        return n, rng.choice(SMOOTH[-500:])
    return n, rng.randint(MAX_K // 2, MAX_K)


def small_case(rng: random.Random) -> tuple[int, int]:
    """愚直解が 1 から N まで数えられる大きさ。K は小さい値、桁の積で出る値、大きい値を混ぜる。"""
    n = rng.choice([
        rng.randint(1, 100),
        rng.randint(1, 10**6),
        10 ** rng.randint(0, 6) - rng.randint(0, 1),
        int("1" * rng.randint(1, 6)),
    ])
    k = rng.choice([
        rng.randint(1, 10),
        rng.randint(1, 1000),
        rng.choice(SMOOTH[:200]),
        rng.randint(1, MAX_K),
    ])
    return max(1, n), k


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 1000:
        n, k = small_case(rng)
    elif seed < len(FIXED):
        n, k = FIXED[seed]
    else:
        n, k = random_case(rng, (seed - len(FIXED)) % 4)
    assert 1 <= n <= MAX_N and 1 <= k <= MAX_K
    print(n, k)


if __name__ == "__main__":
    main()
