"""abc204-e (Rush Hour 2) の入力を作る。N M と M 本の道 A B C D を出す。答えは町 N に着く最も早い時刻 (着けなければ -1)。

seed が 0 から count - 1 までは本番のケース。例の 4 つ、角のケース、N と M が 10^5 のケース。
角のケースは、道が無いもの、自己ループしか無いもの、C と D が 0 か 10^9 のもの、浮動小数点で丸めると 1 ずれるもの。
最後のものは、町 u に時刻 d で着き、D = q(d + 1) - 1 の道を時刻 d に渡り始めるようにしてある。
floor(D / (d + 1)) = q - 1 だが、d + C + D / (d + 1) を double で求めると q - 1 / (d + 1) が次の整数に丸まる。
大きいケースは、ランダムな全域木に道を足したグラフ (C と D はランダム、小さい値だけ、C = 0、D = 0)、
C = D = 10^9 のパス (答えは 10^14 近く)、町 N に着けないもの、町が 1000 で多重辺だらけのもの、格子、
大きいグラフの中に丸めで 1 ずれる道を最短路として埋めたものである。
seed が 1000 以上なら、渡り始める時刻を全部試すダイクストラ法の愚直解で解ける小さい入力を出す
(pj testdata crosscheck 用)。2000 以上なら、町が 200 と道が 400 までの入力を出す。
"""

import random
import sys

MAX_N = 10**5
MAX_M = 10**5
MAX_V = 10**9

Edge = tuple[int, int, int, int]

SAMPLES: list[tuple[int, list[Edge]]] = [
    (2, [(1, 2, 2, 3)]),
    (2, [(1, 2, 2, 3), (1, 2, 2, 1), (1, 1, 1, 1)]),
    (4, [(1, 2, 3, 4), (3, 4, 5, 6)]),
    (6, [
        (1, 1, 0, 0), (1, 3, 1, 2), (1, 5, 2, 3), (5, 2, 16, 5), (2, 6, 1, 10),
        (3, 4, 3, 4), (3, 5, 3, 10), (5, 6, 1, 100), (4, 2, 0, 110),
    ]),
]
FIXED: list[tuple[int, list[Edge]]] = [
    (2, []),  # 道が無い
    (2, [(1, 1, 5, 5), (2, 2, 0, 0)]),  # 自己ループだけ
    (2, [(1, 2, 0, 0)]),  # 答えは 0
    (2, [(1, 2, 0, MAX_V)]),  # sqrt(10^9) 近くまで待つ
    (2, [(1, 2, MAX_V, MAX_V)]),
    # 町 2 に時刻 999999999 で着く。floor(999999999 / 10^9) = 0 で答えは 1999999999。
    (3, [(1, 2, 999999999, 0), (2, 3, MAX_V, 999999999)]),
    # 町 2 に時刻 499999999 で着く。floor(999999999 / 5 x 10^8) = 1 で答えは 1500000000。
    (3, [(1, 2, 499999999, 0), (2, 3, MAX_V, 999999999)]),
]
PLANS = ["random", "path", "small", "zero_c", "zero_d", "unreachable", "dense", "grid", "trap"]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)


def trap_edges(rng: random.Random, u: int, goal: int) -> list[Edge]:
    """1 から u、u から goal の 2 本。u に時刻 d で着き、D = q(d + 1) - 1 の道をすぐ渡るのが最短になる。
    d + C + D / (d + 1) は 2^30 以上で、1 / (d + 1) <= 2^-27 は double の刻み 2^-22 の半分より小さいので、
    q - 1 / (d + 1) が q に丸まる。C を 10^9 - 1000 まで、d を 10^9 / q - 2000 までにして、
    C = 10^9 の道を通るほかの道筋より 2 以上早く着くようにする。
    """
    q = rng.choice([1, 1, 2, 3])
    d = rng.randint(2**27, (MAX_V + 1) // q - 2000)
    c = rng.randint(max(0, 2**30 - d), MAX_V - 1000)
    return [(1, u, d, 0), (u, goal, c, q * (d + 1) - 1)]


def slow_down(edges: list[Edge], n: int) -> list[Edge]:
    """町 1 か町 n に触れる道を C = 10^9 にする。1 と n を直に結ぶ道は町 1 の自己ループにする。"""
    out = []
    for a, b, c, d in edges:
        if {a, b} == {1, n}:
            a = b = 1
        out.append((a, b, MAX_V if 1 in (a, b) or n in (a, b) else c, d))
    return out


def weight(rng: random.Random, how: str) -> tuple[int, int]:
    if how == "small":
        return rng.randint(0, 5), rng.randint(0, 5)
    if how == "zero_c":
        return 0, rng.randint(0, MAX_V)
    if how == "zero_d":
        return rng.randint(0, MAX_V), 0
    if how == "max":
        return MAX_V, MAX_V
    if how == "mixed":
        return tuple(rng.choice([0, 1, rng.randint(0, 1000), rng.randint(0, MAX_V), MAX_V]) for _ in range(2))
    return rng.randint(0, MAX_V), rng.randint(0, MAX_V)


def spanning_plus(rng: random.Random, n: int, m: int) -> list[tuple[int, int]]:
    """ランダムな全域木に、自己ループと多重辺も入る道を足して m 本にする。"""
    perm = list(range(1, n + 1))
    rng.shuffle(perm)
    pairs = [(perm[rng.randrange(i)], perm[i]) for i in range(1, n)]
    pairs += [(rng.randint(1, n), rng.randint(1, n)) for _ in range(m - len(pairs))]
    rng.shuffle(pairs)
    return [(a, b) if rng.random() < 0.5 else (b, a) for a, b in pairs]


def plan_case(rng: random.Random, how: str) -> tuple[int, list[Edge]]:
    n, m = MAX_N, MAX_M
    if how == "path":
        return n, [(i, i + 1, MAX_V, MAX_V) for i in range(1, n)] + [(n, n, 0, 0)]
    if how == "unreachable":
        edges = spanning_plus(rng, n - 1, m)
        return n, [(a, b, *weight(rng, "random")) for a, b in edges]
    if how == "dense":
        n = 1000
        return n, [(a, b, *weight(rng, "random")) for a, b in spanning_plus(rng, n, m)]
    if how == "grid":
        h, w = 200, 250  # 5 x 10^4 町、99550 本
        cell = lambda i, j: i * w + j + 1  # noqa: E731
        pairs = [(cell(i, j), cell(i, j + 1)) for i in range(h) for j in range(w - 1)]
        pairs += [(cell(i, j), cell(i + 1, j)) for i in range(h - 1) for j in range(w)]
        rng.shuffle(pairs)
        return h * w, [(a, b, *weight(rng, "mixed")) for a, b in pairs]
    if how == "trap":
        # ランダムなグラフの中に、1 -> u -> n の丸めで 1 ずれる最短路を埋める。全域木と合わせて 10^5 本にする。
        n -= 1
        edges = [(a, b, *weight(rng, "random")) for a, b in spanning_plus(rng, n, m - 2)]
        edges = slow_down(edges, n) + trap_edges(rng, rng.randint(2, n - 1), n)
        rng.shuffle(edges)
        return n, edges
    return n, [(a, b, *weight(rng, how)) for a, b in spanning_plus(rng, n, m)]


def small_case(rng: random.Random, max_n: int, max_m: int) -> tuple[int, list[Edge]]:
    """愚直解で解ける大きさ。3 割は丸めで 1 ずれる道を最短路として埋める。"""
    n = rng.randint(2, max_n)
    how = rng.choice(["random", "small", "zero_c", "zero_d", "mixed", "max"])
    m = rng.randint(0, max_m)
    pairs = [(rng.randint(1, n), rng.randint(1, n)) for _ in range(m)]
    edges = [(a, b, *weight(rng, how)) for a, b in pairs]
    if n >= 3 and rng.random() < 0.3:
        edges = slow_down(edges, n) + trap_edges(rng, rng.randint(2, n - 1), n)
        rng.shuffle(edges)
    return n, edges


def case_for(seed: int, rng: random.Random) -> tuple[int, list[Edge]]:
    if seed >= 2000:
        return small_case(rng, 200, 400)
    if seed >= 1000:
        return small_case(rng, 8, 12)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    return plan_case(rng, PLANS[seed - len(FIXED)])


def main() -> None:
    seed = int(sys.argv[1])
    n, edges = case_for(seed, random.Random(seed))
    assert 2 <= n <= MAX_N and 0 <= len(edges) <= MAX_M
    assert all(1 <= a <= n and 1 <= b <= n and 0 <= c <= MAX_V and 0 <= d <= MAX_V for a, b, c, d in edges)
    out = [f"{n} {len(edges)}"] + [f"{a} {b} {c} {d}" for a, b, c, d in edges]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
