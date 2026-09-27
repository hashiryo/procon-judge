"""arc090-b (D - People on a Line) の入力を作る。
N M と、M 個の L R D を出す。人の組は、向きを変えても重ならない。

人ごとに座標を決めておき、D はその差にする。答えが No のケースは、もうつながっている 2 人の食い違う情報を
最後に 1 つ足す。食い違う情報は、D を本当の差から 1 ずらすか、左右を逆にしたものである。
seed が 0 から count - 1 までは本番のケース。例の 5 つ、角のケース、小さいランダム、N = 10^5 前後のケース。
大きいケースは、ランダム (Yes と No)、長いパス (列の先頭と後ろの方の人を結んで長い閉路を作り、D を合わせた Yes と
ずらした No)、スター、同じ大きさの成分を 2 つずつつないでいくもの (UnionFind の木が一番高くなる)、
1000 人に 2 × 10^5 個の情報、全員が同じ座標で最後の 1 つだけ D = 1 のもの、M = 0 である。
seed が 1000 以上なら、差の制約を Floyd–Warshall で解く愚直解で解ける小さい入力を出す (pj testdata crosscheck 用)。
"""

import random
import sys

MAX_N = 10**5
MAX_M = 2 * 10**5
MAX_D = 10**4

SAMPLES = [
    (3, [(1, 2, 1), (2, 3, 1), (1, 3, 2)]),
    (3, [(1, 2, 1), (2, 3, 1), (1, 3, 5)]),
    (4, [(2, 1, 1), (2, 3, 5), (3, 4, 2)]),
    (10, [(8, 7, 100), (7, 9, 100), (9, 8, 100)]),
    (100, []),
]
FIXED = [
    (1, []),
    (2, [(1, 2, MAX_D)]),
    (2, [(2, 1, 0)]),
    (3, [(1, 2, 0), (2, 3, 0), (3, 1, 0)]),
    (3, [(1, 2, 0), (2, 3, 0), (3, 1, 1)]),
    (3, [(1, 2, MAX_D), (2, 3, MAX_D), (1, 3, MAX_D)]),  # 1 と 3 の差は 2 × 10^4 のはず
]
# 小さいランダム。(N, M, 食い違う情報を足すか)
SMALL = [(5, 6, False), (100, 300, True), (100, 300, False)]
# N = 10^5 前後のケース。(形, 食い違う情報を足すか)
PLANS = [
    ("random", False),
    ("random", True),
    ("path", False),
    ("path", True),
    ("star", False),
    ("binomial", True),
    ("dense", False),
    ("dense", True),
    ("same", True),
    ("empty", False),
]
COUNT = len(SAMPLES) + len(FIXED) + len(SMALL) + len(PLANS)


def key(u: int, v: int) -> tuple[int, int]:
    return min(u, v), max(u, v)


def make_info(rng: random.Random, x: list[int], pairs: list[tuple[int, int]]) -> list[tuple[int, int, int]]:
    """座標の小さい方を L にして、D = x_R - x_L にする。同じ座標なら向きはランダム。番号は 1 から。"""
    info = []
    for u, v in pairs:
        if x[u] > x[v] or (x[u] == x[v] and rng.random() < 0.5):
            u, v = v, u
        info.append((u + 1, v + 1, x[v] - x[u]))
    return info


def wrong(rng: random.Random, x: list[int], u: int, v: int) -> tuple[int, int, int]:
    """u と v の食い違う情報。D を本当の差から 1 ずらすか、左右を逆にする。"""
    if x[u] > x[v]:
        u, v = v, u
    t = x[v] - x[u]
    shifted = [d for d in (t - 1, t + 1) if 0 <= d <= MAX_D]
    if shifted and (t == 0 or rng.random() < 0.7):
        return u + 1, v + 1, rng.choice(shifted)
    # 右の人を L にする。D >= 0 なので、t > 0 ならどの D も合わない。
    return v + 1, u + 1, rng.randint(0, MAX_D)


def distinct_pairs(rng: random.Random, n: int, m: int) -> list[tuple[int, int]]:
    """向きを変えても重ならない m 個の人の組。"""
    seen = set()
    while len(seen) < m:
        u, v = rng.randrange(n), rng.randrange(n)
        if u != v:
            seen.add(key(u, v))
    pairs = list(seen)
    rng.shuffle(pairs)
    return pairs


def add_wrong(rng: random.Random, x: list[int], info: list[tuple[int, int, int]], pairs: list[tuple[int, int]]) -> list[tuple[int, int, int]]:
    """pairs[0] の成分から、まだ組になっていない 2 人を選び、その食い違う情報を最後に足す。"""
    g = [[] for _ in range(len(x))]
    for u, v in pairs:
        g[u].append(v)
        g[v].append(u)
    comp, stack = {pairs[0][0]}, [pairs[0][0]]
    while stack:
        u = stack.pop()
        for v in g[u]:
            if v not in comp:
                comp.add(v)
                stack.append(v)
    used = {key(u, v) for u, v in pairs}
    members = list(comp)
    while True:
        u, v = rng.sample(members, 2)
        if key(u, v) not in used:
            return info + [wrong(rng, x, u, v)]


def random_case(rng: random.Random, n: int, m: int, bad: bool) -> list[tuple[int, int, int]]:
    """座標を 0 から 10^4 に散らすので、どの組の差も D の範囲に収まる。"""
    x = [rng.randint(0, MAX_D) for _ in range(n)]
    pairs = distinct_pairs(rng, n, m - bad)
    info = make_info(rng, x, pairs)
    return add_wrong(rng, x, info, pairs) if bad else info


def path_case(rng: random.Random, n: int, m: int, bad: bool) -> list[tuple[int, int, int]]:
    """人を 1 列に並べ、隣どうしの差を -50 から 50 にして、200 人以内の組の情報も足す。

    最後に、列の先頭と、座標の差が 10^4 未満の人のうち列の一番後ろにいる人の組を足す。
    bad なら食い違う情報にする。どちらにしても、確かめるには列のほぼ全部をたどることになる。
    """
    order = rng.sample(range(n), n)
    x = [0] * n
    for i in range(1, n):
        x[order[i]] = x[order[i - 1]] + rng.randint(-50, 50)
    low = min(x)
    x = [v - low for v in x]
    pairs = {key(order[i], order[i + 1]) for i in range(n - 1)}
    while len(pairs) < m - 1:
        i = rng.randrange(n - 2)
        j = min(n - 1, i + rng.randint(2, 200))
        pairs.add(key(order[i], order[j]))
    pairs = list(pairs)
    rng.shuffle(pairs)
    first = order[0]
    last = next(order[j] for j in range(n - 1, 200, -1) if abs(x[order[j]] - x[first]) < MAX_D)
    info = make_info(rng, x, pairs)
    return info + [wrong(rng, x, first, last) if bad else make_info(rng, x, [(first, last)])[0]]


def binomial_case(rng: random.Random, k: int, bad: bool) -> list[tuple[int, int, int]]:
    """2^k 人を、同じ大きさの成分どうしで 2 つずつつないでいく。大きさでつなぐ UnionFind の木の高さが k になる。"""
    n = 1 << k
    x = [rng.randint(0, MAX_D) for _ in range(n)]
    perm = rng.sample(range(n), n)
    pairs = []
    s = 1
    while s < n:
        step = [(perm[b + s - 1], perm[b + 2 * s - 1]) for b in range(0, n, 2 * s)]
        rng.shuffle(step)
        pairs += step
        s *= 2
    info = make_info(rng, x, pairs)
    return add_wrong(rng, x, info, pairs) if bad else info


def plan_case(rng: random.Random, how: str, bad: bool) -> tuple[int, list[tuple[int, int, int]]]:
    n = MAX_N
    if how == "random":
        return n, random_case(rng, n, MAX_M, bad)
    if how == "path":
        return n, path_case(rng, n, MAX_M, bad)
    if how == "star":
        center = rng.randrange(n)
        x = [rng.randint(0, MAX_D) for _ in range(n)]
        return n, make_info(rng, x, [(center, v) for v in range(n) if v != center])
    if how == "binomial":
        return 1 << 16, binomial_case(rng, 16, bad)
    if how == "dense":
        return 1000, random_case(rng, 1000, MAX_M, bad)
    if how == "same":
        # 全員が同じ座標なので D = 0 ばかりで、食い違う情報は D = 1 になる。
        x = [0] * n
        pairs = distinct_pairs(rng, n, MAX_M - 1)
        return n, add_wrong(rng, x, make_info(rng, x, pairs), pairs)
    return n, []  # empty


def small_case(rng: random.Random) -> tuple[int, list[tuple[int, int, int]]]:
    """Floyd–Warshall で解ける大きさ。半分は、いくつかの情報を食い違うものに差し替える (閉路に乗っていなければ No にならない)。"""
    n = rng.randint(1, 10)
    m = rng.randint(0, n * (n - 1) // 2)
    width = rng.choice([0, 1, 3, MAX_D])
    x = [rng.randint(0, width) for _ in range(n)]
    pairs = distinct_pairs(rng, n, m)
    info = make_info(rng, x, pairs)
    if info and rng.random() < 0.5:
        for _ in range(rng.randint(1, 2)):
            k = rng.randrange(len(info))
            info[k] = wrong(rng, x, *pairs[k])
    return n, info


def case_for(seed: int, rng: random.Random) -> tuple[int, list[tuple[int, int, int]]]:
    if seed >= 1000:
        return small_case(rng)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    seed -= len(FIXED)
    if seed < len(SMALL):
        n, m, bad = SMALL[seed]
        return n, random_case(rng, n, m, bad)
    return plan_case(rng, *PLANS[seed - len(SMALL)])


def main() -> None:
    seed = int(sys.argv[1])
    n, info = case_for(seed, random.Random(seed))
    assert 1 <= n <= MAX_N and 0 <= len(info) <= MAX_M
    assert all(1 <= l <= n and 1 <= r <= n and l != r and 0 <= d <= MAX_D for l, r, d in info)
    assert len({key(l, r) for l, r, _ in info}) == len(info)
    out = [f"{n} {len(info)}"] + [f"{l} {r} {d}" for l, r, d in info]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
