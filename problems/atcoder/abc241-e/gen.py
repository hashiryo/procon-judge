"""abc241-e (Putting Candies) の入力を作る。N K と A を出す。答えは K 回の操作のあとの飴の数。

X mod N は i から (i + A_i) mod N へ移るので、0 から始めた移り方は、尻尾のあとに輪に入る形 (rho) になる。
seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース、N = 2 × 10^5 のケース。
角のケースは、N = 2、K = 1、答えが 10^18 になるもの (A が全部 10^6 で、X mod N がずっと 0)、全部 1 (長さ N の輪)。
大きいケースは、ランダム、A が 1 から 3 のもの、長い尻尾と長い輪 (尻尾と輪の長さ、その前後の K も入れる)、
長い尻尾と自己ループ、A が全部 N の倍数 (0 の自己ループ) である。長い尻尾は、A_i を N で割った余りを 1 にして作る。
seed が 1000 以上なら、K 回そのまま操作する愚直解で解ける小さい K の入力を出す (pj testdata crosscheck 用)。
2000 以上なら、N を 2 × 10^5 まで、K を 10^8 までにした入力を出す。
"""

import random
import sys

MAX_N = 2 * 10**5
MAX_K = 10**12
MAX_A = 10**6

SAMPLES = [
    (3, [2, 1, 6, 3, 1]),
    (MAX_K, [260522, 914575, 436426, 979445, 648772, 690081, 933447, 190629, 703497, 47202]),
]
# (N, K, A の出し方, 尻尾の長さ)。K が負なら、尻尾と輪の長さから決める (-1: 尻尾ちょうど、-2: 尻尾 + 輪、-3: その 1 つ先)。
PLANS = [
    (2, 1, "ones", 0),
    (2, MAX_K, "max", 0),  # 答えは 10^18
    (MAX_N, MAX_K, "max", 0),  # 10^6 は N の倍数なので、ずっと A_0 を足して 10^18
    (MAX_N, MAX_K, "ones", 0),  # 長さ N の輪
    (MAX_N, 1, "random", 0),
    (MAX_N, MAX_K, "random", 0),
    (MAX_N, MAX_K, "tiny", 0),
    (MAX_N, MAX_K, "multiple", 0),
    (MAX_N, MAX_K, "rho", MAX_N // 2),
    (MAX_N, -1, "rho", MAX_N // 2),
    (MAX_N, -2, "rho", MAX_N // 2),
    (MAX_N, -3, "rho", MAX_N // 2),
    (MAX_N, MAX_K, "rho", MAX_N - 1),  # 尻尾のあとは自己ループ
    (MAX_N, MAX_K - 1, "rho", 1),
    (10, MAX_K, "random", 0),
    (1000, MAX_K, "tiny", 0),
]
COUNT = len(SAMPLES) + len(PLANS)
HOWS = ["ones", "max", "random", "tiny", "multiple", "rho"]


def pick_a(rng: random.Random, how: str, n: int, tail: int) -> list[int]:
    if how == "ones":
        return [1] * n
    if how == "max":
        return [MAX_A] * n
    if how == "random":
        return [rng.randint(1, MAX_A) for _ in range(n)]
    if how == "tiny":
        return [rng.randint(1, 3) for _ in range(n)]
    if how == "multiple":
        return [n * rng.randint(1, MAX_A // n) for _ in range(n)]
    # rho: 0, 1, ..., n - 1 と進み、n - 1 から tail へ戻る。A_i は N で割った余りだけ決めて、N の倍数を足す。
    assert how == "rho" and 0 <= tail < n

    def with_rest(r: int) -> int:
        return r + n * rng.randint(0 if r else 1, (MAX_A - r) // n)

    return [with_rest(1 % n) for _ in range(n - 1)] + [with_rest((tail + 1) % n)]


def case_for(seed: int, rng: random.Random) -> tuple[int, list[int]]:
    if seed >= 1000:
        # 愚直解が K 回そのまま操作できる大きさ。
        big = seed >= 2000
        n = rng.randint(2, MAX_N if big and rng.random() < 0.5 else 20)
        how = rng.choice(HOWS)
        a = pick_a(rng, how, n, rng.randrange(n))
        k = rng.randint(1, 10**8 if big else rng.choice([10, 2000]))
        return k, a
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    n, k, how, tail = PLANS[seed - len(SAMPLES)]
    if k < 0:
        k = {-1: tail, -2: n, -3: n + 1}[k]  # 輪の長さは n - tail
    return k, pick_a(rng, how, n, tail)


def main() -> None:
    seed = int(sys.argv[1])
    k, a = case_for(seed, random.Random(seed))
    n = len(a)
    assert 2 <= n <= MAX_N and 1 <= k <= MAX_K and all(1 <= v <= MAX_A for v in a)
    sys.stdout.write(f"{n} {k}\n" + " ".join(map(str, a)) + "\n")


if __name__ == "__main__":
    main()
