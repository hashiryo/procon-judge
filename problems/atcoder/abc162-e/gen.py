"""abc162-e (Sum of gcd of Tuples (Hard)) の入力を作る。N K を 1 行で出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、制約いっぱいのランダム。
角のケースは、N と K の最小と最大、K が素数、2 のべき、約数の多い数 (83160 は約数が 128 個) など。
seed が 1000 以上なら、小さい N と K を出す (pj testdata crosscheck 用)。愚直解は φ を使った和で
制約いっぱいでも解けるので、本番のケースもそのまま突き合わせられる。
"""

import random
import sys

MAX_N = 10**5
MAX_K = 10**5

FIXED = [
    (3, 2),  # 例 1
    (3, 200),  # 例 2
    (100000, 100000),  # 例 3
    (2, 1),
    (MAX_N, 1),
    (2, 2),
    (2, MAX_K),
    (MAX_N, 2),
    (3, MAX_K),
    (MAX_N, 3),
    (MAX_N, 99991),  # K が素数
    (MAX_N, 65536),  # K が 2 のべき
    (MAX_N, 83160),  # K の約数が多い
    (99999, 99999),
]
RANDOM = 6


def random_case(rng: random.Random, kind: int) -> tuple[int, int]:
    if kind == 0:
        return rng.randint(2, 20), rng.randint(MAX_K // 2, MAX_K)
    if kind == 1:
        return rng.randint(MAX_N // 2, MAX_N), rng.randint(2, 20)
    return rng.randint(MAX_N // 2, MAX_N), rng.randint(MAX_K // 2, MAX_K)


def small_case(rng: random.Random) -> tuple[int, int]:
    n = rng.choice([rng.randint(2, 6), rng.randint(2, 100), rng.randint(2, MAX_N)])
    k = rng.choice([rng.randint(1, 12), rng.randint(1, 1000), rng.choice([1, 2, 60, 64, 97, 720])])
    return n, k


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 1000:
        n, k = small_case(rng)
    elif seed < len(FIXED):
        n, k = FIXED[seed]
    else:
        n, k = random_case(rng, (seed - len(FIXED)) % 3)
    assert 2 <= n <= MAX_N and 1 <= k <= MAX_K
    print(n, k)


if __name__ == "__main__":
    main()
