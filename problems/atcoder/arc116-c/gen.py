"""arc116-c (Multiple Sequences) の入力を作る。N M を 1 行で出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、ランダム。
答えは、M 以下の m ごとに、m を N 個の正の整数の積として並べる数を足したもの。
DirichletSeries::pow は、M / N > N (整数の割り算) なら 2 乗をくり返し、そうでなければ 2^n ≤ M となる
n 項までの二項展開で求めるので、その境目 (M = 2 × 10^5 で N = 446 と 447) と、n が変わる
M = 2^17 の前後を入れる。M = 1 は二項展開の項が 1 つだけになる。
seed が 1000 以上なら、列の長さを 1 つずつ伸ばす DP で解ける小さい入力を出す (pj testdata crosscheck 用)。
2000 以上なら、DP でまだ解ける N, M ≤ 3000 の入力を出す。
"""

import random
import sys

MAX = 2 * 10**5

FIXED = [
    (3, 4),  # 例 1
    (20, 30),  # 例 2
    (MAX, MAX),  # 例 3
    (1, 1),
    (1, MAX),  # 答えは M
    (MAX, 1),  # 答えは 1
    (2, 1),
    (1, 2),
    (2, 2),
    (MAX, 2),  # 答えは N + 1
    (2, MAX),  # 約数の個数の和
    (446, MAX),  # 2 乗をくり返すほうの最後
    (447, MAX),  # 二項展開のほうの最初
    (448, MAX),
    (MAX, 2**17 - 1),
    (MAX, 2**17),
    (MAX, 2**17 + 1),
    (17, 2**17),
    (MAX, 199999),  # 素数
    (MAX, 166320),  # 2^4 × 3^3 × 5 × 7 × 11。約数が多い
    (MAX, 447**2),
    (MAX - 1, MAX),
]
RANDOM = 8


def random_case(rng: random.Random, i: int) -> tuple[int, int]:
    kind = i % 4
    if kind == 0:
        return rng.randint(1, MAX), rng.randint(1, MAX)
    if kind == 1:
        # 2 乗をくり返すほう。
        m = rng.randint(MAX // 2, MAX)
        return rng.randint(1, int(m**0.5) - 1), m
    if kind == 2:
        # 二項展開のほう。
        m = rng.randint(MAX // 2, MAX)
        return rng.randint(int(m**0.5) + 1, MAX), m
    return rng.randint(1, MAX), rng.randint(1, 1000)


def small_case(rng: random.Random, limit_n: int, limit_m: int) -> tuple[int, int]:
    pick = lambda limit: rng.choice([1, 2, rng.randint(1, 10), rng.randint(1, limit), limit])
    n, m = pick(limit_n), pick(limit_m)
    if rng.random() < 0.2:
        # M / N と N が近い所。
        n = max(1, min(limit_n, int(m**0.5) + rng.randint(-2, 2)))
    return n, m


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 2000:
        n, m = small_case(rng, 3000, 3000)
    elif seed >= 1000:
        n, m = small_case(rng, 60, 300)
    elif seed < len(FIXED):
        n, m = FIXED[seed]
    else:
        n, m = random_case(rng, seed - len(FIXED))
    assert 1 <= n <= MAX and 1 <= m <= MAX
    print(n, m)


if __name__ == "__main__":
    main()
