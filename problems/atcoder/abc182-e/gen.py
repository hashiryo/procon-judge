"""abc182-e (Akari) の入力を作る。H W N M、N 個の電球の位置、M 個のブロックの位置を出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、H = W = 1500 のいろいろな置き方。
置き方は、ランダム、ブロックを斜めの格子に並べたもの (どの行と列も 65 個ほどのブロックで切れる)、
ブロックの横一列の壁、電球だけの行を並べたもの、市松模様 (電球は全部ブロックに囲まれる)、ブロックで
2 × 2 の部屋に区切ったもの、マスを全部埋めたもの (H = 1500、W = 400 で N + M = HW)、電球が 1 個だけのもの。
RangeSet は行と列ごとにブロックで区間を切るので、区間の数が多い置き方を入れる。
seed が 1000 以上なら、電球ごとに 4 方向へ 1 マスずつ進む愚直解で解ける小さい盤面を出す
(pj testdata crosscheck 用)。2000 以上なら、H, W ≤ 200 の盤面を出す。
"""

import random
import sys

MAX_HW = 1500
MAX_N = 5 * 10**5
MAX_M = 10**5

Case = tuple[int, int, list[tuple[int, int]], list[tuple[int, int]]]

SAMPLES = [
    (3, 3, [(1, 1), (2, 3)], [(2, 2)]),
    (4, 4, [(1, 2), (1, 3), (3, 4)], [(2, 3), (2, 4), (3, 2)]),
    (5, 5, [(1, 1), (2, 2), (3, 3), (4, 4), (5, 5)], [(4, 2)]),
]
# 本番のケースのうち例のあとに並べるもの。
PLANS = [
    "min_row",  # H = 1, W = 2
    "min_column",  # H = 2, W = 1
    "enclosed",  # 3 × 3 の真ん中の電球を 4 個のブロックで囲む。答えは 1
    "one_row",
    "one_column",
    "single_center",  # 電球 1 個とブロック 1 個。答えは 2H - 1
    "single_corner",
    "random_max",
    "lattice_max",
    "full_rect",  # H = 1500、W = 400 を全部埋める。答えは N
    "bulb_rows",
    "walls",
    "checker",  # 答えは N
    "rooms",  # ブロックで 2 × 2 の部屋に区切る
    "sparse_bulbs",
    "one_block",
    "diagonal",
    "random_mid",
    "random_mid",
]
COUNT = len(SAMPLES) + len(PLANS)


def random_cells(rng: random.Random, h: int, w: int, n: int, m: int) -> Case:
    cells = rng.sample(range(h * w), n + m)
    to = lambda c: (c // w + 1, c % w + 1)
    return h, w, [to(c) for c in cells[:n]], [to(c) for c in cells[n:]]


def shuffled(rng: random.Random, cells: set[tuple[int, int]]) -> list[tuple[int, int]]:
    """集合の並びに頼らないよう、並べてから混ぜる。"""
    out = sorted(cells)
    rng.shuffle(out)
    return out


def split_rest(rng: random.Random, h: int, w: int, blocks: set[tuple[int, int]], n: int) -> Case:
    """ブロックを決めたあと、残りのマスから電球を n 個選ぶ。"""
    free = [(i, j) for i in range(1, h + 1) for j in range(1, w + 1) if (i, j) not in blocks]
    bulbs = rng.sample(free, min(n, len(free)))
    return h, w, bulbs, shuffled(rng, blocks)


def plan_case(rng: random.Random, plan: str) -> Case:
    n_max = MAX_HW
    if plan == "min_row":
        return 1, 2, [(1, 1)], [(1, 2)]
    if plan == "min_column":
        return 2, 1, [(2, 1)], [(1, 1)]
    if plan == "enclosed":
        return 3, 3, [(2, 2)], [(1, 2), (2, 1), (2, 3), (3, 2)]
    if plan == "one_row":
        return random_cells(rng, 1, n_max, 500, 500)
    if plan == "one_column":
        return random_cells(rng, n_max, 1, 500, 500)
    if plan == "single_center":
        return n_max, n_max, [(750, 750)], [(1, 1)]
    if plan == "single_corner":
        return n_max, n_max, [(1, 1)], [(n_max, n_max)]
    if plan == "random_max":
        return random_cells(rng, n_max, n_max, MAX_N, MAX_M)
    if plan == "lattice_max":
        # (i + 7j) mod 23 = 0 のマスにブロック。どの行と列でも 23 マスおきに来る (97826 個)。
        blocks = {(i, j) for i in range(1, n_max + 1) for j in range(1, n_max + 1) if (i + 7 * j) % 23 == 0}
        return split_rest(rng, n_max, n_max, blocks, MAX_N)
    if plan == "full_rect":
        return random_cells(rng, n_max, 400, MAX_N, MAX_M)
    if plan == "bulb_rows":
        # 上の 150 行は全部電球、残りの行にブロックをランダムに置く。
        bulbs = [(i, j) for i in range(1, 151) for j in range(1, n_max + 1)]
        rest = rng.sample(range(150 * n_max, n_max * n_max), MAX_M)
        return n_max, n_max, bulbs, [(c // n_max + 1, c % n_max + 1) for c in rest]
    if plan == "walls":
        # 23 行おきにブロックだけの行を置く (65 行)。
        blocks = {(i, j) for i in range(12, n_max + 1, 23) for j in range(1, n_max + 1)}
        return split_rest(rng, n_max, n_max, blocks, 10**5)
    if plan == "checker":
        k = 447
        blocks = {(i, j) for i in range(1, k + 1) for j in range(1, k + 1) if (i + j) % 2 == 0}
        bulbs = [(i, j) for i in range(1, k + 1) for j in range(1, k + 1) if (i + j) % 2 == 1]
        rng.shuffle(bulbs)
        return k, k, bulbs, shuffled(rng, blocks)
    if plan == "rooms":
        k = 300
        blocks = {(i, j) for i in range(1, k + 1) for j in range(1, k + 1) if i % 3 == 0 or j % 3 == 0}
        return split_rest(rng, k, k, blocks, 20000)
    if plan == "sparse_bulbs":
        return random_cells(rng, n_max, n_max, 50000, MAX_M)
    if plan == "one_block":
        return random_cells(rng, n_max, n_max, 10**5, 1)
    if plan == "diagonal":
        cells = rng.sample([c for c in range(n_max * n_max) if c // n_max != c % n_max], MAX_M)
        return n_max, n_max, [(i, i) for i in range(1, n_max + 1)], [(c // n_max + 1, c % n_max + 1) for c in cells]
    assert plan == "random_mid"
    h, w = rng.randint(1, n_max), rng.randint(1, n_max)
    while h * w < 4:
        h, w = rng.randint(1, n_max), rng.randint(1, n_max)
    m = rng.randint(1, min(MAX_M, h * w // 2))
    n = rng.randint(1, min(50000, h * w - m))
    return random_cells(rng, h, w, n, m)


def small_case(rng: random.Random, max_hw: int, max_nm: int) -> Case:
    h, w = rng.randint(1, max_hw), rng.randint(1, max_hw)
    while h * w < 2:
        h, w = rng.randint(1, max_hw), rng.randint(1, max_hw)
    cells = h * w
    m = rng.randint(1, max(1, min(max_nm, cells - 1, int(cells * rng.choice([0.05, 0.2, 0.5])))))
    n = rng.randint(1, max(1, min(max_nm, cells - m, int(cells * rng.choice([0.02, 0.1, 0.5, 1.0])))))
    return random_cells(rng, h, w, n, m)


def case_for(seed: int, rng: random.Random) -> Case:
    if seed >= 2000:
        return small_case(rng, 200, 5000)
    if seed >= 1000:
        return small_case(rng, 8, 64)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    return plan_case(rng, PLANS[seed - len(SAMPLES)])


def main() -> None:
    seed = int(sys.argv[1])
    h, w, bulbs, blocks = case_for(seed, random.Random(seed))
    n, m = len(bulbs), len(blocks)
    assert 1 <= h <= MAX_HW and 1 <= w <= MAX_HW and 1 <= n <= MAX_N and 1 <= m <= MAX_M
    assert all(1 <= a <= h and 1 <= b <= w for a, b in bulbs + blocks)
    assert len(set(bulbs + blocks)) == n + m
    out = [f"{h} {w} {n} {m}"] + [f"{a} {b}" for a, b in bulbs] + [f"{c} {d}" for c, d in blocks]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
