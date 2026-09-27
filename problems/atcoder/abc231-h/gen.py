"""abc231-h (Minimum Coloring) の入力を作る。H W N と、N 行の A_i B_i C_i を出す。答えは二部グラフの辺被覆の最小の重み。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、N = 1000 のいろいろな盤面。
角のケースは、1 マス、1 行 (全部黒にするしかない)、1 列、置換行列 (全部黒にするしかない)、C が全部 10^9
(答えは 10^12 近くになり、int に収まらない)。
盤面は、ほぼ埋まった 32 × 32 (提出の重み付きマトロイド交差で増加路を 1000 回近く探す)、埋まった 10 × 100 と
25 × 40、まばらな 500 × 500 と 100 × 100、1 行と 1 列が埋まった十字、置換行列にランダムに足したもの、
どの行とどの列にも駒が 2 つあって全体が長さ 1000 の 1 つの閉路になるもの (黒くしない駒は閉路の上で隣り合わない
辺の集合になり、増加路が長くなりやすい)。
重みは、ランダム、全部同じ、1 から 10 (同点だらけ)、1 と 10^9 だけ、を混ぜる。
seed が 1000 以上なら、行ごとに黒くする駒を選んで塗った列の集合を持つ DP の愚直解で解ける小さい入力を出す
(pj testdata crosscheck 用)。2000 以上なら、同じ愚直解でまだ解ける N = 300 までの入力を出す。
"""

import random
import sys

MAX_HW = 1000
MAX_N = 1000
MAX_C = 10**9

SAMPLES = [
    (2, 3, [(1, 1, 1), (1, 2, 10), (1, 3, 100), (2, 1, 1000), (2, 2, 10000), (2, 3, 100000)]),
    (1, 7, [
        (1, 2, 200000000), (1, 7, 700000000), (1, 4, 400000000), (1, 3, 300000000), (1, 6, 600000000),
        (1, 5, 500000000), (1, 1, 100000000),
    ]),
    (3, 3, [
        (3, 2, 1), (3, 1, 2), (2, 3, 1), (2, 2, 100), (2, 1, 100), (1, 3, 2), (1, 2, 100), (1, 1, 100),
    ]),
]
# (H, W, N, 盤面の形, 重みの出し方)。本番のケースのうち例のあとに並べる。
PLANS = [
    (1, 1, 1, "cover", "max"),
    (2, 2, 4, "cover", "random"),
    (2, 2, 3, "cover", "equal"),
    (1, MAX_HW, MAX_N, "cover", "random"),
    (MAX_HW, 1, MAX_N, "cover", "max"),
    (MAX_HW, MAX_HW, MAX_N, "cover", "random"),  # 置換行列
    (32, 32, MAX_N, "cover", "random"),
    (32, 32, MAX_N, "cover", "ties"),
    (32, 32, MAX_N, "cover", "extreme"),
    (32, 32, MAX_N, "cover", "max"),
    (10, 100, MAX_N, "cover", "random"),
    (40, 25, MAX_N, "cover", "ties"),
    (500, 500, MAX_N, "cover", "random"),
    (100, 100, MAX_N, "cover", "random"),
    (300, 300, MAX_N, "cross", "random"),
    (600, 600, MAX_N, "cover", "extreme"),
    (999, 2, MAX_N, "cover", "random"),
    (500, 500, MAX_N, "cycle", "random"),
    (500, 500, MAX_N, "cycle", "ties"),
]


def weights(rng: random.Random, n: int, how: str) -> list[int]:
    if how == "random":
        return [rng.randint(1, MAX_C) for _ in range(n)]
    if how == "max":
        return [MAX_C] * n
    if how == "equal":
        return [rng.randint(1, MAX_C)] * n
    if how == "ties":
        return [rng.randint(1, 10) for _ in range(n)]
    assert how == "extreme"
    return [rng.choice([1, MAX_C]) for _ in range(n)]


def board(rng: random.Random, h: int, w: int, n: int, shape: str) -> list[tuple[int, int]]:
    """0 始まりのマスを n 個返す。どの行とどの列にも 1 つ以上ある。"""
    assert max(h, w) <= n <= h * w
    rows, cols = list(range(h)), list(range(w))
    rng.shuffle(rows)
    rng.shuffle(cols)
    if shape == "cycle":
        # 行 rows[i] に列 cols[i] と cols[i + 1] の駒を置き、長さ 2h の 1 つの閉路にする。どの行とどの列も駒が 2 つ。
        assert h == w and n == 2 * h
        out = [(rows[i], cols[i]) for i in range(h)] + [(rows[i], cols[(i + 1) % w]) for i in range(h)]
        rng.shuffle(out)
        return out
    if shape == "cross":
        # 1 行と 1 列を埋めれば、どの行とどの列にもある。
        r0, c0 = rows[0], cols[0]
        cells = {(r0, c) for c in range(w)} | {(r, c0) for r in range(h)}
    else:
        # i 番目の駒を (rows[i mod h], cols[i mod w]) に置けば、どの行とどの列にもある。
        cells = {(rows[i % h], cols[i % w]) for i in range(max(h, w))}
    free = [(r, c) for r in range(h) for c in range(w) if (r, c) not in cells] if h * w <= 10**6 else None
    if free is not None:
        cells |= set(rng.sample(free, n - len(cells)))
    while len(cells) < n:
        cells.add((rng.randrange(h), rng.randrange(w)))
    out = sorted(cells)
    rng.shuffle(out)
    return out


def small_case(rng: random.Random, seed: int) -> tuple[int, int, list[tuple[int, int, int]]]:
    """愚直解は O(max(H, W) 4^min(H, W))。"""
    if seed >= 2000:
        lo, hi = rng.randint(1, 9), rng.randint(1, 40)
        h, w = (lo, hi) if rng.random() < 0.5 else (hi, lo)
        n = rng.randint(max(h, w), min(h * w, 300))
    else:
        h, w = rng.randint(1, 6), rng.randint(1, 6)
        n = rng.randint(max(h, w), h * w)
    how = rng.choice(["random", "equal", "ties", "extreme", "small"])
    cells = board(rng, h, w, n, "cover")
    cs = [rng.randint(1, 5) for _ in range(n)] if how == "small" else weights(rng, n, how)
    return h, w, [(r + 1, c + 1, x) for (r, c), x in zip(cells, cs)]


def case_for(seed: int, rng: random.Random) -> tuple[int, int, list[tuple[int, int, int]]]:
    if seed >= 1000:
        return small_case(rng, seed)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    h, w, n, shape, how = PLANS[seed - len(SAMPLES)]
    cells = board(rng, h, w, n, shape)
    return h, w, [(r + 1, c + 1, x) for (r, c), x in zip(cells, weights(rng, n, how))]


def main() -> None:
    seed = int(sys.argv[1])
    h, w, pieces = case_for(seed, random.Random(seed))
    n = len(pieces)
    assert 1 <= h <= MAX_HW and 1 <= w <= MAX_HW and 1 <= n <= MAX_N
    assert all(1 <= a <= h and 1 <= b <= w and 1 <= c <= MAX_C for a, b, c in pieces)
    assert len({(a, b) for a, b, _ in pieces}) == n
    assert {a for a, _, _ in pieces} == set(range(1, h + 1)) and {b for _, b, _ in pieces} == set(range(1, w + 1))
    out = [f"{h} {w} {n}"] + [f"{a} {b} {c}" for a, b, c in pieces]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
