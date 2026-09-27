"""arc080-d (F - Prime Flip) の入力を作る。N と x_1 < ... < x_N を出す。

表のカードの並びを隣との差 (境目) で見ると、境目は表の連続した区間の左端と右端の次になり、最大 2N = 200 個。
提出は境目どうしを、差が奇素数なら 1 回、偶数なら 2 回、それ以外なら 3 回で結ぶ最小重み完全マッチングを解く。
seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース (N = 1、端のカード、連続した区間)、
N = 100 のいろいろな並び。並びは、10^7 までのランダム (境目が 200 個)、狭い範囲のランダム (区間が
混ざる)、1 つおき、差が全部奇素数、差が全部偶数、差が全部 1 でも素数でもない奇数、連続した区間を
いくつも並べたもの、10^7 に寄せたもの。
seed が 1000 以上なら、境目の間の最短の回数を幅優先探索で求め、境目の組み方をビット DP で全部試す愚直解で
解ける小さい入力を出す (pj testdata crosscheck 用)。2000 以上なら、愚直解でまだ解ける x = 5000 までの入力を出す。
"""

import random
import sys

MAX_N = 100
MAX_X = 10**7

SAMPLES = [
    [4, 5],
    [1, 2, 3, 4, 5, 6, 7, 8, 9],
    [1, MAX_X],
]
FIXED = [
    [1],  # 1 枚だけ裏返すのは 3 回
    [MAX_X],
    [1, 2, 3],  # p = 3 で 1 回
    [1, 3],
    [2, 3, 4, 5, 6],
    list(range(1, 101)),  # 境目が 1 と 101 で、差が偶数なので 2 回
    list(range(MAX_X - 99, MAX_X + 1)),
    list(range(1, 98)) + [200, 300, 400],
]
# (並びの出し方)。N = 100。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    "random",
    "random",
    "narrow",
    "narrow",
    "every_other",
    "odd_prime_gaps",
    "even_gaps",
    "odd_composite_gaps",
    "blocks",
    "top",
]


def is_prime(n: int) -> bool:
    if n < 2:
        return False
    i = 2
    while i * i <= n:
        if n % i == 0:
            return False
        i += 1
    return True


ODD_PRIMES = [p for p in range(3, 2000) if is_prime(p)]
ODD_COMPOSITES = [q for q in range(9, 2000, 2) if not is_prime(q)]


def from_gaps(rng: random.Random, gaps: list[int]) -> list[int]:
    """x_1 をランダムに決め、隣との差が gaps になる並びを作る。"""
    total = sum(gaps)
    x = [rng.randint(1, MAX_X - total)]
    for g in gaps:
        x.append(x[-1] + g)
    return x


def plan_x(rng: random.Random, how: str) -> list[int]:
    n = MAX_N
    if how == "random":
        return sorted(rng.sample(range(1, MAX_X + 1), n))
    if how == "narrow":
        return sorted(rng.sample(range(1, rng.randint(150, 400)), n))
    if how == "every_other":
        return from_gaps(rng, [2] * (n - 1))
    if how == "odd_prime_gaps":
        return from_gaps(rng, [rng.choice(ODD_PRIMES) for _ in range(n - 1)])
    if how == "even_gaps":
        return from_gaps(rng, [2 * rng.randint(1, 1000) for _ in range(n - 1)])
    if how == "odd_composite_gaps":
        return from_gaps(rng, [rng.choice(ODD_COMPOSITES) for _ in range(n - 1)])
    if how == "blocks":
        # 長さがまちまちの連続した区間を、間を空けて並べる。
        x = []
        start = rng.randint(1, 1000)
        while len(x) < n:
            length = min(n - len(x), rng.randint(1, 12))
            x += range(start, start + length)
            start += length + rng.randint(1, 60)
        return x
    assert how == "top"
    return sorted(rng.sample(range(MAX_X - 300, MAX_X + 1), n))


def small_x(rng: random.Random, medium: bool) -> list[int]:
    """愚直解で解ける大きさ。境目が 22 個以下になるように N を小さくし、x の範囲も狭くする。"""
    n = rng.randint(1, 11 if medium else 8)
    hi = rng.randint(n, 5000 if medium else rng.choice([12, 30, 100, 300]))
    how = rng.random()
    if how < 0.3:
        # 連続した区間を混ぜる。
        x = set()
        while len(x) < n:
            s = rng.randint(1, hi)
            x.update(range(s, min(hi, s + rng.randint(0, 4)) + 1))
        return sorted(x)[:n]
    return sorted(rng.sample(range(1, hi + 1), n))


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 1000:
        x = small_x(rng, seed >= 2000)
    elif seed < len(SAMPLES):
        x = SAMPLES[seed]
    elif seed < len(SAMPLES) + len(FIXED):
        x = FIXED[seed - len(SAMPLES)]
    else:
        x = plan_x(rng, PLANS[seed - len(SAMPLES) - len(FIXED)])
    assert 1 <= len(x) <= MAX_N and all(1 <= v <= MAX_X for v in x) and x == sorted(set(x))
    print(len(x))
    print(*x)


if __name__ == "__main__":
    main()
