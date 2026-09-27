"""s8pc-3-g (Sum of Fibonacci Sequence) の入力を作る。n m を 1 行で出す。答えは d_{n,m} mod 998244353。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、大きいケース。
角のケースは、n = 1 (提出は n - 1 = 0 で配列の範囲外の b[-1] を読む。読んだ値は約分で消えるので答えは合う)、
m = 1 (答えは 1)、m = 2 (答えは n)、m < n (提出の sample_points_shift が標本点をそのまま返す)、m が P の倍数の近く。
提出は m - 1 を P で割った余りで多項式を評価するので、余りが n 未満になる m (m - 1 = P × t + j) と、
ちょうど n - 1 と n になる m を入れる。大きいケースは n = 2 × 10^5 と m = 10^18 の近くのランダム。
seed が 1000 以上なら、定義どおりに累積和を n - 1 回取る愚直解で解ける小さい入力を出す
(pj testdata crosscheck 用)。2000 以上なら、同じ愚直解でまだ解ける n m <= 3 × 10^7 の入力を出す。
"""

import random
import sys

P = 998244353
MAX_N = 2 * 10**5
MAX_M = 10**18

SAMPLES = [
    (4, 7),
    (12, 20),
    (16, 30),
]
FIXED = [
    (1, 1),
    (1, 2),
    (1, 3),
    (2, 1),
    (1, MAX_M),
    (2, MAX_M),
    (3, MAX_M),
    (MAX_N, 1),
    (MAX_N, 2),
    (MAX_N, 1000),  # m - 1 < n
    (1, P),
    (1, P + 1),
    (MAX_N, P + 1),  # m - 1 ≡ 0
    (MAX_N, 10**9 * P + 1),
    (MAX_N, 10**9 * P + MAX_N),  # m - 1 ≡ n - 1
    (MAX_N, 10**9 * P + MAX_N + 1),  # m - 1 ≡ n
    (MAX_N, 10**9 * P),  # m - 1 ≡ -1
    (MAX_N - 1, 10**9 * P - MAX_N),
    (3000, 3000),
    (MAX_N, MAX_N),
    (1000, MAX_M),
    (MAX_N, MAX_M),
    (MAX_N - 1, MAX_M - 1),
]
RANDOM = 5


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 2000:
        # 愚直解は O(n m)。n か m の片方を大きくする。
        n = rng.choice([rng.randint(1, 100), rng.randint(1, 3000), rng.randint(1, MAX_N)])
        m = rng.randint(1, max(1, 3 * 10**7 // n))
    elif seed >= 1000:
        n, m = rng.choice([
            (rng.randint(1, 40), rng.randint(1, 200)),
            (rng.randint(1, 3), rng.randint(1, 5000)),
            (rng.randint(1, MAX_N), rng.randint(1, 5)),
        ])
    elif seed < len(SAMPLES):
        n, m = SAMPLES[seed]
    elif seed < len(SAMPLES) + len(FIXED):
        n, m = FIXED[seed - len(SAMPLES)]
    else:
        n = rng.choice([MAX_N, rng.randint(1, MAX_N)])
        m = rng.randint(MAX_M // 2, MAX_M)
    assert 1 <= n <= MAX_N and 1 <= m <= MAX_M
    print(n, m)


if __name__ == "__main__":
    main()
