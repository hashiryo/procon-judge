"""abc194-e (Mex Min) の入力を作る。N M と A_1 ... A_N を出す。答えは長さ M の区間の mex の最小。

seed が 0 から count - 1 までは本番のケース。例の 4 つ、角のケース、N = 1.5 × 10^6 などの大きいケース。
角のケースは、N = 1、M = 1、M = N (区間が 1 つ)、答えが 0、答えが N (A が順列で M = N)。
大きいケースは、全部 0、周期 M の列 (どの区間にも 0 から M - 1 が 1 つずつあり、答えは M)、周期 M の列の
1 か所だけ別の値にしたもの (答えはその位置の元の値)、少ない種類の値のランダム (M を、どこかの区間で
値が欠けるかどうかのきわどい長さにする)、小さい値と大きい値を混ぜたランダム。周期 M の列では、区間を
1 つずらすたびに、出ていく値と入ってくる値が同じになる。RangeSet は区間を割ってからすぐ繋ぐことになる。
入力が大きい (値が 7 桁だと 1 ケース 11 MB になる) ので、N = 1.5 × 10^6 は 3 ケースにする。
seed が 1000 以上なら、区間ごとに mex を数える愚直解で解ける小さい入力を出す (pj testdata crosscheck 用)。
2000 以上なら、愚直解でまだ解ける N = 3000 までの入力を出す。
"""

import random
import sys

MAX_N = 1500000

Case = tuple[int, list[int]]

SAMPLES: list[Case] = [
    (2, [0, 0, 1]),
    (2, [1, 1, 1]),
    (2, [0, 1, 0]),
    (3, [0, 0, 1, 2, 0, 1, 0]),
]
FIXED: list[Case] = [
    (1, [0]),
    (1, [1, 0]),
    (1, [0, 0]),
    (2, [1, 0]),  # 区間が 1 つで、答えは N
    (2, [1, 1]),
    (5, [3, 1, 4, 0, 2]),
    (3, [0, 1, 2, 0, 1, 2]),
    (4, [2, 0, 1, 3, 0, 5]),
]
# (N, M, 値の作り方, 値の種類)。本番のケースのうち角のケースのあとに並べる。
# zeros は全部 0、periodic は 0 から M - 1 の順列を繰り返したもの、broken はそれの 1 か所を M 以上の値に
# 変えたもの、random は 0 から K - 1 の一様なランダム、mixed は半分を 0 から K - 1、残りを 0 から N - 1 に
# したもの、perm は 0 から N - 1 の順列。M = 0 は M = N の意味。
PLANS = [
    (MAX_N, 1, "zeros", 0),
    (100000, 0, "zeros", 0),
    (MAX_N, 750000, "periodic", 0),
    (500000, 1000, "broken", 0),
    (MAX_N, 130, "random", 10),  # 長さ 130 の区間で 10 種類のどれかが欠けるかがきわどい
    (500000, 160, "random", 10),
    (500000, 3, "random", 3),
    (10**6, 450000, "mixed", 20),  # RangeSet の区間が最も多くなる
    (200000, 0, "perm", 0),  # 答えは N
    (200000, 100000, "perm", 0),
    (200000, 199999, "sorted", 0),
    (200000, 50000, "mixed", 5),
]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)


def build(rng: random.Random, n: int, m: int, how: str, k: int) -> list[int]:
    if how == "zeros":
        return [0] * n
    if how == "sorted":
        return list(range(n))
    if how == "perm":
        a = list(range(n))
        rng.shuffle(a)
        return a
    if how in ("periodic", "broken"):
        block = list(range(m))
        rng.shuffle(block)
        a = [block[i % m] for i in range(n)]
        if how == "broken":
            p = rng.randrange(n)
            a[p] = rng.randrange(m, n)
        return a
    if how == "random":
        return [rng.randrange(k) for _ in range(n)]
    assert how == "mixed"
    return [rng.randrange(k) if rng.random() < 0.5 else rng.randrange(n) for _ in range(n)]


def small_case(rng: random.Random, n: int) -> Case:
    m = rng.choice([1, n, rng.randint(1, n), rng.randint(1, n)])
    how = rng.choice(["zeros", "sorted", "perm", "periodic", "broken", "random", "random", "mixed"])
    if how in ("periodic", "broken"):
        # 周期は M でも、M より短くてもよい。
        period = rng.randint(1, m)
        a = build(rng, n, period, "periodic", 0)
        if how == "broken" and period < n:
            a[rng.randrange(n)] = rng.randrange(period, n)
        return m, a
    return m, build(rng, n, m, how, rng.randint(1, n))


def case_for(seed: int, rng: random.Random) -> Case:
    if seed >= 1000:
        return small_case(rng, rng.randint(1000, 3000) if seed >= 2000 else rng.randint(1, rng.choice([8, 30])))
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    n, m, how, k = PLANS[seed - len(FIXED)]
    return m or n, build(rng, n, m or n, how, k)


def main() -> None:
    seed = int(sys.argv[1])
    m, a = case_for(seed, random.Random(seed))
    n = len(a)
    assert 1 <= m <= n <= MAX_N and all(0 <= v < n for v in a)
    sys.stdout.write(f"{n} {m}\n" + " ".join(map(str, a)) + "\n")


if __name__ == "__main__":
    main()
