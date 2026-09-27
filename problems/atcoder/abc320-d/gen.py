"""abc320-d (Relative Position) の入力を作る。N M と、M 個の A B X Y を出す。

人ごとに座標を決めておき、X と Y はその差にするので、情報は必ず矛盾しない。どちらの人から見た情報にするかと
情報の順番はランダムにする。
seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、小さいランダム、N か M が制約いっぱいのケース。
大きいケースは、ランダム (人 1 のいない成分も混ぜる)、パス (人 1 から遠い人の座標は 32 bit に収まらない)、スター、
同じ大きさの成分を 2 つずつつないでいくもの (UnionFind の木が一番高くなる)、少ない人に情報が何度も来るもの、M = 0 である。
seed が 1000 以上なら、人 1 から幅優先で座標を決める愚直解と突き合わせる小さい入力を出す (pj testdata crosscheck 用)。
"""

import itertools
import random
import sys

MAX_N = 2 * 10**5
MAX_M = 2 * 10**5
MAX_V = 10**9

SAMPLES = [
    (3, [(1, 2, 2, 1), (1, 3, -1, -2)]),
    (3, [(2, 1, -2, -1), (2, 3, -3, -3)]),
    (5, [(1, 2, 0, 0), (1, 2, 0, 0), (2, 3, 0, 0), (3, 1, 0, 0), (2, 1, 0, 0), (3, 2, 0, 0), (4, 5, 0, 0)]),
]
FIXED = [
    (1, []),
    (2, []),
    (2, [(2, 1, MAX_V, -MAX_V)]),  # 人 2 は (-10^9, 10^9)
    # 人 4 は (3 × 10^9, -3 × 10^9) で、32 bit に収まらない
    (4, [(1, 2, MAX_V, -MAX_V), (2, 3, MAX_V, -MAX_V), (3, 4, MAX_V, -MAX_V)]),
    # 人 1 だけ離れている。ほかの 4 人は互いの位置が決まっていても undecidable
    (5, [(2, 3, 1, 1), (3, 4, 1, 1), (4, 5, 1, 1), (5, 2, -3, -3)]),
    # 同じ情報を、向きを変えて何度も
    (3, [(1, 2, -MAX_V, MAX_V), (2, 1, MAX_V, -MAX_V), (1, 2, -MAX_V, MAX_V), (3, 2, 0, 0)]),
]
# 小さいランダム。(N, M)
SMALL = [(10, 8), (100, 150)]
PLANS = ["random", "path", "star", "binomial", "dense", "empty"]
COUNT = len(SAMPLES) + len(FIXED) + len(SMALL) + len(PLANS)


def make_info(rng: random.Random, coords: list[tuple[int, int]], pairs: list[tuple[int, int]]) -> list[tuple[int, int, int, int]]:
    """人の組ごとに、どちらから見るかをランダムに決めて A B X Y にする。番号は 1 から。"""
    info = []
    for u, v in pairs:
        if rng.random() < 0.5:
            u, v = v, u
        info.append((u + 1, v + 1, coords[v][0] - coords[u][0], coords[v][1] - coords[u][1]))
    return info


def random_coords(rng: random.Random, n: int, scale: int) -> list[tuple[int, int]]:
    return [(rng.randint(-scale, scale), rng.randint(-scale, scale)) for _ in range(n)]


def random_case(rng: random.Random, n: int, m: int, scale: int, big: int) -> list[tuple[int, int, int, int]]:
    """人 1 を含む big 人の成分と、残りを 1 人から 30 人の成分に分け、成分の中だけに情報を出す。"""
    others = rng.sample(range(1, n), n - 1)
    comps = [[0] + others[: big - 1]]
    rest = others[big - 1 :]
    while rest:
        k = rng.randint(1, 30)
        comps.append(rest[:k])
        rest = rest[k:]
    pairs = [(comp[rng.randrange(i)], comp[i]) for comp in comps for i in range(1, len(comp))]
    rng.shuffle(pairs)
    pairs = pairs[:m]
    # 残りは、同じ成分の中の組を何度でも。
    multi = [comp for comp in comps if len(comp) > 1]
    cum = list(itertools.accumulate(len(comp) for comp in multi))
    while multi and len(pairs) < m:
        comp = rng.choices(multi, cum_weights=cum)[0]
        u, v = rng.sample(comp, 2)
        pairs.append((u, v))
    rng.shuffle(pairs)
    return make_info(rng, random_coords(rng, n, scale), pairs)


def path_case(rng: random.Random, n: int, low: int, high: int, start: int) -> list[tuple[int, int, int, int]]:
    """人を 1 列に並べ、隣どうしの差を low 以上 high 以下にする。人 1 は列の start 番目。"""
    order = rng.sample(range(1, n), n - 1)
    order.insert(start, 0)
    coords = [(0, 0)] * n
    x = y = 0
    for v in order:
        coords[v] = (x, y)
        x += rng.randint(low, high)
        y -= rng.randint(low, high)
    pairs = [(order[i], order[i + 1]) for i in range(n - 1)]
    rng.shuffle(pairs)
    return make_info(rng, coords, pairs)


def binomial_case(rng: random.Random, k: int, scale: int) -> list[tuple[int, int, int, int]]:
    """2^k 人を、同じ大きさの成分どうしで 2 つずつつないでいく。大きさでつなぐ UnionFind の木の高さが k になる。"""
    n = 1 << k
    perm = rng.sample(range(n), n)
    pairs = []
    s = 1
    while s < n:
        step = [(perm[b + s - 1], perm[b + 2 * s - 1]) for b in range(0, n, 2 * s)]
        rng.shuffle(step)
        pairs += step
        s *= 2
    return make_info(rng, random_coords(rng, n, scale), pairs)


def plan_case(rng: random.Random, how: str) -> tuple[int, list[tuple[int, int, int, int]]]:
    if how == "random":
        return MAX_N, random_case(rng, MAX_N, MAX_M, MAX_V // 2, MAX_N * 7 // 10)
    if how == "path":
        # 隣どうしの差は 2 × 10^4 から 4 × 10^4 なので、人 1 から遠い人は 3 × 10^9 くらいになる。
        n = 10**5
        info = path_case(rng, n, 2 * 10**4, 4 * 10**4, 0)
        return n, info + [info[rng.randrange(len(info))]]
    if how == "star":
        n = 10**5
        center = rng.randrange(n)
        coords = random_coords(rng, n, 10**4)
        pairs = [(center, v) for v in range(n) if v != center]
        pairs.append(rng.choice(pairs))
        rng.shuffle(pairs)
        return n, make_info(rng, coords, pairs)
    if how == "binomial":
        return 1 << 17, binomial_case(rng, 17, 10**3)
    if how == "dense":
        # 1000 人に 2 × 10^5 個の情報。ほとんどが、もう分かっている位置の確かめ直しになる。
        n = 1000
        coords = random_coords(rng, n, 100)
        pairs = [tuple(rng.sample(range(n), 2)) for _ in range(MAX_M)]
        return n, make_info(rng, coords, pairs)
    return MAX_N, []  # empty


def small_case(rng: random.Random) -> tuple[int, list[tuple[int, int, int, int]]]:
    """愚直解の幅優先で解ける大きさ。パスでは、隣どうしの差を ±10^9 にして座標を 32 bit から出す。"""
    n = rng.randint(1, 12)
    if n >= 2 and rng.random() < 0.25:
        info = path_case(rng, n, -MAX_V, MAX_V, rng.randrange(n))
        info = rng.sample(info, rng.randint(0, len(info)))
        info += [rng.choice(info) for _ in range(rng.randint(0, 3))] if info else []
        return n, info
    m = rng.randint(0, 20)
    scale = rng.choice([0, 3, MAX_V // 2])
    return n, random_case(rng, n, m, scale, rng.randint(1, n))


def case_for(seed: int, rng: random.Random) -> tuple[int, list[tuple[int, int, int, int]]]:
    if seed >= 1000:
        return small_case(rng)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    seed -= len(FIXED)
    if seed < len(SMALL):
        n, m = SMALL[seed]
        return n, random_case(rng, n, m, MAX_V // 2, n // 2)
    return plan_case(rng, PLANS[seed - len(SMALL)])


def main() -> None:
    seed = int(sys.argv[1])
    n, info = case_for(seed, random.Random(seed))
    assert 1 <= n <= MAX_N and 0 <= len(info) <= MAX_M
    assert all(1 <= a <= n and 1 <= b <= n and a != b and abs(x) <= MAX_V and abs(y) <= MAX_V for a, b, x, y in info)
    out = [f"{n} {len(info)}"] + [f"{a} {b} {x} {y}" for a, b, x, y in info]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
