"""abc174-f (Range Set Query) の入力を作る。N Q、色 c、Q 個の区間 l r を出す。

seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース、N = 5 * 10^5 あたりのいろいろな色の並び。
N = Q = 5 * 10^5 のランダムは 1 つだけにして、ほかは Q = 5 * 10^4 にする (データが大きくなりすぎるため)。
色の並びは、ランダム、全部同じ (1 か N)、全部違う (順列)、2 色が交互、少ない色、同じ色が続く塊。
区間は、ランダム、全体、短いもの。RangeCountDistinct は前に同じ色が出た位置を WaveletMatrix に入れて
数えるので、色が全部違うと前の位置が無いものばかりになり、全部同じだと前の位置がすぐ隣になる。
seed が 1000 以上なら、区間ごとに色を数える愚直解で解ける小さい入力を出す (pj testdata crosscheck 用)。
2000 以上なら、愚直解でまだ解ける N, Q <= 3000 の入力を出す。
"""

import random
import sys

MAX_N = 5 * 10**5
MAX_Q = 5 * 10**5

SAMPLES = [
    (4, [1, 2, 1, 3], [(1, 3), (2, 4), (3, 3)]),
    (10, [2, 5, 6, 5, 2, 1, 7, 9, 7, 2], [(5, 5), (2, 4), (6, 7), (2, 2), (7, 8), (7, 9), (1, 8), (6, 9), (8, 10), (6, 8)]),
]
# (N, Q, 色の並び, 区間の選び方)。本番のケースのうち例のあとに並べる。Q = 0 は全部の区間を 1 回ずつ。
PLANS = [
    (1, 1, "same", "random"),
    (2, 0, "same_max", "random"),
    (2, 0, "permutation", "random"),
    (5, 0, "alternate", "random"),
    (300, 0, "few", "random"),
    (300, 0, "random", "random"),
    (100, 5000, "random", "random"),
    (1000, 1000, "few", "short"),
    (2000, 2000, "same_max", "random"),
    (3000, 3000, "runs", "whole"),
    (4096, 4096, "random", "random"),  # WaveletMatrix の段の数が変わる境目
    (4095, 4095, "permutation", "random"),
    (MAX_N, MAX_Q, "random", "random"),
    (MAX_N, 50000, "permutation", "random"),
    (MAX_N, 50000, "same", "random"),
    (MAX_N, 50000, "alternate", "random"),
    (MAX_N, 50000, "few", "whole"),
    (262144, 50000, "random", "short"),
    (200000, 50000, "runs", "random"),
]
COUNT = len(SAMPLES) + len(PLANS)
COLORINGS = ["random", "same", "same_max", "permutation", "alternate", "few", "runs"]


def colors(rng: random.Random, n: int, how: str) -> list[int]:
    if how == "random":
        return [rng.randint(1, n) for _ in range(n)]
    if how == "same":
        return [1] * n
    if how == "same_max":
        return [n] * n
    if how == "permutation":
        c = list(range(1, n + 1))
        rng.shuffle(c)
        return c
    if how == "alternate":
        return [1 + i % 2 for i in range(n)]
    if how == "few":
        palette = rng.sample(range(1, min(n, 5) + 1), rng.randint(1, min(n, 5)))
        return [rng.choice(palette) for _ in range(n)]
    assert how == "runs"
    # 同じ色が続く塊を並べる。塊の長さは 1 から 20。
    c: list[int] = []
    while len(c) < n:
        c += [rng.randint(1, n)] * rng.randint(1, 20)
    return c[:n]


def queries(rng: random.Random, n: int, q: int, how: str) -> list[tuple[int, int]]:
    if q == 0:
        return [(l, r) for l in range(1, n + 1) for r in range(l, n + 1)]
    if how == "whole":
        return [(1, n)] * q
    if how == "short":
        out = []
        for _ in range(q):
            l = rng.randint(1, n)
            out.append((l, min(n, l + rng.randint(0, 9))))
        return out
    return [tuple(sorted((rng.randint(1, n), rng.randint(1, n)))) for _ in range(q)]


def case_for(seed: int, rng: random.Random) -> tuple[int, list[int], list[tuple[int, int]]]:
    if seed >= 1000:
        # 愚直解が区間ごとに数えても速い大きさ。
        limit = 3000 if seed >= 2000 else 30
        n, q = rng.randint(1, limit), rng.randint(1, limit)
        c = colors(rng, n, rng.choice(COLORINGS))
        return n, c, queries(rng, n, q, rng.choice(["random", "random", "whole", "short"]))
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    n, q, how_colors, how_queries = PLANS[seed - len(SAMPLES)]
    return n, colors(rng, n, how_colors), queries(rng, n, q, how_queries)


def main() -> None:
    seed = int(sys.argv[1])
    n, c, qs = case_for(seed, random.Random(seed))
    assert 1 <= n <= MAX_N and 1 <= len(qs) <= MAX_Q and len(c) == n
    assert all(1 <= x <= n for x in c) and all(1 <= l <= r <= n for l, r in qs)
    out = [f"{n} {len(qs)}", " ".join(map(str, c))] + [f"{l} {r}" for l, r in qs]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
