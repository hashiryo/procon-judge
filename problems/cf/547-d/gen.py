"""cf-547-d (Mike and Fish) の入力を作る。n (1 <= n <= 2 * 10^5) と、相異なる n 個の点 x y (1 <= x, y <= 2 * 10^5) を出す。

答えは、点ごとに r か b を塗り、どの縦の線と横の線でも r と b の数の差を 1 以下にする塗り方 (いつもある)。塗り方は 1 つに
決まらないので、checker.cpp が入力と提出の出力だけで判定する。x 座標と y 座標を頂点、点を辺にした二部グラフの Euler 分割になる。
seed が 0 から count - 1 までは本番のケース。問題文の例 2 つ、n = 1、小さいランダム、制約いっぱいのランダム、1 本の横の線と縦の線に全部の点を並べた
もの、格子、階段 (長い閉路と道)、列の少ないもの、どの線にも 1 点だけのもの、狭い範囲に詰めたもの。seed が 1000 以上なら、
n <= 12 で座標が 4 以下の入力を出す (全部の塗り方を試す愚直解 brute.cpp 用)。
"""

import random
import sys

N_MAX = 200000
C_MAX = 200000


def distinct_points(rng: random.Random, n: int, xs: int, ys: int) -> list[tuple[int, int]]:
    """[1, xs] x [1, ys] から相異なる n 点を選ぶ。"""
    assert n <= xs * ys
    if n * 3 > xs * ys:
        cells = rng.sample(range(xs * ys), n)
        return [(c % xs + 1, c // xs + 1) for c in cells]
    seen = set()
    while len(seen) < n:
        seen.add((rng.randint(1, xs), rng.randint(1, ys)))
    pts = list(seen)
    rng.shuffle(pts)
    return pts


def staircase(rng: random.Random, n: int) -> list[tuple[int, int]]:
    """(i, i) と (i, i + 1) を交互に並べた階段。点が 1 本の長い道になる。"""
    pts = []
    i = 1
    while len(pts) < n:
        pts.append((i, i))
        if len(pts) < n:
            pts.append((i, i + 1))
        i += 1
    rng.shuffle(pts)
    return pts


def grid(rng: random.Random, side: int) -> list[tuple[int, int]]:
    pts = [(x, y) for x in range(1, side + 1) for y in range(1, side + 1)]
    rng.shuffle(pts)
    return pts


def small(rng: random.Random) -> list[tuple[int, int]]:
    n = rng.randint(1, 12)
    side = rng.choice([2, 3, 4])
    return distinct_points(rng, min(n, side * side), side, side)


PLANS = [
    lambda rng: [(1, 1), (1, 2), (2, 1), (2, 2)],
    lambda rng: [(1, 1), (1, 2), (2, 1)],
    lambda rng: [(1, 1)],
    lambda rng: distinct_points(rng, 30, 6, 6),
    lambda rng: distinct_points(rng, N_MAX, C_MAX, C_MAX),
    lambda rng: [(x, 1) for x in rng.sample(range(1, C_MAX + 1), N_MAX)],
    lambda rng: [(7, y) for y in rng.sample(range(1, C_MAX + 1), N_MAX)],
    lambda rng: grid(rng, 447),
    lambda rng: staircase(rng, N_MAX),
    lambda rng: distinct_points(rng, N_MAX, 10, 20000),
    lambda rng: [(i, i) for i in rng.sample(range(1, C_MAX + 1), N_MAX)],
    lambda rng: distinct_points(rng, N_MAX, 500, 500),
]


def case_for(seed: int) -> list[tuple[int, int]]:
    rng = random.Random(seed)
    if seed >= 1000:
        return small(rng)
    return PLANS[seed](rng)


def main() -> None:
    pts = case_for(int(sys.argv[1]))
    n = len(pts)
    assert 1 <= n <= N_MAX and len(set(pts)) == n
    assert all(1 <= x <= C_MAX and 1 <= y <= C_MAX for x, y in pts)
    sys.stdout.write(f"{n}\n" + "".join(f"{x} {y}\n" for x, y in pts))


if __name__ == "__main__":
    main()
