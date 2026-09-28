"""nikkei2019-2-qual-d (Shortest Path on a Line) の入力を作る。N M と M 個の操作 L_i R_i C_i を出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、N = M = 10^5 を中心にいろいろな区間。
提出は区間から区間への辺をセグメント木の頂点で張るので、N が 2 のべきのものとそうでないもの (65536 と 65537) を混ぜる。
区間は、ランダム、長さ 1 の区間を端から端まで並べたもの (答えは 10^9 × (N - 1) で int に収まらない)、
どこかに隙間があって届かないもの (答えは -1)、全部 [1, N]、入れ子、1 から N までつながった短い区間、長さ 1000 の区間をずらしたもの。
頂点 1 を含む区間は L = 1 のものだけなので、ランダムな区間には L = 1 のものと R = N のものを 1 つずつ入れる。
seed が 1000 以上なら、頂点どうしの辺を全部張ってダイクストラ法で解く愚直解で解ける N = 30 までの入力を出す
(pj testdata crosscheck 用)。2000 以上なら、愚直解でまだ解ける N = 300 までの入力を出す。
"""

import random
import sys

MAX_N = 10**5
MAX_M = 10**5
MAX_C = 10**9

SAMPLES = [
    (4, [(1, 3, 2), (2, 4, 3), (1, 4, 6)]),
    (4, [(1, 2, 1), (3, 4, 2)]),
    (10, [(1, 5, 18), (3, 4, 8), (1, 3, 5), (4, 7, 10), (5, 9, 8), (6, 10, 5), (8, 10, 3)]),
]
FIXED = [
    (2, [(1, 2, 1)]),
    (2, [(1, 2, MAX_C)]),
    (3, [(1, 2, 5), (2, 3, 5), (1, 3, 11)]),
    (3, [(2, 3, 1)]),  # 1 から出る辺がない
    (MAX_N, [(1, MAX_N, MAX_C)]),
]
# (N, M, 区間の作り方)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    (MAX_N, MAX_M, "random"),
    (MAX_N, MAX_N - 1, "chain_max"),
    (MAX_N, MAX_M, "gap"),
    (MAX_N, MAX_M, "whole"),
    (MAX_N, MAX_M, "nested"),
    (MAX_N, MAX_M, "short"),
    (MAX_N, MAX_M, "window"),
    (65536, MAX_M, "random"),
    (65537, MAX_M, "short"),
    (MAX_N, MAX_M, "random_small_c"),
    (1000, 1000, "random"),
]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)


def random_interval(rng: random.Random, n: int, max_len: int) -> tuple[int, int]:
    length = rng.randint(1, min(max_len, n - 1))
    left = rng.randint(1, n - length)
    return left, left + length


def touch_ends(ops: list[tuple[int, int, int]], n: int, rng: random.Random, max_c: int) -> list[tuple[int, int, int]]:
    """頂点 1 を含む区間は L = 1 のものだけ、頂点 N を含むのは R = N のものだけなので、1 つずつ入れておく。"""
    ops[0] = (1, rng.randint(2, n), rng.randint(1, max_c))
    ops[-1] = (rng.randint(1, n - 1), n, rng.randint(1, max_c))
    return ops


def short_chain(n: int, m: int, max_len: int, rng: random.Random) -> list[tuple[int, int, int]]:
    """長さ max_len 以下の区間を 1 から N までつなぎ (最短路が長くなる)、残りは短いランダムな区間。"""
    ops, cur = [], 1
    while cur < n:
        nxt = min(n, cur + rng.randint(1, max_len))
        ops.append((cur, nxt, rng.randint(1, MAX_C)))
        cur = nxt
    assert len(ops) <= m
    while len(ops) < m:
        ops.append((*random_interval(rng, n, max_len), rng.randint(1, MAX_C)))
    rng.shuffle(ops)
    return ops


def make(n: int, m: int, how: str, rng: random.Random) -> list[tuple[int, int, int]]:
    if how == "random":
        ops = [(*sorted(rng.sample(range(1, n + 1), 2)), rng.randint(1, MAX_C)) for _ in range(m)]
        return touch_ends(ops, n, rng, MAX_C)
    if how == "random_small_c":
        ops = [(*sorted(rng.sample(range(1, n + 1), 2)), rng.randint(1, 3)) for _ in range(m)]
        return touch_ends(ops, n, rng, 3)
    if how == "chain_max":
        ops = [(i, i + 1, MAX_C) for i in range(1, n)]
        rng.shuffle(ops)
        return ops
    if how == "gap":
        # k と k + 1 をまたぐ区間を作らない。答えは -1。
        k = rng.randint(n // 3, 2 * n // 3)
        ops = []
        while len(ops) < m:
            l, r = random_interval(rng, n, 5000)
            if not (l <= k < r):
                ops.append((l, r, rng.randint(1, MAX_C)))
        return ops
    if how == "whole":
        return [(1, n, rng.randint(1, MAX_C)) for _ in range(m)]
    if how == "nested":
        # [i, n + 1 - i] の入れ子。内側ほど安い。
        ops = [(i, n + 1 - i, MAX_C - i) for i in range(1, min(m, n // 2) + 1)]
        while len(ops) < m:
            ops.append((*random_interval(rng, n, 50), rng.randint(1, MAX_C)))
        rng.shuffle(ops)
        return ops
    if how == "short":
        return short_chain(n, m, 3, rng)
    assert how == "window"
    ops = [(i, i + 1000, rng.randint(1, MAX_C)) for i in range(1, n - 999)]
    ops = ops[:m]
    while len(ops) < m:
        ops.append((*random_interval(rng, n, 1000), rng.randint(1, MAX_C)))
    rng.shuffle(ops)
    return ops


def small_case(rng: random.Random, medium: bool) -> tuple[int, list[tuple[int, int, int]]]:
    n = rng.randint(31, 300) if medium else rng.randint(2, 30)
    m = rng.randint(1, n if medium else 30)
    max_len = rng.choice([1, 2, 5, n - 1])
    max_c = rng.choice([1, 10, MAX_C])
    ops = [(*random_interval(rng, n, max_len), rng.randint(1, max_c)) for _ in range(m)]
    if rng.random() < 0.5:
        ops = touch_ends(ops + [(1, 2, 1)], n, rng, max_c)
    return n, ops


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 1000:
        n, ops = small_case(rng, seed >= 2000)
    elif seed < len(SAMPLES):
        n, ops = SAMPLES[seed]
    elif seed < len(SAMPLES) + len(FIXED):
        n, ops = FIXED[seed - len(SAMPLES)]
    else:
        n, m, how = PLANS[seed - len(SAMPLES) - len(FIXED)]
        ops = make(n, m, how, rng)
    assert 2 <= n <= MAX_N and 1 <= len(ops) <= MAX_M
    assert all(1 <= l < r <= n and 1 <= c <= MAX_C for l, r, c in ops)
    sys.stdout.write("\n".join([f"{n} {len(ops)}"] + [f"{l} {r} {c}" for l, r, c in ops]) + "\n")


if __name__ == "__main__":
    main()
