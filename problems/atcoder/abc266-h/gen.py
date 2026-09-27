"""abc266-h (Snuke Panic (2D)) の入力を作る。N と、N 行の T X Y A を出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、N = 10^5 のランダム。
i から j へ移れるのは Y_i <= Y_j かつ |X_j - X_i| + (Y_j - Y_i) <= T_j - T_i のとき。
点の置き方は、一様、原点から届く点だけ、時刻に比べて座標が小さいもの (Y の順だけで決まる)、
ほぼ届く道筋に沿ったもの、同じ位置に時刻だけ違うもの (全部取れて答えが 10^14)、Y = 0 の直線、
時刻が数種類だけ、小さい格子に詰めたもの (変換した座標が重なる)。
seed が 1000 以上なら、全部の組を調べる O(N^2) の DP で解ける小さい入力を出す
(pj testdata crosscheck 用)。
"""

import random
import sys

MAX_N = 10**5
MAX_T = 10**9
MAX_C = 10**9
MAX_A = 10**9

Snuke = tuple[int, int, int, int]

SAMPLES = [
    [(1, 0, 0, 100), (3, 2, 1, 10), (5, 3, 1, 1)],
    [(100, 0, 1, 1), (200, 1, 0, 10)],
    [
        (797829355, 595605750, 185676190, 353195922),
        (913575467, 388876063, 395940406, 533206504),
        (810900084, 201398242, 159760440, 87027328),
        (889089200, 220046203, 85488350, 325976483),
        (277429832, 161055688, 73308100, 940778720),
        (927999455, 429014248, 477195779, 174616807),
        (673419335, 415891345, 81019893, 286986530),
        (989248231, 147792453, 417536200, 219371588),
        (909664305, 22150727, 414107912, 317441890),
        (988670052, 140275628, 468278658, 67181740),
    ],
]
# 角のケース。原点から届く端と届かない端、同じ位置、同じ時刻、ちょうど間に合う移動、Y が減る移動。
FIXED = [
    [(1, 0, 0, 1)],
    [(1, 1, 1, MAX_A)],
    [(MAX_T, MAX_C, 0, MAX_A)],
    [(MAX_T, 0, MAX_C, MAX_A)],
    [(MAX_T, MAX_C, MAX_C, MAX_A)],
    [(1, 0, 0, 5), (2, 0, 0, 7), (MAX_T, 0, 0, MAX_A)],
    [(5, 1, 0, 3), (5, 0, 1, 4), (5, 2, 2, 100)],
    [(3, 3, 0, 1), (6, 0, 0, 1)],
    [(2, 1, 1, 1), (4, 1, 0, 1)],
]
# (点の置き方, A の出し方, N)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    ("uniform", "random", MAX_N),
    ("reachable", "random", MAX_N),
    ("near_time", "random", MAX_N),
    ("chain", "random", MAX_N),
    ("same_pos", "max", MAX_N),
    ("y_zero", "random", MAX_N),
    ("same_t", "random", MAX_N),
    ("grid", "random", MAX_N),
    ("grid", "small", 1000),
    ("reachable", "random", 1000),
    ("chain", "small", 1000),
]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)


def distinct(rng: random.Random, n: int, point) -> list[tuple[int, int, int]]:
    """point(rng) で (T, X, Y) を作り、重ならない n 個を集める。"""
    seen: set[tuple[int, int, int]] = set()
    out = []
    while len(out) < n:
        p = point(rng)
        if p not in seen:
            seen.add(p)
            out.append(p)
    return out


def chain(rng: random.Random, n: int, step: int) -> list[tuple[int, int, int]]:
    """ほぼ間に合う道筋。たまに 1 だけ遅れて、直前からは移れなくなる。"""
    t, x, y = 0, rng.randint(0, 10 * step), 0
    out = []
    for i in range(n):
        nx = max(0, min(MAX_C, x + rng.randint(-step, step)))
        ny = min(MAX_C, y + rng.randint(0, step))
        need = abs(nx - x) + ny - y if i else nx + ny
        t = max(t + 1, t + need + rng.choice([0, 0, 0, 1, 3, -1]))
        x, y = nx, ny
        out.append((t, x, y))
    return out


def points(rng: random.Random, how: str, n: int) -> list[tuple[int, int, int]]:
    if how == "uniform":
        return distinct(rng, n, lambda r: (r.randint(1, MAX_T), r.randint(0, MAX_C), r.randint(0, MAX_C)))
    if how == "reachable":
        def reachable(r: random.Random) -> tuple[int, int, int]:
            t = r.randint(1, MAX_T)
            x = r.randint(0, t)
            return t, x, r.randint(0, t - x)
        return distinct(rng, n, reachable)
    if how == "near_time":
        return distinct(rng, n, lambda r: (r.randint(1, MAX_T), r.randint(0, 1000), r.randint(0, 1000)))
    if how == "chain":
        return chain(rng, n, 1000 if n > 1000 else 5)
    if how == "same_pos":
        x, y = rng.randint(0, 1000), rng.randint(0, 1000)
        return [(t, x, y) for t in sorted(rng.sample(range(max(1, x + y), MAX_T + 1), n))]
    if how == "y_zero":
        return distinct(rng, n, lambda r: (r.randint(1, 3 * n), r.randint(0, n), 0))
    if how == "same_t":
        ts = [rng.randint(1, MAX_T) for _ in range(10)]
        return distinct(rng, n, lambda r: (r.choice(ts), r.randint(0, MAX_C), r.randint(0, MAX_C)))
    assert how == "grid"
    t_max, c_max = (200, 30) if n > 1000 else (40, 10)
    return distinct(rng, n, lambda r: (r.randint(1, t_max), r.randint(0, c_max), r.randint(0, c_max)))


def sizes(rng: random.Random, how: str, n: int) -> list[int]:
    if how == "max":
        return [MAX_A] * n
    if how == "small":
        return [rng.randint(1, 10) for _ in range(n)]
    return [rng.randint(1, MAX_A) for _ in range(n)]


def small_case(rng: random.Random) -> list[Snuke]:
    """愚直解の O(N^2) で解ける大きさ。座標は、移れる組が多い小さい範囲を多めにする。"""
    n = rng.randint(1, 8) if rng.random() < 0.3 else rng.randint(1, 60)
    how = rng.choice(["tiny", "tiny", "tiny", "uniform", "reachable", "near_time", "chain", "same_pos", "same_t"])
    if how == "tiny":
        t_max, c_max = rng.choice([(10, 3), (30, 10), (60, 20)])
        ps = distinct(rng, n, lambda r: (r.randint(1, t_max), r.randint(0, c_max), r.randint(0, c_max)))
    else:
        ps = points(rng, how, n)
    rng.shuffle(ps)
    return [(t, x, y, a) for (t, x, y), a in zip(ps, sizes(rng, rng.choice(["random", "small", "max"]), n))]


def case_for(seed: int, rng: random.Random) -> list[Snuke]:
    if seed >= 1000:
        return small_case(rng)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    if seed < len(SAMPLES) + len(FIXED):
        return FIXED[seed - len(SAMPLES)]
    how, how_a, n = PLANS[seed - len(SAMPLES) - len(FIXED)]
    ps = points(rng, how, n)
    rng.shuffle(ps)
    return [(t, x, y, a) for (t, x, y), a in zip(ps, sizes(rng, how_a, n))]


def main() -> None:
    seed = int(sys.argv[1])
    rows = case_for(seed, random.Random(seed))
    assert 1 <= len(rows) <= MAX_N
    assert all(1 <= t <= MAX_T and 0 <= x <= MAX_C and 0 <= y <= MAX_C and 1 <= a <= MAX_A for t, x, y, a in rows)
    assert len({(t, x, y) for t, x, y, _ in rows}) == len(rows)
    out = [str(len(rows))] + [f"{t} {x} {y} {a}" for t, x, y, a in rows]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
