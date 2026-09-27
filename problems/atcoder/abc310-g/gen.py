"""abc310-g (Takahashi And Pass-The-Ball Game) の入力を作る。N K、A_1 ... A_N、B_1 ... B_N を出す。

i から A_i への辺で関数グラフになる。seed が 0 から count - 1 までは本番のケース。例の 4 つ、角のケース、
N = 2 × 10^5 と 5 × 10^4 のいろいろな形のグラフ。角のケースは、N = 1、N = 2 の入れ替え、B が全部 0、
全部 998244352、K が 1、10^18、998244353 で割って 1 や -1 余るもの。
グラフの形は、ランダム、ランダムな順列 (輪がたくさん)、全体で 1 つの輪、全部が自分へ (答えは B のまま)、
全部が 1 人へ、長い道の先が自己ループ、尻尾と輪が半分ずつの ρ、二分木を根へ向けたもの、毛虫、
2 つの輪に木がぶら下がるもの、深いランダム。K は、尻尾の長さちょうど、尻尾 + 輪、輪の長さの前後も入れる。
出力が N 個の数で大きいので、N = 2 × 10^5 は 5 ケースにして、そのうち 4 つは B を 10 未満にする。
seed が 1000 以上なら、K 回の操作をそのまま行うか、人ごとに玉の行き先をたどって数える愚直解で解ける
小さい入力を出す (pj testdata crosscheck 用)。
"""

import random
import sys

P = 998244353
MAX_N = 2 * 10**5
MAX_K = 10**18

SAMPLES = [
    (2, [3, 1, 4, 1, 5], [1, 1, 2, 3, 5]),
    (1000, [1, 1, 1], [1, 10, 100]),
    (1000007, [16, 12, 6, 12, 1, 8, 14, 14, 5, 7, 6, 5, 9, 6, 10, 9],
     [719092922, 77021920, 539975779, 254719514, 967592487, 476893866, 368936979, 465399362, 342544824, 540338192,
      42663741, 165480608, 616996494, 16552706, 590788849, 221462860]),
    (100000000007, [19, 10, 19, 15, 1, 20, 13, 15, 8, 23, 22, 16, 19, 22, 2, 20, 12, 19, 17, 20, 16, 8, 23, 6],
     [944071276, 364842194, 5376942, 671161415, 477159272, 339665353, 176192797, 2729865, 676292280, 249875565,
      259803120, 103398285, 466932147, 775082441, 720192643, 535473742, 263795756, 898670859, 476980306, 12045411,
      620291602, 593937486, 761132791, 746546443]),
]
FIXED = [
    (1, [1], [0]),
    (MAX_K, [1], [P - 1]),
    (1, [2, 1], [1, 2]),
    (2, [2, 1], [1, 2]),
    (P - 1, [2, 3, 1], [1, 2, 3]),  # K ≡ -1
    (P + 1, [2, 3, 1], [1, 2, 3]),  # K ≡ 1
    (2 * P - 1, [2, 2, 3, 3], [P - 1, P - 1, P - 1, P - 1]),
    (MAX_K, [3, 3, 1, 2, 4], [0, 0, 0, 0, 0]),
]
# (N, グラフの形, K の決め方, B の上限)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    (MAX_N, "random", "max", P),
    (MAX_N, "permutation", "random", 10),
    (MAX_N, "rho", "tail+cycle", 10),
    (MAX_N, "path_to_loop", "n", 10),
    (MAX_N, "binary", "random", 10),
    (50000, "cycle", "n", P),
    (50000, "cycle", "n+1", P),
    (50000, "identity", "random", P),
    (50000, "star", "one", P),
    (50000, "rho", "tail", P),
    (50000, "caterpillar", "random", P),
    (50000, "two_cycles", "max", P),
    (50000, "deep", "random", P),
    (50000, "random", "one", P),
    (50000, "random", "two", P),
]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)
SHAPES = ["random", "permutation", "cycle", "identity", "star", "path_to_loop", "rho", "binary", "caterpillar",
          "two_cycles", "deep"]


def graph(rng: random.Random, n: int, shape: str) -> list[int]:
    """頂点 0 から n - 1 の行き先。"""
    if shape == "random":
        return [rng.randrange(n) for _ in range(n)]
    if shape == "permutation":
        to = list(range(n))
        rng.shuffle(to)
        return to
    if shape == "cycle":
        return [(i + 1) % n for i in range(n)]
    if shape == "identity":
        return list(range(n))
    if shape == "star":
        return [n - 1] * n
    if shape == "path_to_loop":
        return [min(i + 1, n - 1) for i in range(n)]
    if shape == "rho":
        return [i + 1 for i in range(n - 1)] + [n // 2]
    if shape == "binary":
        # 根から幅優先で j 番目の頂点を n - 1 - j にし、親を (j - 1) // 2 番目にする。根 n - 1 は自己ループ。
        return [n - 1 - (n - 2 - v) // 2 if v < n - 1 else n - 1 for v in range(n)]
    if shape == "caterpillar":
        spine = max(1, n // 2)
        return [min(i + 1, spine - 1) for i in range(spine)] + [rng.randrange(spine) for _ in range(spine, n)]
    if shape == "two_cycles":
        half = max(1, n // 4)
        to = [0] * n
        for c in (list(range(half)), list(range(half, min(n, 2 * half)))):
            for j, v in enumerate(c):
                to[v] = c[(j + 1) % len(c)]
        for v in range(2 * half, n):
            to[v] = rng.randrange(v)
        return to
    assert shape == "deep"
    return [rng.randint(i + 1, min(n - 1, i + 3)) if i < n - 1 else n - 1 for i in range(n)]


def rho(to: list[int]) -> tuple[int, int]:
    """頂点 0 から始めたときの (尻尾の長さ, 輪の長さ)。"""
    seen: dict[int, int] = {}
    v = 0
    while v not in seen:
        seen[v] = len(seen)
        v = to[v]
    return seen[v], len(seen) - seen[v]


def pick_k(rng: random.Random, how: str, n: int, tail: int, cycle: int) -> int:
    k = {
        "max": MAX_K,
        "random": rng.randint(1, MAX_K),
        "one": 1,
        "two": 2,
        "n": n,
        "n+1": n + 1,
        "tail": max(1, tail),
        "tail+cycle": tail + cycle,
    }[how]
    return k if k % P else k + 1


def labeled(rng: random.Random, to: list[int]) -> list[int]:
    """番号を付け直して 1 始まりにする。"""
    n = len(to)
    perm = list(range(1, n + 1))
    rng.shuffle(perm)
    out = [0] * n
    for v in range(n):
        out[perm[v] - 1] = perm[to[v]]
    return out


def small_case(rng: random.Random) -> tuple[int, list[int], list[int]]:
    n = rng.choice([1, rng.randint(1, 8), rng.randint(1, 30)])
    to = graph(rng, n, rng.choice(SHAPES))
    tail, cycle = rho(to)
    a = labeled(rng, to)
    top = rng.choice([1, 10, P])
    b = [rng.randrange(top) for _ in range(n)]
    k = rng.choice([
        rng.randint(1, 20),
        rng.randint(1, 2000),
        rng.randint(1, MAX_K),
        MAX_K,
        max(1, tail + cycle * rng.randint(0, 3) + rng.randint(-1, 1)),
        rng.choice([P - 1, P + 1, 2 * P + 5]),
    ])
    return (k if k % P else k + 1), a, b


def case_for(seed: int, rng: random.Random) -> tuple[int, list[int], list[int]]:
    if seed >= 1000:
        return small_case(rng)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    n, shape, how_k, top = PLANS[seed - len(FIXED)]
    to = graph(rng, n, shape)
    return pick_k(rng, how_k, n, *rho(to)), labeled(rng, to), [rng.randrange(top) for _ in range(n)]


def main() -> None:
    seed = int(sys.argv[1])
    k, a, b = case_for(seed, random.Random(seed))
    n = len(a)
    assert 1 <= n <= MAX_N and len(b) == n and 1 <= k <= MAX_K and k % P != 0
    assert all(1 <= v <= n for v in a) and all(0 <= v < P for v in b)
    sys.stdout.write(f"{n} {k}\n{' '.join(map(str, a))}\n{' '.join(map(str, b))}\n")


if __name__ == "__main__":
    main()
