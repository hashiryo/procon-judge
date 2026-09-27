"""agc018-c (Coins) の入力を作る。X Y Z と、X+Y+Z 人ぶんの A B C を出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、X+Y+Z = 10^5 のランダム。
値は、ランダム、全員同じ、1 人の中で同じ、同点だらけ、10^9 の近くに固まったもの、端に散ったもの、
A - B で並べたものを混ぜる。10^9 の近くに固まった値では、双対の変数が大きくなる。
seed が 1000 以上なら、人ごとの DP で解ける小さい入力を出す (pj testdata crosscheck 用)。
2000 以上なら、DP でまだ解ける 300 人までの入力を出す。
"""

import random
import sys

MAX_N = 10**5
MAX_V = 10**9

SAMPLES = [
    ((1, 2, 1), [(2, 4, 4), (3, 2, 1), (7, 6, 7), (5, 2, 3)]),
    ((3, 3, 2), [(16, 17, 1), (2, 7, 5), (2, 16, 12), (17, 7, 7), (13, 2, 10), (12, 18, 3), (16, 15, 19), (5, 6, 2)]),
    ((6, 2, 4), [
        (33189, 87907, 277349742), (71616, 46764, 575306520), (8801, 53151, 327161251),
        (58589, 4337, 796697686), (66854, 17565, 289910583), (50598, 35195, 478112689),
        (13919, 88414, 103962455), (7953, 69657, 699253752), (44255, 98144, 468443709),
        (2332, 42580, 752437097), (39752, 19060, 845062869), (60126, 74101, 382963164),
    ]),
]
# (X, Y, Z の分け方, 値の出し方)。本番のケースのうち例のあとに並べる。
PLANS = [
    ("min", "random"),
    ("min", "max"),
    ("balanced", "random"),
    ("balanced", "max"),
    ("balanced", "same_person"),
    ("balanced", "ties"),
    ("balanced", "near_max"),
    ("balanced", "extreme"),
    ("balanced", "sorted"),
    ("gold", "random"),
    ("silver", "random"),
    ("bronze", "random"),
    ("gold", "near_max"),
    ("bronze", "extreme"),
    ("random", "random"),
    ("random", "near_max"),
    ("random", "ties"),
    ("random", "extreme"),
    ("tiny", "random"),
    ("tiny", "ties"),
]
COUNT = len(SAMPLES) + len(PLANS)


def split(rng: random.Random, how: str, n: int) -> tuple[int, int, int]:
    if how == "min":
        return 1, 1, 1
    if how == "balanced":
        return n // 3, n // 3, n - 2 * (n // 3)
    if how in ("gold", "silver", "bronze"):
        big = n - 2
        return {"gold": (big, 1, 1), "silver": (1, big, 1), "bronze": (1, 1, big)}[how]
    x = rng.randint(1, n - 2)
    y = rng.randint(1, n - 1 - x)
    return x, y, n - x - y


def values(rng: random.Random, how: str, n: int) -> list[tuple[int, int, int]]:
    if how == "random":
        return [tuple(rng.randint(1, MAX_V) for _ in range(3)) for _ in range(n)]
    if how == "max":
        return [(MAX_V, MAX_V, MAX_V)] * n
    if how == "same_person":
        return [(v, v, v) for v in (rng.randint(1, MAX_V) for _ in range(n))]
    if how == "ties":
        return [tuple(rng.randint(1, 3) for _ in range(3)) for _ in range(n)]
    if how == "near_max":
        return [tuple(MAX_V - rng.randint(0, 1000) for _ in range(3)) for _ in range(n)]
    if how == "extreme":
        return [tuple(rng.choice([1, 2, MAX_V - 1, MAX_V]) for _ in range(3)) for _ in range(n)]
    # sorted: A - B で並べ、C は一定。境目の選び方で間違えやすい。
    rows = [(rng.randint(1, MAX_V), rng.randint(1, MAX_V), MAX_V // 2) for _ in range(n)]
    return sorted(rows, key=lambda r: r[0] - r[1])


def case_for(seed: int, rng: random.Random) -> tuple[tuple[int, int, int], list[tuple[int, int, int]]]:
    if seed >= 1000:
        # 人ごとの DP で解ける大きさ。値の出し方は本番と同じものから選ぶ。
        n = rng.randint(13, 300) if seed >= 2000 else rng.randint(3, 12)
        how = rng.choice(["random", "max", "same_person", "ties", "near_max", "extreme", "sorted"])
        return split(rng, "random" if n > 3 else "min", n), values(rng, how, n)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    how_split, how_values = PLANS[seed - len(SAMPLES)]
    n = 3 if how_split == "min" else rng.randint(3, 50) if how_split == "tiny" else MAX_N
    return split(rng, how_split, n), values(rng, how_values, n)


def main() -> None:
    seed = int(sys.argv[1])
    (x, y, z), rows = case_for(seed, random.Random(seed))
    assert min(x, y, z) >= 1 and x + y + z == len(rows) <= MAX_N
    assert all(1 <= v <= MAX_V for row in rows for v in row)
    out = [f"{x} {y} {z}"] + [f"{a} {b} {c}" for a, b, c in rows]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
