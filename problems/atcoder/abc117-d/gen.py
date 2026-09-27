"""abc117-d (XXOR) の入力を作る。N K と A を出す。答えは 0 <= X <= K での (X xor A_1) + ... + (X xor A_N) の最大。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、N = 10^5 のランダム。
角のケースは、K = 0、A が全部 0 か全部 10^12、K が 2^39 の前後、どのビットも 0 と 1 が同じ数だけあるもの
(X のそのビットはどちらでも同じ) など。ランダムは、A と K を一様に選ぶもの、ビットごとの 1 の割合を変えたもの、
A を小さくするか 1 のビットを少なくして、X のビットを全部立てたくなるようにしたもの (K で抑えられる)、
K を小さくしたものである。
seed が 1000 以上なら、X を 0 から K まで全部試す愚直解で解ける小さい K の入力を出す (pj testdata crosscheck 用)。
2000 以上なら、K を 2^22 までにした入力を出す。
"""

import random
import sys

MAX_N = 10**5
MAX_V = 10**12
BITS = 40  # 10^12 < 2^40

SAMPLES = [
    (7, [1, 6, 3]),
    (9, [7, 4, 0, 3]),
    (0, [MAX_V]),
]
# (N, K の出し方, A の出し方)。本番のケースのうち例のあとに並べる。
PLANS = [
    (1, "zero", "zero"),  # 答えは 0
    (1, "max", "zero"),  # X = K で 10^12
    (1, "max", "max"),
    (2, "one", "tie"),
    (1000, "random", "biased"),
    (MAX_N, "zero", "max"),  # X = 0 しか選べず、答えは 10^17
    (MAX_N, "max", "zero"),  # X = K で 10^17
    (MAX_N, "max", "max"),
    (MAX_N, "pow-1", "random"),  # K = 2^39 - 1
    (MAX_N, "pow", "random"),  # K = 2^39
    (MAX_N, "max", "tie"),
    (MAX_N, "random", "random"),
    (MAX_N, "random", "biased"),
    (MAX_N, "random", "small"),
    (MAX_N, "random", "sparse"),
    (MAX_N, "small", "random"),
]
COUNT = len(SAMPLES) + len(PLANS)
HOWS = ["random", "biased", "small", "sparse", "tie", "zero", "max", "equal"]


def pick_k(rng: random.Random, how: str, top: int) -> int:
    if how == "zero":
        return 0
    if how == "max":
        return top
    if how == "one":
        return 1
    if how == "pow-1":
        return 2**39 - 1
    if how == "pow":
        return 2**39
    if how == "small":
        return rng.randint(0, 1000)
    assert how == "random"
    return rng.randint(0, top)


def pick_a(rng: random.Random, how: str, n: int) -> list[int]:
    if how == "random":
        return [rng.randint(0, MAX_V) for _ in range(n)]
    if how == "zero":
        return [0] * n
    if how == "max":
        return [MAX_V] * n
    if how == "equal":
        return [rng.randint(0, MAX_V)] * n
    if how == "small":
        return [rng.randrange(2 ** rng.randint(1, 20)) for _ in range(n)]
    if how == "tie":
        # 前半と後半で 39 ビットを反転した組にする。下の 39 ビットはどれも 0 と 1 が同じ数になる。
        half = [rng.randrange(2**39) for _ in range(n // 2)]
        a = half + [(2**39 - 1) ^ v for v in half] + [rng.randint(0, MAX_V)] * (n % 2)
        rng.shuffle(a)
        return a
    # ビットごとに 1 になる割合を決める。sparse は 1 が少ないので、X のビットを全部立てたくなる。
    if how == "biased":
        rate = [rng.choice([0.0, 0.1, 0.3, 0.5, 0.7, 0.9, 1.0]) for _ in range(BITS)]
    else:
        assert how == "sparse"
        rate = [0.05] * BITS
    a = []
    for _ in range(n):
        v = sum(1 << b for b in range(BITS) if rng.random() < rate[b])
        a.append(v if v <= MAX_V else v & (2**39 - 1))
    return a


def case_for(seed: int, rng: random.Random) -> tuple[int, list[int]]:
    if seed >= 2000:
        # 愚直解が X を 2^22 通りまで試せる大きさ。
        n, how_a = rng.randint(1, 20), rng.choice(HOWS)
        k = rng.choice([rng.randint(2**15, 2**22), 2 ** rng.randint(15, 22) - rng.randint(0, 1)])
        return k, pick_a(rng, how_a, n)
    if seed >= 1000:
        # 愚直解が X を全部試せる大きさ。K は 2 のべきの前後と 0 を多めにする。
        n, how_a = rng.randint(1, 30), rng.choice(HOWS)
        k = rng.choice([0, rng.randint(0, 2000), 2 ** rng.randint(0, 10) - rng.randint(0, 1)])
        return k, pick_a(rng, how_a, n)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    n, how_k, how_a = PLANS[seed - len(SAMPLES)]
    return pick_k(rng, how_k, MAX_V), pick_a(rng, how_a, n)


def main() -> None:
    seed = int(sys.argv[1])
    k, a = case_for(seed, random.Random(seed))
    n = len(a)
    assert 1 <= n <= MAX_N and 0 <= k <= MAX_V and all(0 <= v <= MAX_V for v in a)
    sys.stdout.write(f"{n} {k}\n" + " ".join(map(str, a)) + "\n")


if __name__ == "__main__":
    main()
