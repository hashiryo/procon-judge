"""abc328-f (Good Set Query) の入力を作る。N Q と、Q 個の a b d を出す。

数列 X を 1 つ決めておき、d はたいてい X_a - X_b にして、残りはずらした値にする。ずらした式は、
a と b がまだつながっていなければ S に入り、つながっていれば入らない。
seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、小さいランダム、N か Q が制約いっぱいのケース。
大きいケースは、ランダム、N が小さくてほとんどがつながったあとの確かめになるもの、a = b だけのもの、
長いパスで差が 32 bit を超えるもの、同じ大きさの成分を 2 つずつつないでいくもの (UnionFind の木が一番高くなる) である。
長いパスには、差を 32 bit で計算すると一致してしまう d (差を 2^32 で割った余り) を混ぜる。
seed が 1000 以上なら、毎回 S ∪ {i} の X を幅優先で決め直す愚直解で解ける小さい入力を出す (pj testdata crosscheck 用)。
"""

import random
import sys

MAX_N = 2 * 10**5
MAX_Q = 2 * 10**5
MAX_D = 10**9

SAMPLES = [
    (3, [(1, 2, 2), (3, 2, -3), (2, 1, -1), (3, 3, 0), (1, 3, 5)]),
    (200000, [(1, 1, 1)]),
    (5, [
        (4, 2, 125421359), (2, 5, -191096267), (3, 4, -42422908), (3, 5, -180492387), (3, 3, 174861038),
        (2, 3, -82998451), (3, 4, -134761089), (3, 1, -57159320), (5, 2, 191096267), (2, 4, -120557647),
        (4, 2, 125421359), (2, 3, 142216401), (4, 5, -96172984), (3, 5, -108097816), (1, 5, -50938496),
        (1, 2, 140157771), (5, 4, 65674908), (4, 3, 35196193), (4, 4, 0), (3, 4, 188711840),
    ]),
]
FIXED = [
    (1, [(1, 1, 0)]),
    (1, [(1, 1, 0), (1, 1, 5), (1, 1, -MAX_D), (1, 1, 0)]),
    (2, [(1, 2, MAX_D), (2, 1, -MAX_D)]),
    (2, [(1, 2, MAX_D), (2, 1, MAX_D), (1, 2, -MAX_D), (1, 2, MAX_D)]),
    # X_1 - X_6 = 2^32 + 5、X_2 - X_6 = 2^32 - 999999995。32 bit で計算すると後ろの 3 つが一致してしまう。
    (6, [
        (1, 2, MAX_D), (2, 3, MAX_D), (3, 4, MAX_D), (4, 5, MAX_D), (5, 6, 2**32 + 5 - 4 * MAX_D),
        (1, 6, 5), (6, 1, -5), (2, 6, -999999995),
    ]),
    (3, [(1, 1, 0), (2, 2, 1), (3, 3, 0), (2, 2, -1)]),
]
# 小さいランダム。(N, Q)
SMALL = [(5, 10), (50, 100)]
PLANS = ["random", "few", "self", "trap", "binomial"]
COUNT = len(SAMPLES) + len(FIXED) + len(SMALL) + len(PLANS)


def wrap32(v: int) -> int:
    """v を 2^32 で割った余りを -2^31 以上 2^31 未満で表したもの。"""
    return (v + 2**31) % 2**32 - 2**31


def query(rng: random.Random, x: list[int], a: int, b: int, true_rate: float) -> tuple[int, int, int]:
    """X_a - X_b = d の式。true_rate の割合で正しい d にし、残りは 1 ずらすかランダムにする。"""
    d = x[a] - x[b]
    if rng.random() >= true_rate:
        d = d + rng.choice([-1, 1]) if rng.random() < 0.5 else rng.randint(-MAX_D, MAX_D)
        if abs(d) > MAX_D:
            d = rng.randint(-MAX_D, MAX_D)
    return a + 1, b + 1, d


def random_queries(rng: random.Random, n: int, q: int, scale: int, true_rate: float, self_rate: float) -> list[tuple[int, int, int]]:
    x = [rng.randint(-scale, scale) for _ in range(n)]
    out = []
    for _ in range(q):
        a = rng.randrange(n)
        b = a if n == 1 or rng.random() < self_rate else rng.randrange(n)
        out.append(query(rng, x, a, b, true_rate))
    return out


def trap_queries(rng: random.Random, n: int, q: int, step: int, signs: list[int]) -> list[tuple[int, int, int]]:
    """人を 1 列に並べ、隣どうしの差を step / 2 から step (符号は signs から選ぶ) にした式を先に全部出す。

    そのあとは、隣どうしの式の出し直しと、離れた 2 つの差を 32 bit で計算すると一致してしまう式を混ぜる。
    """
    order = rng.sample(range(n), n)
    x = [0] * n
    for i in range(1, n):
        x[order[i]] = x[order[i - 1]] + rng.choice(signs) * rng.randint(step // 2, step)
    path = [(order[i], order[i + 1]) for i in range(n - 1)]
    rng.shuffle(path)
    out = [(a + 1, b + 1, x[a] - x[b]) if rng.random() < 0.5 else (b + 1, a + 1, x[b] - x[a]) for a, b in path]
    while len(out) < q:
        if rng.random() < 0.5:
            a, b = rng.choice(path)
            out.append((a + 1, b + 1, x[a] - x[b]))
            continue
        a, b = rng.sample(range(n), 2)
        d = wrap32(x[a] - x[b])
        if abs(d) <= MAX_D and d != x[a] - x[b]:
            out.append((a + 1, b + 1, d))
    return out


def binomial_queries(rng: random.Random, k: int, q: int, scale: int) -> list[tuple[int, int, int]]:
    """2^k 個を、同じ大きさの成分どうしで 2 つずつつなぐ式を先に出す。大きさでつなぐ UnionFind の木の高さが k になる。"""
    n = 1 << k
    x = [rng.randint(-scale, scale) for _ in range(n)]
    perm = rng.sample(range(n), n)
    out = []
    s = 1
    while s < n:
        step = [(perm[b + s - 1], perm[b + 2 * s - 1]) for b in range(0, n, 2 * s)]
        rng.shuffle(step)
        out += [query(rng, x, a, b, 1.0) if rng.random() < 0.5 else query(rng, x, b, a, 1.0) for a, b in step]
        s *= 2
    while len(out) < q:
        out.append(query(rng, x, rng.randrange(n), rng.randrange(n), 0.5))
    return out


def plan_case(rng: random.Random, how: str) -> tuple[int, list[tuple[int, int, int]]]:
    if how == "random":
        return MAX_N, random_queries(rng, MAX_N, MAX_Q, MAX_D // 2, 0.7, 0.01)
    if how == "few":
        return 10, random_queries(rng, 10, MAX_Q, MAX_D // 2, 0.7, 0.05)
    if how == "self":
        return MAX_N, random_queries(rng, MAX_N, MAX_Q, 0, 0.5, 1.0)
    if how == "trap":
        return 10**5, trap_queries(rng, 10**5, MAX_Q, MAX_D, [-1, 1])
    return 1 << 17, binomial_queries(rng, 17, MAX_Q, 10**4)


def small_case(rng: random.Random) -> tuple[int, list[tuple[int, int, int]]]:
    """愚直解が毎回はじめから解ける大きさ。4 回に 1 回は、差が 32 bit を超えるパス (差が同じ向きに増える) にする。"""
    n = rng.randint(1, 8)
    q = rng.randint(1, 25)
    if n >= 4 and rng.random() < 0.25:
        return n, trap_queries(rng, n, max(q, n - 1), MAX_D, [1])
    scale = rng.choice([0, 2, 10, MAX_D // 2])
    return n, random_queries(rng, n, q, scale, rng.choice([0.3, 0.6, 0.9]), rng.choice([0.0, 0.1, 0.3]))


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
        n, q = SMALL[seed]
        return n, random_queries(rng, n, q, 20, 0.6, 0.1)
    return plan_case(rng, PLANS[seed - len(SMALL)])


def main() -> None:
    seed = int(sys.argv[1])
    n, queries = case_for(seed, random.Random(seed))
    assert 1 <= n <= MAX_N and 1 <= len(queries) <= MAX_Q
    assert all(1 <= a <= n and 1 <= b <= n and abs(d) <= MAX_D for a, b, d in queries)
    out = [f"{n} {len(queries)}"] + [f"{a} {b} {d}" for a, b, d in queries]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
