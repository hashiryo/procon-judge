"""abc212-h (Nim Counting) の入力を作る。N K と A_1 ... A_K を出す。答えは、長さ 1 から N の列で xor が 0 でないものの数。

seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース、N = 2 × 10^5 のいろいろな A。
角のケースは、N = 1、K = 1 (提出が別に扱う。答えは長さが奇数の列の数)、A が 1 から 2^16 - 1 の全部など。
A の形は、全部 (K = 2^16 - 1)、ランダム、2 の冪 15 個 (アダマール変換の値が 15 - 2 × popcount になり、
提出が別に扱う値 1 が出る)、上半分 (2^15 から 2^16 - 1)、10 次元の部分空間の 0 以外の元全部 (xor で閉じる)、
1 から 63 の全部。N は偶数と奇数を混ぜる ((-1)^N で答えが変わる)。
seed が 1000 以上なら、長さごとに xor の値の個数を数える DP の愚直解で解ける小さい入力を出す
(pj testdata crosscheck 用)。2000 以上なら、同じ愚直解でまだ解ける N = 2000 までの入力を出す。
"""

import random
import sys

MAX_N = 2 * 10**5
MAX_A = 2**16 - 1

SAMPLES = [
    (2, [1, 2]),
    (100, [3, 5, 7]),
]
FIXED = [
    (1, [1]),
    (1, [MAX_A]),
    (2, [1]),
    (3, [1, 2, 3]),
    (1, list(range(1, MAX_A + 1))),
    (MAX_N, [1]),
    (MAX_N - 1, [MAX_A]),
]
# (N, A の形)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    (MAX_N, "all"),
    (MAX_N - 1, "all"),
    (MAX_N, "random_large"),
    (MAX_N - 1, "random_mid"),
    (MAX_N, "powers"),
    (MAX_N, "top_half"),
    (MAX_N, "subspace"),
    (MAX_N, "small_all"),
    (MAX_N - 1, "random_small"),
    (2, "random_large"),
    (12345, "random_mid"),
]


def values(rng: random.Random, how: str) -> list[int]:
    if how == "all":
        out = list(range(1, MAX_A + 1))
    elif how == "random_large":
        out = rng.sample(range(1, MAX_A + 1), rng.randint(30000, MAX_A - 1))
    elif how == "random_mid":
        out = rng.sample(range(1, MAX_A + 1), 1001)
    elif how == "random_small":
        out = rng.sample(range(1, 64), rng.randint(20, 50))
    elif how == "powers":
        out = [1 << b for b in range(15)]
    elif how == "top_half":
        out = list(range(1 << 15, MAX_A + 1))
    elif how == "subspace":
        # ランダムな 10 本の基底が張る空間の、0 以外の元全部。
        span = {0}
        while len(span) < 1 << 10:
            v = rng.randint(1, MAX_A)
            if v not in span:
                span |= {s ^ v for s in span}
        out = sorted(span - {0})
    else:
        assert how == "small_all"
        out = list(range(1, 64))
    rng.shuffle(out)
    return out


def small_case(rng: random.Random, seed: int) -> tuple[int, list[int]]:
    """愚直解の DP は O(N |span| K) (span は A の xor で作れる値の集合)。どちらの段でも収まる大きさにする。"""
    if seed >= 2000:
        top = rng.choice([16, 64, 256])
        return rng.randint(1, 2000), rng.sample(range(1, top), rng.randint(1, top - 1))
    how = rng.randrange(4)
    n = rng.randint(1, 30)
    if how == 0:
        return n, rng.sample(range(1, 8), rng.randint(1, 7))
    if how == 1:
        return n, rng.sample(range(1, 64), rng.randint(1, 20))
    if how == 2:
        # 大きい値を少し。
        return rng.randint(1, 8), rng.sample(range(1, MAX_A + 1), rng.randint(1, 4))
    return n, rng.sample([1 << b for b in range(6)] + [MAX_A & ((1 << 6) - 1)], rng.randint(1, 7))


def case_for(seed: int, rng: random.Random) -> tuple[int, list[int]]:
    if seed >= 1000:
        return small_case(rng, seed)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    n, how = PLANS[seed - len(FIXED)]
    return n, values(rng, how)


def main() -> None:
    seed = int(sys.argv[1])
    n, a = case_for(seed, random.Random(seed))
    k = len(a)
    assert 1 <= n <= MAX_N and 1 <= k <= MAX_A and len(set(a)) == k and all(1 <= v <= MAX_A for v in a)
    sys.stdout.write(f"{n} {k}\n{' '.join(map(str, a))}\n")


if __name__ == "__main__":
    main()
