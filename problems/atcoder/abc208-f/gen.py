"""abc208-f (Cumulative Sum) の入力を作る。N M K を 1 行で出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、K = 2.5 × 10^6 と N が大きいランダム。
f(n, M) は n の K + M 次の多項式なので、提出は n = 0 から K + M までの値を作って N で補間する。
N がその範囲の中なら表を引くだけになるので、N = K + M と K + M + 1 の境目を入れる。
また P = 10^9 + 7 を法にすると多項式の値は N mod P だけで決まるので、N が P の倍数 (答えは 0) や、
N mod P が小さいもの (表を引く方に行く) も入れる。10^18 も P で割ると 49 なので表を引く方に行く。
補間する方は、N mod P が大きいランダムな N と、P - 1、10^18 - 10^6 で試す。
seed が 1000 以上なら、f(n, m) の表を n = 0 から N まで作る愚直解で解ける小さい N の入力を出す
(pj testdata crosscheck 用)。2000 以上なら、愚直解でまだ解ける N = 10^6 までの入力を出す。
"""

import random
import sys

P = 10**9 + 7
MAX_N = 10**18
MAX_M = 30
MAX_K = 2_500_000

SAMPLES = [
    (3, 4, 2),
    (0, 1, 2),
    (MAX_N, 30, 123456),
]
FIXED = [
    (0, 0, 1),
    (0, MAX_M, MAX_K),
    (1, 0, 1),
    (1, MAX_M, MAX_K),  # f(1, m) は m によらず 1
    (2, 0, MAX_K),
    (MAX_N, 0, MAX_K),
    (P, MAX_M, MAX_K),  # N mod P = 0 なので 0
    (MAX_N // P * P, MAX_M, MAX_K),
    (P - 1, MAX_M, MAX_K),
    (P + 5, MAX_M, 1000),  # f(5, 30) と同じ
    (MAX_K + MAX_M, MAX_M, MAX_K),  # 表の最後の点
    (MAX_K + MAX_M + 1, MAX_M, MAX_K),  # 表の外の最初の点
    (MAX_N, MAX_M, 1),
    (MAX_N, 1, MAX_K),
    (MAX_N, MAX_M, MAX_K),
    (MAX_N - 10**6, MAX_M, MAX_K - 1),  # N mod P = 999000056
    (10**6, MAX_M, 5),  # 愚直解でも解ける
]
RANDOM = 6


def random_case(rng: random.Random, i: int) -> tuple[int, int, int]:
    if i % 3 == 0:
        return rng.randint(0, MAX_N), MAX_M, MAX_K
    if i % 3 == 1:
        return rng.randint(0, MAX_N), rng.randint(0, MAX_M), rng.randint(1, MAX_K)
    # N mod P が小さい大きな N。
    return rng.randint(0, 10**5) + rng.randint(1, MAX_N // P - 1) * P, rng.randint(0, MAX_M), rng.randint(1, MAX_K)


def small_case(rng: random.Random, medium: bool) -> tuple[int, int, int]:
    """愚直解で解ける大きさ。K が小さいときは、N を K + M の前後に寄せたものも混ぜる。"""
    m = rng.choice([0, 1, 2, MAX_M, rng.randint(0, MAX_M)])
    if medium:
        k = rng.randint(1, 2 * 10**5)
        return rng.randint(k + m, 10**6), m, k
    if rng.random() < 0.2:
        # K が大きく N が小さい (表を引く方)。
        return rng.randint(0, 2000), m, rng.choice([MAX_K, rng.randint(1, MAX_K)])
    k = rng.choice([1, 2, rng.randint(1, 30), rng.randint(1, 1000)])
    if rng.random() < 0.3:
        n = max(0, k + m + rng.randint(-2, 2))
    else:
        n = rng.randint(0, 3000)
    return n, m, k


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 1000:
        n, m, k = small_case(rng, seed >= 2000)
    elif seed < len(SAMPLES):
        n, m, k = SAMPLES[seed]
    elif seed < len(SAMPLES) + len(FIXED):
        n, m, k = FIXED[seed - len(SAMPLES)]
    else:
        n, m, k = random_case(rng, seed - len(SAMPLES) - len(FIXED))
    assert 0 <= n <= MAX_N and 0 <= m <= MAX_M and 1 <= k <= MAX_K
    print(n, m, k)


if __name__ == "__main__":
    main()
