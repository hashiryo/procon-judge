"""self-kd-tree-nearest の入力を作る。N Q と、N 個の点、Q 個のクエリの点を出す。

seed が 0 から count - 1 までは本番のケース。N と Q を 1 から 2 × 10^5 まで動かし、点の置き方を
一様なランダム、格子、横一列、縦一列、斜め、塊、同じ点だらけ、端に寄せたものから選ぶ。
クエリは、点と同じ場所、点の近く、一様なランダムを混ぜる。

seed が 1000 以上なら、全部の点を調べる愚直解で解ける小さい入力を出す (pj testdata crosscheck 用)。
2000 以上なら、さらに小さくする。
"""

import random
import sys

MAX_N = 2 * 10**5
MAX_Q = 2 * 10**5
MAX_C = 10**9

Case = tuple[int, list[tuple[int, int]], list[tuple[int, int]]]

# 手で置いた小さいケース。
SAMPLES: list[Case] = [
    (1, [(5, 5)], [(5, 5)]),                     # 点が 1 つ、同じ場所
    (3, [(0, 0), (0, 0), (10, 10)], [(0, 0), (5, 5), (10, 10)]),  # 同じ点
    (2, [(0, MAX_C), (MAX_C, 0)], [(0, 0), (MAX_C, MAX_C), (0, MAX_C)]),  # 端
    (4, [(1, 1), (2, 2), (3, 3), (4, 4)], [(0, 4), (4, 0), (2, 3)]),  # 斜め
]


def small_case(rng: random.Random, tiny: bool) -> Case:
    n = rng.randint(1, 50 if tiny else 200)
    q = rng.randint(1, 50 if tiny else 200)
    c = rng.choice([3, 1000, MAX_C])
    pts = [(rng.randint(0, c), rng.randint(0, c)) for _ in range(n)]
    qs = []
    for _ in range(q):
        if pts and rng.random() < 0.3:
            p = rng.choice(pts)
            qs.append(p)
        else:
            qs.append((rng.randint(0, c), rng.randint(0, c)))
    return n, pts, qs


def plan(seed: int, rng: random.Random) -> Case:
    kind = seed % 7
    if kind == 0:  # 一様なランダム
        n = rng.choice([10, 1000, 50000, MAX_N])
        q = rng.choice([10, 1000, 50000, MAX_Q])
        pts = [(rng.randint(0, MAX_C), rng.randint(0, MAX_C)) for _ in range(n)]
    elif kind == 1:  # 格子
        side = max(1, int((rng.choice([100, 1000, MAX_N])) ** 0.5))
        pts = [(i * (MAX_C // side), j * (MAX_C // side)) for i in range(side) for j in range(side)]
        n = len(pts)
        q = rng.choice([1000, MAX_Q])
    elif kind == 2:  # 横一列 (y が同じ)
        n = rng.choice([1000, MAX_N])
        pts = [(rng.randint(0, MAX_C), 12345) for _ in range(n)]
        q = rng.choice([1000, MAX_Q])
    elif kind == 3:  # 縦一列 (x が同じ)
        n = rng.choice([1000, MAX_N])
        pts = [(777, rng.randint(0, MAX_C)) for _ in range(n)]
        q = rng.choice([1000, MAX_Q])
    elif kind == 4:  # 同じ点だらけ
        n = rng.choice([1000, MAX_N])
        base = [(rng.randint(0, 100), rng.randint(0, 100)) for _ in range(5)]
        pts = [rng.choice(base) for _ in range(n)]
        q = rng.choice([1000, MAX_Q])
    elif kind == 5:  # 塊
        centers = [(rng.randint(0, MAX_C), rng.randint(0, MAX_C)) for _ in range(8)]
        n = rng.choice([1000, MAX_N])
        pts = []
        for _ in range(n):
            cx, cy = rng.choice(centers)
            pts.append((min(MAX_C, max(0, cx + rng.randint(-1000, 1000))), min(MAX_C, max(0, cy + rng.randint(-1000, 1000)))))
        q = rng.choice([1000, MAX_Q])
    else:  # 端に寄せたもの
        n = rng.choice([1000, MAX_N])
        pts = [(rng.choice([0, MAX_C]), rng.choice([0, MAX_C])) for _ in range(n)]
        q = rng.choice([1000, MAX_Q])
    qs = []
    for _ in range(q):
        r = rng.random()
        if pts and r < 0.3:
            qs.append(rng.choice(pts))
        elif pts and r < 0.6:
            p = rng.choice(pts)
            qs.append((min(MAX_C, max(0, p[0] + rng.randint(-100, 100))), min(MAX_C, max(0, p[1] + rng.randint(-100, 100)))))
        else:
            qs.append((rng.randint(0, MAX_C), rng.randint(0, MAX_C)))
    return len(pts), pts, qs


def case_for(seed: int, rng: random.Random) -> Case:
    if seed >= 1000:
        return small_case(rng, seed >= 2000)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    return plan(seed - len(SAMPLES), rng)


def main() -> None:
    seed = int(sys.argv[1])
    n, pts, qs = case_for(seed, random.Random(seed))
    assert 1 <= n <= MAX_N and 1 <= len(qs) <= MAX_Q
    assert all(0 <= x <= MAX_C and 0 <= y <= MAX_C for x, y in pts)
    assert all(0 <= x <= MAX_C and 0 <= y <= MAX_C for x, y in qs)
    out = [f"{n} {len(qs)}"]
    out += [f"{x} {y}" for x, y in pts]
    out += [f"{x} {y}" for x, y in qs]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
