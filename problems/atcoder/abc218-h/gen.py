"""abc218-h (Red and Blue Lamps) の入力を作る。N R と A_1 ... A_{N-1} を出す。
答えは、R 個を赤、残りを青にしたときの、色が変わる隣どうしの A の和の最大。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、N = 2 × 10^5 のケース。
角のケースは、N = 2、R = 1 と R = N - 1、A が 10^9 ばかりのもの。大きいケースは、R = 1、R = N - 1、
R = N / 2 (赤と青を交互に置けるので、答えは A の和)、N が奇数で R = (N - 1) / 2 (これも A の和)、
R = N / 2 - 1、ランダムな R。A は、ランダム、全部 10^9 (答えは 10^9 × min(2R, N - 1) で 32 ビットに
収まらない)、1 から 3 の同点だらけ、増える列、減る列、10^9 と 1 の交互、ほとんど 1 で所々大きいもの、
10^9 の近くに固まったもの。同点が多いと、Alien DP で探す罰金の関数に平らな所が多くなる。
seed が 1000 以上なら、(赤の数, 直前の色) を状態にする DP で解ける小さい入力を出す (pj testdata crosscheck 用)。
2000 以上なら、DP でまだ解ける N = 3000 までの入力を出す。
"""

import random
import sys

MAX_N = 2 * 10**5
MAX_A = 10**9

Case = tuple[int, list[int]]

SAMPLES: list[Case] = [
    (2, [3, 1, 4, 1, 5]),
    (6, [2, 7, 1, 8, 2, 8]),
    (7, [12345, 678, 90123, 45678901, 234567, 89012, 3456, 78901, 23456, 7890]),
]
FIXED: list[Case] = [
    (1, [1]),
    (1, [MAX_A]),
    (1, [1, 2]),
    (2, [5, 1]),
    (2, [1, 100, 1]),
    (2, [MAX_A] * 4),
    (1, [1, MAX_A, MAX_A, 1]),
    (3, [MAX_A, 1, MAX_A, 1, MAX_A, 1]),
]
# (N, R, A の作り方)。本番のケースのうち角のケースのあとに並べる。R = 0 はランダムな R の意味。
PLANS = [
    (MAX_N, 1, "random"),
    (MAX_N, MAX_N - 1, "random"),
    (MAX_N, MAX_N // 2, "random"),  # 答えは A の和
    (MAX_N - 1, (MAX_N - 1) // 2, "random"),  # N が奇数。答えは A の和
    (MAX_N, MAX_N // 2 - 1, "random"),
    (MAX_N, 0, "random"),
    (MAX_N, 0, "max"),
    (MAX_N, 0, "ties"),
    (MAX_N, 0, "increasing"),
    (MAX_N, 0, "decreasing"),
    (MAX_N, 0, "zigzag"),
    (MAX_N, 0, "sparse"),
    (MAX_N, 0, "near_max"),
    (MAX_N, MAX_N - 1000, "ties"),
]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)
HOWS = ["random", "max", "ties", "increasing", "decreasing", "zigzag", "sparse", "near_max"]


def values(rng: random.Random, n: int, how: str) -> list[int]:
    if how == "random":
        return [rng.randint(1, MAX_A) for _ in range(n - 1)]
    if how == "max":
        return [MAX_A] * (n - 1)
    if how == "ties":
        return [rng.randint(1, 3) for _ in range(n - 1)]
    if how in ("increasing", "decreasing"):
        a = sorted(rng.randint(1, MAX_A) for _ in range(n - 1))
        return a if how == "increasing" else a[::-1]
    if how == "zigzag":
        return [MAX_A if i % 2 == 0 else 1 for i in range(n - 1)]
    if how == "sparse":
        return [rng.randint(1, MAX_A) if rng.random() < 0.05 else 1 for _ in range(n - 1)]
    assert how == "near_max"
    return [MAX_A - rng.randint(0, 1000) for _ in range(n - 1)]


def case_for(seed: int, rng: random.Random) -> Case:
    if seed >= 1000:
        # (赤の数, 直前の色) の DP で解ける大きさ。
        n = rng.randint(100, 3000) if seed >= 2000 else rng.randint(2, 12)
        r = rng.choice([1, n - 1, n // 2, (n + 1) // 2, rng.randint(1, n - 1), rng.randint(1, n - 1)])
        return max(1, min(n - 1, r)), values(rng, n, rng.choice(HOWS))
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    n, r, how = PLANS[seed - len(FIXED)]
    return r or rng.randint(1, n - 1), values(rng, n, how)


def main() -> None:
    seed = int(sys.argv[1])
    r, a = case_for(seed, random.Random(seed))
    n = len(a) + 1
    assert 2 <= n <= MAX_N and 1 <= r <= n - 1 and all(1 <= v <= MAX_A for v in a)
    sys.stdout.write(f"{n} {r}\n" + " ".join(map(str, a)) + "\n")


if __name__ == "__main__":
    main()
