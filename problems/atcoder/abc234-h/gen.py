"""abc234-h (Enumerate Pairs) の入力を作る。N K と、N 行の x_i y_i を出す。

距離が K 以下の組は 4 × 10^5 個までという制約があるので、一辺 K の升目に振り分けて数えて確かめる。
seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、N = 2 × 10^5 までのいろいろな点の置き方。
角のケースは、N = 1 (答え 0)、同じ点 2 つ、距離がちょうど K (3 4 5 の直角三角形、7.5 × 10^8 と 10^9 と 1.25 × 10^9)、
距離が K を 1 だけ超えるもの、距離が √2 × 10^9 で K = 1414213563 と 1414213562、K = 1.5 × 10^9 で座標が端のもの。
大きいケースは、一様なランダム (組が 2.5 × 10^5 個ほど)、間隔 s の格子で K = s - 1 (組は無く、どの点の円も隣の点を
ぎりぎり外す)、間隔 s の格子で K = s (隣とちょうど距離 K)、間隔 K の横一列、縦一列 (x が全部同じ)、斜め一列、
距離 K + 1 の平行な 2 列、同じ点が 894 個 (組が 399171 個)、座標が 0 から 1000 で K = 1 (同じ点が多い)、
N = 500 で K = 1.5 × 10^9 (全部の組)。出力も組ごとに 1 行で大きいので、組の多いケースは数を絞る。
seed が 1000 以上なら、全部の組を調べる愚直解で解ける小さい入力を出す (pj testdata crosscheck 用)。
2000 以上なら、N を 894 までにする (全部の組が 4 × 10^5 個以下になる)。愚直解は N = 2 × 10^5 でも 20 秒ほどなので、
本番のケースも全部突き合わせられる。
"""

import math
import random
import sys

MAX_N = 2 * 10**5
MAX_K = 15 * 10**8
MAX_C = 10**9
MAX_PAIRS = 4 * 10**5

Case = tuple[int, list[tuple[int, int]]]

SAMPLES: list[Case] = [
    (5, [(2, 0), (2, 2), (3, 4), (0, 0), (5, 5), (8, 3)]),
    (1414213562, [(0, 0), (MAX_C, MAX_C)]),
    (150, [(300, 300), (300, 400), (300, 500), (400, 300), (400, 400), (400, 400), (400, 500), (500, 300),
           (500, 400), (500, 500)]),
]
FIXED: list[Case] = [
    (1, [(0, 0)]),  # 答え 0
    (1, [(MAX_C, MAX_C)] * 2),  # 同じ点
    (5, [(0, 0), (3, 4)]),  # ちょうど K
    (4, [(0, 0), (3, 4)]),
    (1250000000, [(0, 0), (750000000, MAX_C)]),  # ちょうど K
    (1249999999, [(0, 0), (750000000, MAX_C)]),
    (MAX_C, [(0, 0), (1, MAX_C)]),  # 距離の 2 乗が K^2 + 1
    (MAX_C, [(0, 0), (0, MAX_C), (MAX_C, 0), (1, MAX_C)]),
    (1414213563, [(0, 0), (MAX_C, MAX_C)]),  # √2 × 10^9 = 1414213562.37...
    (MAX_K, [(0, MAX_C), (MAX_C, 0), (0, 0), (MAX_C, MAX_C)]),  # 全部の組
]
PLANS = ["uniform", "grid_miss", "grid_hit", "line", "vertical", "diagonal", "parallel", "identical",
         "small_coords", "max_k"]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)


def count_pairs(k: int, pts: list[tuple[int, int]]) -> int:
    """距離が k 以下の組の数。一辺 k の升目に振り分け、同じ升目と、隣の 4 つ (右、右上、上、左上) を調べる。"""
    k2 = k * k
    cells: dict[tuple[int, int], list[tuple[int, int]]] = {}
    for x, y in pts:
        cells.setdefault((x // k, y // k), []).append((x, y))
    total = 0
    for (cx, cy), here in cells.items():
        for i, (xi, yi) in enumerate(here):
            for xj, yj in here[i + 1:]:
                total += (xi - xj) ** 2 + (yi - yj) ** 2 <= k2
        for ox, oy in ((1, 0), (1, 1), (0, 1), (-1, 1)):
            for xj, yj in cells.get((cx + ox, cy + oy), ()):
                for xi, yi in here:
                    total += (xi - xj) ** 2 + (yi - yj) ** 2 <= k2
    return total


def plan(rng: random.Random, name: str) -> Case:
    if name == "uniform":
        n = MAX_N
        k = int(math.sqrt(2 * 250000 / (math.pi * n * n)) * MAX_C)
        return k, [(rng.randint(0, MAX_C), rng.randint(0, MAX_C)) for _ in range(n)]
    if name in ("grid_miss", "grid_hit"):
        side, s = (447, 2236067) if name == "grid_miss" else (150, 6 * 10**6)
        pts = [(i * s, j * s) for i in range(side) for j in range(side)]
        return (s - 1 if name == "grid_miss" else s), pts
    if name == "line":  # 隣とちょうど距離 K
        k = 9999
        y = rng.randint(0, MAX_C)
        return k, [(i * k, y) for i in range(6 * 10**4)]
    if name == "vertical":  # x が全部同じ
        k = 10**4
        x = rng.randint(0, MAX_C)
        return k, [(x, i * k) for i in range(6 * 10**4)]
    if name == "diagonal":  # 隣との距離は 10^4 √2 = 14142.13...
        return 14143, [(i * 10**4, i * 10**4) for i in range(6 * 10**4)]
    if name == "parallel":  # 列の中は隣とちょうど K、列どうしは K + 1 離れる
        k = 9999
        half = 3 * 10**4
        return k, [(i * k, 0) for i in range(half)] + [(i * k, k + 1) for i in range(half)]
    if name == "identical":  # 894 × 893 / 2 = 399171 組
        p = (rng.randint(0, MAX_C), rng.randint(0, MAX_C))
        return rng.randint(1, MAX_K), [p] * 894
    if name == "small_coords":
        return 1, [(rng.randint(0, 1000), rng.randint(0, 1000)) for _ in range(MAX_N)]
    assert name == "max_k"
    return MAX_K, [(rng.randint(0, MAX_C), rng.randint(0, MAX_C)) for _ in range(500)]


def small_case(rng: random.Random, medium: bool) -> Case:
    n = rng.randint(61, 894) if medium else rng.randint(1, 60)
    top = rng.choice([3, 10, 1000, MAX_C])
    kind = rng.randrange(4)
    if kind == 0:  # ランダム
        pts = [(rng.randint(0, top), rng.randint(0, top)) for _ in range(n)]
    elif kind == 1:  # 格子
        s = rng.randint(1, max(1, top // 30))
        w = rng.randint(1, 30)
        pts = [(s * (i % w), s * (i // w)) for i in range(n)]
    elif kind == 2:  # 3 4 5 の直角三角形の倍数で離れた点
        c = rng.randint(1, max(1, top // 30))
        pts = [(3 * c * rng.randint(0, 5), 4 * c * rng.randint(0, 5)) for _ in range(n)]
    else:  # 少ない点の重複
        base = [(rng.randint(0, top), rng.randint(0, top)) for _ in range(rng.randint(1, 5))]
        pts = [rng.choice(base) for _ in range(n)]
    pts = [(min(x, MAX_C), min(y, MAX_C)) for x, y in pts]
    rng.shuffle(pts)
    k = rng.choice([1, rng.randint(1, 20), rng.randint(1, max(1, top)), 5 * rng.randint(1, max(1, top // 10)),
                    rng.randint(1, MAX_K)])
    return k, pts


def case_for(seed: int, rng: random.Random) -> Case:
    if seed >= 1000:
        return small_case(rng, seed >= 2000)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    k, pts = plan(rng, PLANS[seed - len(FIXED)])
    rng.shuffle(pts)
    return k, pts


def main() -> None:
    seed = int(sys.argv[1])
    k, pts = case_for(seed, random.Random(seed))
    n = len(pts)
    assert 1 <= n <= MAX_N and 1 <= k <= MAX_K and all(0 <= x <= MAX_C and 0 <= y <= MAX_C for x, y in pts)
    assert n <= 894 or count_pairs(k, pts) <= MAX_PAIRS
    sys.stdout.write(f"{n} {k}\n" + "\n".join(f"{x} {y}" for x, y in pts) + "\n")


if __name__ == "__main__":
    main()
