"""abc339-g (Smaller Sum) の入力を作る。N と A、Q と、暗号にした Q 個の α β γ を出す。

クエリ (L, R, X) を先に全部決め、答えを求めてから、1 つ前の答えとの xor で暗号にする。
答えは、クエリを X の小さい順に並べ、A を小さい順に Fenwick 木へ足していって求める。
seed が 0 から count - 1 までは本番のケース。例の 1 つ、角のケース、小さいランダム、N と Q が 2 × 10^5 前後のケース。
大きいケースは、ランダム、L が先頭の 700 個に R が末尾の 700 個にある広い区間ばかりのもの (700 ごとのバケットで
分ける平方分割では、ほぼ全部のバケットと端の半端を見るので遅い)、A が全部 10^9 で答えが 2 × 10^14 になるもの、
A と X が 0 から 2 だけで同じ値だらけのものである。
seed が 1000 以上なら、区間を 1 つずつ見る愚直解で解ける小さい入力を出す (pj testdata crosscheck 用)。
"""

import random
import sys

MAX_N = 2 * 10**5
MAX_Q = 2 * 10**5
MAX_A = 10**9
MAX_E = 10**18  # 暗号にした値の上限
BUCKET = 700  # SortedPerBucket のバケットの大きさ

# 例は、復号したクエリで持つ。暗号にすると問題文のとおりの入力になる。
SAMPLES = [
    ([2, 0, 2, 4, 0, 2, 0, 3], [(1, 8, 3), (3, 5, 2), (1, 1, 0), (3, 6, 5), (4, 8, 3)]),
]
FIXED = [
    ([0], [(1, 1, 0)]),
    ([MAX_A], [(1, 1, MAX_A - 1), (1, 1, MAX_A), (1, 1, 0), (1, 1, MAX_A)]),
    # X が A の値ちょうどのとき、その値を足す。
    ([5, 5, 5, 5, 5], [(1, 5, 4), (1, 5, 5), (2, 4, 6), (3, 3, 5), (5, 5, 4)]),
    ([0, MAX_A, 0, MAX_A, 1], [(1, 5, MAX_A), (2, 4, MAX_A - 1), (1, 5, 0), (4, 5, 1), (1, 4, MAX_A)]),
]
# 小さいランダム。(N, Q, A の上限)
SMALL = [(10, 10, 5), (1000, 1000, 100), (1000, 1000, 0)]
PLANS = ["random", "wide", "max", "ties"]
COUNT = len(SAMPLES) + len(FIXED) + len(SMALL) + len(PLANS)


def answers(a: list[int], queries: list[tuple[int, int, int]]) -> list[int]:
    """クエリを X の小さい順に並べ、A を小さい順に Fenwick 木へ足して答える。"""
    n = len(a)
    by_value = sorted(range(n), key=a.__getitem__)
    tree = [0] * (n + 1)
    out = [0] * len(queries)
    p = 0
    for k in sorted(range(len(queries)), key=lambda k: queries[k][2]):
        left, right, x = queries[k]
        while p < n and a[by_value[p]] <= x:
            i, v = by_value[p] + 1, a[by_value[p]]
            while i <= n:
                tree[i] += v
                i += i & -i
            p += 1
        s = 0
        i = right
        while i > 0:
            s += tree[i]
            i -= i & -i
        i = left - 1
        while i > 0:
            s -= tree[i]
            i -= i & -i
        out[k] = s
    return out


def random_range(rng: random.Random, n: int) -> tuple[int, int]:
    left, right = rng.randint(1, n), rng.randint(1, n)
    return min(left, right), max(left, right)


def random_case(rng: random.Random, n: int, q: int, max_a: int) -> tuple[list[int], list[tuple[int, int, int]]]:
    a = [rng.randint(0, max_a) for _ in range(n)]
    return a, [(*random_range(rng, n), rng.randint(0, max_a)) for _ in range(q)]


def plan_case(rng: random.Random, how: str) -> tuple[list[int], list[tuple[int, int, int]]]:
    if how == "random":
        return random_case(rng, MAX_N, MAX_Q, MAX_A)
    if how == "wide":
        a = [rng.randint(0, 1000) for _ in range(MAX_N)]
        queries = [
            (rng.randint(1, BUCKET), rng.randint(MAX_N - BUCKET + 1, MAX_N), rng.randint(0, 1000)) for _ in range(MAX_Q)
        ]
        return a, queries
    if how == "max":
        n = MAX_N
        return [MAX_A] * n, [(1, n, MAX_A), (1, n, MAX_A - 1), (1, n, MAX_A), (2, n - 1, MAX_A), (1, 1, MAX_A)]
    return random_case(rng, MAX_N, MAX_Q // 2, 2)  # ties


def small_case(rng: random.Random) -> tuple[list[int], list[tuple[int, int, int]]]:
    """愚直解が区間を 1 つずつ見て解ける大きさ。X は A の値ちょうどとその前後を多めにする。"""
    n = rng.randint(1, 30) if rng.random() < 0.9 else rng.randint(31, 2000)
    q = rng.randint(1, 30) if rng.random() < 0.9 else rng.randint(31, 2000)
    how = rng.choice(["small", "small", "large", "equal", "zero"])
    if how == "small":
        a = [rng.randint(0, 3) for _ in range(n)]
    elif how == "large":
        a = [rng.randint(0, MAX_A) for _ in range(n)]
    elif how == "equal":
        a = [rng.choice([1, MAX_A, rng.randint(0, MAX_A)])] * n
    else:
        a = [0] * n

    def pick_x() -> int:
        v = rng.choice(a) + rng.choice([-1, 0, 0, 1])
        return min(MAX_A, max(0, rng.choice([v, v, 0, MAX_A, rng.randint(0, MAX_A)])))

    return a, [(*random_range(rng, n), pick_x()) for _ in range(q)]


def case_for(seed: int, rng: random.Random) -> tuple[list[int], list[tuple[int, int, int]]]:
    if seed >= 1000:
        return small_case(rng)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    seed -= len(FIXED)
    if seed < len(SMALL):
        return random_case(rng, *SMALL[seed])
    return plan_case(rng, PLANS[seed - len(SMALL)])


def main() -> None:
    seed = int(sys.argv[1])
    a, queries = case_for(seed, random.Random(seed))
    n, q = len(a), len(queries)
    assert 1 <= n <= MAX_N and 1 <= q <= MAX_Q and all(0 <= v <= MAX_A for v in a)
    assert all(1 <= left <= right <= n and 0 <= x <= MAX_A for left, right, x in queries)
    out = [str(n), " ".join(map(str, a)), str(q)]
    prev = 0
    for (left, right, x), ans in zip(queries, answers(a, queries)):
        e = (left ^ prev, right ^ prev, x ^ prev)
        assert all(0 <= v <= MAX_E for v in e)
        out.append(f"{e[0]} {e[1]} {e[2]}")
        prev = ans
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
