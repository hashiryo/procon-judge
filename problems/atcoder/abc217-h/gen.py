"""abc217-h (Snuketoon) の入力を作る。N と、N 行の T D X を出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、N = 2 × 10^5 のランダム。
T は 1, 2, ..., N と詰めたもの (1 秒ずつしか動けない) と、10^9 までに散らしたもの
(一度に大きく動ける) の 2 通り。X は、ランダム、その時刻に居られる範囲 [-T, T] の中、
向きを交互に ±10^9 (例 3 を伸ばしたもので、答えが 10^14 を超える)、小さく揺れる列、
同じ値だらけを混ぜる。
seed が 1000 以上なら、1 秒ずつ位置を動かす愚直解で解ける、T_N が 1000 までの入力を出す
(pj testdata crosscheck 用)。
"""

import random
import sys

MAX_N = 2 * 10**5
MAX_T = 10**9
MAX_X = 10**9

SAMPLES = [
    [(1, 0, 3), (3, 1, 0), (4, 0, 6)],
    [(1, 0, 1), (6, 1, 1), (8, 0, -1)],
    [(1, 0, MAX_X), (2, 1, -MAX_X), (3, 0, MAX_X), (4, 1, -MAX_X), (5, 0, MAX_X)],
]
# 角のケース。N = 1 で、届く端と届かない端。
FIXED = [
    [(1, 0, 0)],
    [(1, 0, MAX_X)],  # 1 だけ近づいて 10^9 - 1
    [(1, 1, -MAX_X)],
    [(1, 1, MAX_X)],  # 当たらない
    [(MAX_T, 0, MAX_X)],  # ちょうど届く
    [(MAX_T, 1, -MAX_X)],
    [(MAX_T - 1, 0, MAX_X), (MAX_T, 1, -MAX_X)],
]
# (T の出し方, X の出し方, N)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    ("dense", "random", MAX_N),
    ("dense", "reachable", MAX_N),
    ("dense", "alternate", MAX_N),
    ("sparse", "random", MAX_N),
    ("sparse", "reachable", MAX_N),
    ("dense", "walk", MAX_N),
    ("dense", "few", MAX_N),
    ("sparse", "left_only", 5 * 10**4),
    ("dense", "reachable", 1000),
    ("sparse", "random", 1000),
]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)


def times(rng: random.Random, how: str, n: int, t_max: int = MAX_T) -> list[int]:
    if how == "dense":
        return list(range(1, n + 1))
    return sorted(rng.sample(range(1, t_max + 1), n))


def shots(rng: random.Random, how: str, ts: list[int]) -> list[tuple[int, int, int]]:
    if how == "random":
        return [(t, rng.randint(0, 1), rng.randint(-MAX_X, MAX_X)) for t in ts]
    if how == "reachable":
        return [(t, rng.randint(0, 1), rng.randint(-min(t, MAX_X), min(t, MAX_X))) for t in ts]
    if how == "alternate":
        return [(t, i % 2, MAX_X if i % 2 == 0 else -MAX_X) for i, t in enumerate(ts)]
    if how == "walk":
        # X が 1 秒に 3 まで動く列。追いかけきれないので、折れ点が最小の左右を行き来する。
        out, x = [], 0
        for t in ts:
            x = max(-MAX_X, min(MAX_X, x + rng.randint(-3, 3)))
            out.append((t, rng.randint(0, 1), x))
        return out
    if how == "few":
        # 同じ X が何度も来る。折れ点の座標が重なる。
        xs = [rng.randint(-5, 5) for _ in range(3)]
        return [(t, rng.randint(0, 1), rng.choice(xs)) for t in ts]
    assert how == "left_only"
    return [(t, 0, rng.randint(-MAX_X, MAX_X)) for t in ts]


def small_case(rng: random.Random) -> list[tuple[int, int, int]]:
    """愚直解が位置 [-T_N, T_N] を 1 秒ずつ動かせる大きさ。X は届く範囲と端の値を混ぜる。"""
    t_max = rng.choice([rng.randint(1, 20), rng.randint(1, 300), rng.randint(300, 1000)])
    n = rng.randint(1, min(t_max, rng.choice([5, 40, 300])))
    ts = sorted(rng.sample(range(1, t_max + 1), n))
    d_how = rng.choice(["random", "zero", "one", "alternate"])
    out = []
    for i, t in enumerate(ts):
        d = {"random": rng.randint(0, 1), "zero": 0, "one": 1, "alternate": i % 2}[d_how]
        x = rng.choice([
            rng.randint(-t - 2, t + 2),
            rng.randint(-5, 5),
            rng.randint(-MAX_X, MAX_X),
            rng.choice([-MAX_X, MAX_X, 1 - MAX_X, MAX_X - 1]),
        ])
        out.append((t, d, x))
    return out


def case_for(seed: int, rng: random.Random) -> list[tuple[int, int, int]]:
    if seed >= 1000:
        return small_case(rng)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    if seed < len(SAMPLES) + len(FIXED):
        return FIXED[seed - len(SAMPLES)]
    how_t, how_x, n = PLANS[seed - len(SAMPLES) - len(FIXED)]
    return shots(rng, how_x, times(rng, how_t, n))


def main() -> None:
    seed = int(sys.argv[1])
    rows = case_for(seed, random.Random(seed))
    assert 1 <= len(rows) <= MAX_N
    assert all(1 <= t <= MAX_T and d in (0, 1) and -MAX_X <= x <= MAX_X for t, d, x in rows)
    assert all(rows[i][0] < rows[i + 1][0] for i in range(len(rows) - 1))
    out = [str(len(rows))] + [f"{t} {d} {x}" for t, d, x in rows]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
