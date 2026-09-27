"""arc129-e (Yet Another Minimization) の入力を作る。N M、N * M 行の A C と、W の上三角を出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、N = 2 の角のケース、N = 50 の大きいもの。
値は、ランダム、C も W も最大 (答えは 5 * 10^16 ほど)、C が小さくて W が効くもの、W が 1 で C が
効くもの、C と W * |A の差| の大きさを揃えて張り合わせたもの、どの i も同じ候補を持つもの、i ごとに
候補の範囲が離れたもの、候補が 1 付近と 10^6 付近に分かれたものを混ぜる。M は 5 を中心に 2 と 3 も。
seed が 1000 以上なら、選び方 M^N 通りを全部試す愚直解で解ける M^N <= 2 * 10^5 の入力を出す
(pj testdata crosscheck 用)。
"""

import random
import sys

MAX_N = 50
MAX_M = 5
MAX_A = 10**6
MAX_C = 10**15
MAX_W = 10**6

SAMPLES = [
    ([[(1, 1), (5, 2)], [(2, 3), (9, 4)], [(7, 2), (8, 2)]], [[1, 5], [3]]),  # 例 1
    (
        [
            [(19, 2517), (38, 785), (43, 3611)], [(3, 681), (20, 758), (45, 4745)],
            [(6, 913), (7, 2212), (22, 536)], [(4, 685), (27, 148), (36, 2283)],
            [(25, 3304), (36, 1855), (43, 2747)], [(11, 1976), (32, 4973), (43, 3964)],
            [(3, 4242), (16, 4750), (50, 24)], [(4, 4231), (22, 1526), (31, 2152)],
            [(15, 2888), (28, 2249), (49, 2208)], [(31, 3127), (40, 3221), (47, 4671)],
        ],
        [
            [24, 6, 16, 47, 42, 50, 35, 43, 47], [29, 18, 28, 24, 27, 25, 33, 12],
            [5, 43, 20, 9, 39, 46, 30], [40, 24, 34, 5, 30, 21], [50, 6, 21, 36, 5],
            [50, 16, 13, 13], [2, 40, 15], [25, 48], [20],
        ],
    ),  # 例 2
    ([[(1, 1), (10, 10)], [(1, 1), (10, 10)]], [[100]]),  # 例 3
]
FIXED = [
    ([[(1, 1), (2, 1)], [(1, 1), (2, 1)]], [[1]]),
    ([[(MAX_A - 1, MAX_C), (MAX_A, MAX_C)], [(MAX_A - 1, MAX_C), (MAX_A, MAX_C)]], [[MAX_W]]),
    ([[(1, 1), (MAX_A, MAX_C)], [(1, MAX_C), (MAX_A, 1)]], [[1]]),  # 離れた候補を選ぶほうが安い
    ([[(k, 1) for k in range(1, 6)], [(MAX_A - 5 + k, 1) for k in range(1, 6)]], [[MAX_W]]),
]
# (N, M, 値の出し方)。本番のケースのうち FIXED のあとに並べる。
PLANS = [
    (MAX_N, MAX_M, "random"),
    (MAX_N, MAX_M, "max"),  # C も W も最大
    (MAX_N, MAX_M, "c_small"),
    (MAX_N, MAX_M, "w_small"),
    (MAX_N, MAX_M, "balanced"),
    (MAX_N, MAX_M, "balanced"),
    (MAX_N, MAX_M, "same_a"),
    (MAX_N, MAX_M, "disjoint"),
    (MAX_N, MAX_M, "split"),
    (MAX_N, 2, "balanced"),
    (MAX_N, 3, "random"),
    (None, None, "balanced"),  # N と M はランダム
    (None, None, "random"),
    (None, None, "split"),
]
KINDS = ["random", "max", "c_small", "w_small", "balanced", "same_a", "disjoint", "split"]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)


def candidates(rng: random.Random, m: int, lo: int, hi: int) -> list[int]:
    """[lo, hi] から相異なる m 個を小さい順に。"""
    return sorted(rng.sample(range(lo, hi + 1), m))


def build(rng: random.Random, n: int, m: int, how: str, a_max: int = MAX_A) -> tuple[list, list]:
    if how == "same_a":
        shared = candidates(rng, m, 1, a_max)
        a = [shared] * n
    elif how == "disjoint":
        # i ごとに候補の範囲を分け、範囲の並びを混ぜる。
        width = max(a_max // n, m)
        order = list(range(n))
        rng.shuffle(order)
        a = [candidates(rng, m, order[i] * width + 1, (order[i] + 1) * width) for i in range(n)]
    elif how == "split":
        # 候補を 1 付近と a_max 付近に分ける。
        low = rng.randint(1, m - 1)
        a = [list(range(1, low + 1)) + list(range(a_max - (m - low) + 1, a_max + 1)) for _ in range(n)]
    else:
        a = [candidates(rng, m, 1, a_max) for _ in range(n)]
    c_max, w_max = {
        "max": (MAX_C, MAX_W),
        "c_small": (10, MAX_W),
        "w_small": (MAX_C, 1),
        # C と、W * |A の差| を N 個ぶん足したものの大きさを揃える。
        "balanced": (min(MAX_C, n * a_max * MAX_W // 4), MAX_W),
    }.get(how, (MAX_C, MAX_W))
    fixed = how == "max"
    c = [[c_max if fixed else rng.randint(1, c_max) for _ in range(m)] for _ in range(n)]
    w = [[w_max if fixed else rng.randint(1, w_max) for _ in range(n - 1 - i)] for i in range(n - 1)]
    return [list(zip(a[i], c[i])) for i in range(n)], w


def small_size(rng: random.Random) -> tuple[int, int]:
    """M^N <= 2 * 10^5 の N と M。"""
    n = rng.randint(2, 12)
    m = max(k for k in range(2, MAX_M + 1) if k**n <= 2 * 10**5)
    return n, rng.randint(2, m)


def case_for(seed: int, rng: random.Random) -> tuple[list, list]:
    if seed >= 1000:
        n, m = small_size(rng)
        return build(rng, n, m, rng.choice(KINDS), a_max=rng.choice([10, 100, MAX_A]))
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    n, m, how = PLANS[seed - len(FIXED)]
    return build(rng, n or rng.randint(2, MAX_N), m or rng.randint(2, MAX_M), how)


def main() -> None:
    seed = int(sys.argv[1])
    rows, w = case_for(seed, random.Random(seed))
    n, m = len(rows), len(rows[0])
    assert 2 <= n <= MAX_N and 2 <= m <= MAX_M and all(len(row) == m for row in rows)
    assert all(1 <= row[0][0] and row[-1][0] <= MAX_A for row in rows)
    assert all(row[k][0] < row[k + 1][0] for row in rows for k in range(m - 1))
    assert all(1 <= cost <= MAX_C for row in rows for _, cost in row)
    assert [len(line) for line in w] == list(range(n - 1, 0, -1))
    assert all(1 <= x <= MAX_W for line in w for x in line)
    out = [f"{n} {m}"] + [f"{a} {c}" for row in rows for a, c in row] + [" ".join(map(str, line)) for line in w]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
