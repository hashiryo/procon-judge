"""abc228-f (Stamp Game) の入力を作る。H W h_1 w_1 h_2 w_2 と H 行 W 列の A を出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、H = W = 1000 の盤面。
提出は h_2 と w_2 を h_1 と w_1 で切り詰めて、白いはんこを置ける位置の和を 2 次元のセグメント木に
入れる。h_2 = w_2 = 1 なら点が 10^6 個になり、黒いはんこが小さいほど質問も 10^6 回近くになるので、
(h_1, w_1) が (1, 1)、(2, 2)、(10, 10)、(500, 500) のものを入れる。白いはんこの方が大きい (0 か、
切り詰める) ものや、全部 10^9 (答えが 10^15 近く) も入れる。10^6 マスに大きな値を書くとデータが
大きくなるので、H = W = 1000 で 10^9 まで使うのは全部 10^9 のものだけにし、大きな値のランダムは
300 × 300 で試す。
seed が 1000 以上なら、黒と白の置き方を全部試す愚直解で解ける小さい盤面を出す (pj testdata crosscheck 用)。
2000 以上なら、愚直解でまだ解ける 40 × 40 までの盤面を出す。
"""

import random
import sys

MAX_HW = 1000
MAX_A = 10**9

SAMPLES = [
    ((3, 4, 2, 3, 3, 1), [[3, 1, 4, 1], [5, 9, 2, 6], [5, 3, 5, 8]]),
    ((3, 4, 2, 3, 3, 4), [[3, 1, 4, 1], [5, 9, 2, 6], [5, 3, 5, 8]]),
    (
        (10, 10, 3, 7, 2, 3),
        [
            [9, 7, 19, 7, 10, 4, 13, 9, 4, 8],
            [10, 15, 16, 3, 18, 19, 17, 12, 13, 2],
            [12, 18, 4, 9, 13, 13, 6, 13, 5, 2],
            [16, 12, 2, 14, 18, 17, 14, 7, 8, 12],
            [12, 13, 17, 12, 14, 15, 19, 7, 13, 15],
            [5, 2, 16, 10, 4, 6, 1, 2, 7, 8],
            [10, 14, 14, 10, 9, 13, 11, 4, 9, 19],
            [16, 12, 3, 19, 19, 6, 2, 19, 14, 20],
            [15, 3, 19, 19, 2, 10, 1, 4, 3, 15],
            [13, 20, 5, 6, 19, 1, 7, 17, 10, 19],
        ],
    ),
]
FIXED = [
    ((2, 2, 1, 1, 1, 1), [[1, 2], [3, 4]]),  # 白が黒を覆えるので 0
    ((2, 2, 2, 2, 1, 1), [[1, 2], [3, 4]]),
    ((2, 2, 2, 2, 2, 2), [[MAX_A, MAX_A], [MAX_A, MAX_A]]),
    ((2, 3, 1, 3, 2, 1), [[5, 1, 5], [1, 9, 1]]),  # 白の方が高いので切り詰める
    ((3, 3, 2, 2, 3, 1), [[1, 1, 1], [1, 1, 1], [1, 1, 1]]),
    ((3, 5, 3, 5, 1, 5), [[1, 2, 3, 4, 5], [5, 4, 3, 2, 1], [9, 9, 9, 9, 9]]),
    ((5, 5, 3, 3, 1, 1), [[MAX_A if (i, j) == (2, 2) else 1 for j in range(5)] for i in range(5)]),
    ((2, MAX_HW, 2, MAX_HW, 2, MAX_HW - 1), None),  # 白が黒をほとんど覆う
]
# (H, W, h_1, w_1, h_2, w_2, 値の出し方)。0 の大きさは乱数で決める。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    (300, 300, 0, 0, 0, 0, "large"),
    (300, 300, 150, 200, 20, 30, "large"),
    (MAX_HW, MAX_HW, MAX_HW, MAX_HW, 1, 1, "max"),  # 答えは (10^6 - 1) × 10^9
    (MAX_HW, MAX_HW, 1, 1, 1, 1, "digit"),
    (MAX_HW, MAX_HW, 2, 2, 1, 1, "digit"),
    (MAX_HW, MAX_HW, 10, 10, 1, 1, "digit"),
    (MAX_HW, MAX_HW, 500, 500, 1, 1, "digit"),
    (MAX_HW, MAX_HW, MAX_HW, 2, 1, 1, "digit"),
    (MAX_HW, MAX_HW, 700, 300, 1, MAX_HW, "digit"),
    (MAX_HW, MAX_HW, 0, 0, 0, 0, "small"),
]


def grid(rng: random.Random, h: int, w: int, how: str) -> list[list[int]]:
    if how == "max":
        return [[MAX_A] * w for _ in range(h)]
    if how == "large":
        return [[rng.randint(1, MAX_A) for _ in range(w)] for _ in range(h)]
    if how == "small":
        return [[rng.randint(1, 999) for _ in range(w)] for _ in range(h)]
    if how == "edge":
        return [[rng.choice([1, 2, MAX_A - 1, MAX_A]) for _ in range(w)] for _ in range(h)]
    if how == "same":
        v = rng.randint(1, MAX_A)
        return [[v] * w for _ in range(h)]
    assert how == "digit"
    return [[rng.randint(1, 9) for _ in range(w)] for _ in range(h)]


def stamps(rng: random.Random, h: int, w: int) -> tuple[int, int, int, int]:
    return rng.randint(1, h), rng.randint(1, w), rng.randint(1, h), rng.randint(1, w)


def case_for(seed: int, rng: random.Random) -> tuple[tuple[int, ...], list[list[int]]]:
    if seed >= 1000:
        # 愚直解で解ける大きさ。はんこの大きさはランダムで、白の方が大きいものも混ぜる。
        limit = 40 if seed >= 2000 else 8
        h, w = rng.randint(2, limit), rng.randint(2, limit)
        how = rng.choice(["large", "small", "digit", "edge", "same"])
        return (h, w, *stamps(rng, h, w)), grid(rng, h, w, how)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        size, a = FIXED[seed]
        return size, a if a is not None else grid(rng, size[0], size[1], "large")
    h, w, h1, w1, h2, w2, how = PLANS[seed - len(FIXED)]
    if h1 == 0:
        h1, w1, h2, w2 = stamps(rng, h, w)
    return (h, w, h1, w1, h2, w2), grid(rng, h, w, how)


def main() -> None:
    seed = int(sys.argv[1])
    (h, w, h1, w1, h2, w2), a = case_for(seed, random.Random(seed))
    assert 2 <= h <= MAX_HW and 2 <= w <= MAX_HW and 1 <= h1 <= h and 1 <= h2 <= h and 1 <= w1 <= w and 1 <= w2 <= w
    assert len(a) == h and all(len(row) == w and all(1 <= v <= MAX_A for v in row) for row in a)
    out = [f"{h} {w} {h1} {w1} {h2} {w2}"] + [" ".join(map(str, row)) for row in a]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
