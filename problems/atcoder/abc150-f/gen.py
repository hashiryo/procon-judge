"""abc150-f (Xor Shift) の入力を作る。N と、数列 a と b を出す。答えは a' = b になる (k, x) の全部。

seed が 0 から count - 1 までは本番のケース。例の 4 つ、角のケース、N = 2 × 10^5 と 5 × 10^4 のいろいろな数列。
b は、a を回して x を xor したもの (答えがある)、それを 1 か所だけ変えたもの (答えが無い)、ランダムを混ぜる。
周期のある a では答えが周期ごとに出る。全部同じ値や、2 つの値が交互に並ぶ a なら、答えは N 個になる。
a_i = i (N = 2^17) なら答えは k = 0 と k = 2^16 の 2 つ (i + 2^16 を 2^17 で割った余りは i xor 2^16)。
出力が N 行になるケースは大きいので、N = 2 × 10^5 は 5 ケースにする。
seed が 1000 以上なら、k ごとに全部の i を確かめる愚直解で解ける小さい入力を出す (pj testdata crosscheck 用)。
2000 以上なら、愚直解でまだ解ける N = 2000 までの入力を出す。
"""

import random
import sys

MAX_N = 2 * 10**5
MAX_V = 2**30 - 1

SAMPLES = [
    ([0, 2, 1], [1, 2, 3]),
    ([0, 0, 0, 0, 0], [2, 2, 2, 2, 2]),
    ([0, 1, 3, 7, 6, 4], [1, 5, 4, 6, 2, 3]),
    ([1, 2], [0, 0]),
]
FIXED = [
    ([0], [0]),
    ([MAX_V], [0]),
    ([1, 2], [2, 1]),  # (0, 3) と (1, 0)
    ([0, MAX_V], [0, 0]),
    ([5, 5, 5], [5, 5, 5]),
    ([0, 1, 0, 1], [1, 0, 1, 0]),
    ([MAX_V, 0, 5], [MAX_V, MAX_V ^ 5, 0]),  # (1, 2^30 - 1) だけ
]
# (N, a の作り方, b の作り方)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    (MAX_N, "zero", "same"),  # 答えは N 個
    (MAX_N, "random", "rotate"),
    (50000, "random", "random"),
    (MAX_N, "period1000", "rotate"),  # 答えは 200 個
    (MAX_N, "alternate", "rotate"),  # 答えは N 個
    (2**17, "iota", "rotate"),
    (50000, "max", "zero"),  # 答えは N 個で、x = 2^30 - 1
    (50000, "random", "near"),
    (50000, "bits", "rotate"),
    (50000, "period1000", "near"),
    (50000, "thue_morse", "rotate"),
    (49999, "random", "rotate"),
    (49998, "period3", "rotate"),
    (50000, "top", "rotate"),
]


def make_a(rng: random.Random, n: int, how: str) -> list[int]:
    if how == "zero":
        return [0] * n
    if how == "max":
        return [MAX_V] * n
    if how == "random":
        return [rng.randint(0, MAX_V) for _ in range(n)]
    if how.startswith("period"):
        p = int(how[len("period"):])
        base = [rng.randint(0, MAX_V) for _ in range(p)]
        return [base[i % p] for i in range(n)]
    if how == "alternate":
        u, v = rng.randint(0, MAX_V), rng.randint(0, MAX_V)
        return [u if i % 2 == 0 else v for i in range(n)]
    if how == "iota":
        return list(range(n))
    if how == "bits":
        return [rng.randint(0, 1) for _ in range(n)]
    if how == "thue_morse":
        return [(bin(i).count("1") % 2) << 29 for i in range(n)]
    assert how == "top"
    return [MAX_V - rng.randint(0, 3) for _ in range(n)]


def make_b(rng: random.Random, a: list[int], how: str) -> list[int]:
    n = len(a)
    if how == "same":
        return a[:]
    if how == "zero":
        return [0] * n
    if how == "random":
        return [rng.randint(0, MAX_V) for _ in range(n)]
    k, x = rng.randrange(n), rng.randint(0, MAX_V)
    b = [a[(i + k) % n] ^ x for i in range(n)]
    if how == "near":
        i = rng.randrange(n)
        b[i] ^= 1 << rng.randrange(30)
    else:
        assert how == "rotate"
    return b


def small_case(rng: random.Random, n: int) -> tuple[list[int], list[int]]:
    """愚直解が O(N^2) で解ける大きさ。値の種類を少なくして、答えが出やすくする。"""
    values = rng.choice([
        [0, 1],
        [0, 1, 2, 3],
        [0, MAX_V],
        [rng.randint(0, MAX_V) for _ in range(3)],
        None,
    ])
    divisors = [p for p in range(1, n + 1) if n % p == 0]
    period = rng.choice(divisors) if rng.random() < 0.4 else n
    base = [rng.choice(values) if values else rng.randint(0, MAX_V) for _ in range(period)]
    a = [base[i % period] for i in range(n)]
    b = make_b(rng, a, rng.choice(["rotate", "rotate", "near", "random", "same"]))
    if values and rng.random() < 0.3:
        b = [rng.choice(values) for _ in range(n)]
    return a, b


def case_for(seed: int, rng: random.Random) -> tuple[list[int], list[int]]:
    if seed >= 1000:
        return small_case(rng, rng.randint(100, 2000) if seed >= 2000 else rng.randint(1, 30))
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    n, how_a, how_b = PLANS[seed - len(FIXED)]
    a = make_a(rng, n, how_a)
    return a, make_b(rng, a, how_b)


def main() -> None:
    seed = int(sys.argv[1])
    a, b = case_for(seed, random.Random(seed))
    n = len(a)
    assert 1 <= n <= MAX_N and len(b) == n
    assert all(0 <= v <= MAX_V for v in a + b)
    sys.stdout.write(f"{n}\n{' '.join(map(str, a))}\n{' '.join(map(str, b))}\n")


if __name__ == "__main__":
    main()
