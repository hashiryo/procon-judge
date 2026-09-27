"""abc127-f (Absolute Minima) の入力を作る。Q と Q 個の質問 (1 a b か 2) を出す。

seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース、Q = 1000 のランダム、Q = 2 × 10^5 の列。
列は、更新と評価の交互でランダムな値、a が増える列、a が大きい値と小さい値の交互 (中央値が
行き来する)、更新だけのあとに評価 1 回、更新 1 回のあとに評価だけ、a = ±10^9 の交互で
b = 10^9 (最小値が 4 × 10^14 近くになる)。全部同じ a と数種類の a は、折れ点が少なくて速いので
Q = 10^5 にする (データを 30 MB ほどに収めるため)。
Q = 1 は更新だけなので、出力は空になる。
seed が 1000 以上なら、評価のたびに折れ点を全部試す愚直解で解ける小さい入力を出す
(pj testdata crosscheck 用)。2000 以上なら、愚直解でまだ解ける Q = 5000 までの入力を出す。
"""

import random
import sys

MAX_Q = 2 * 10**5
MAX_V = 10**9

SAMPLES = [
    [(1, 4, 2), (2,), (1, 1, -8), (2,)],
    [(1, -MAX_V, MAX_V)] * 3 + [(2,)],
]
FIXED = [
    [(1, -MAX_V, -MAX_V)],  # 評価が無いので出力は空
    [(1, MAX_V, MAX_V), (2,)],
    [(1, -MAX_V, -MAX_V), (2,)],
    [(1, MAX_V, 0), (1, -MAX_V, 0), (2,)],  # 最小にする x は区間なので、左端の -10^9
    [(1, 0, 0), (2,), (1, 0, 0), (2,), (1, 1, 0), (2,), (1, -1, 0), (2,)],
    [(1, 3, 1), (1, 1, 1), (1, 2, 1), (1, 2, -1), (2,), (2,)],
]
# (Q, 列の出し方)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    (1000, "alternate"),
    (1000, "mix"),
    (MAX_Q, "alternate"),
    (MAX_Q, "increasing"),
    (MAX_Q, "zigzag"),
    (10**5, "same"),
    (10**5, "few"),
    (MAX_Q, "updates"),
    (MAX_Q, "evals"),
    (MAX_Q, "extreme"),
]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)


def value(rng: random.Random) -> int:
    return rng.randint(-MAX_V, MAX_V)


def small_value(rng: random.Random, how: str) -> int:
    if how == "tiny":
        return rng.randint(-3, 3)
    if how == "edge":
        return rng.choice([-MAX_V, -MAX_V + 1, -1, 0, 1, MAX_V - 1, MAX_V])
    return value(rng)


def plan_queries(rng: random.Random, q: int, how: str) -> list[tuple[int, ...]]:
    if how in ("alternate", "increasing", "zigzag"):
        # 更新と評価の交互。
        k = q // 2
        if how == "alternate":
            a = [value(rng) for _ in range(k)]
        elif how == "increasing":
            a = sorted(value(rng) for _ in range(k))
        else:
            a = [(-MAX_V + i if i % 2 == 0 else MAX_V - i) for i in range(k)]
        out = []
        for x in a:
            out += [(1, x, value(rng)), (2,)]
        return out
    if how == "same":
        x = value(rng)
        return [(1, x, value(rng)) if i == 0 or rng.random() < 0.5 else (2,) for i in range(q)]
    if how == "few":
        xs = [value(rng) for _ in range(5)]
        return [(1, rng.choice(xs), value(rng)) if i == 0 or rng.random() < 0.5 else (2,) for i in range(q)]
    if how == "updates":
        return [(1, value(rng), value(rng)) for _ in range(q - 1)] + [(2,)]
    if how == "evals":
        return [(1, rng.randint(-9, 9), rng.randint(-9, 9))] + [(2,)] * (q - 1)
    if how == "extreme":
        # a = ±10^9 の交互で、b は全部 10^9。評価は 1000 回に 1 回と、最後に 1 回。
        out = [(1, MAX_V if i % 2 else -MAX_V, MAX_V) if i % 1000 else (2,) for i in range(1, q)]
        return out + [(2,)]
    assert how == "mix"
    return [(1, value(rng), value(rng)) if i == 0 or rng.random() < 0.5 else (2,) for i in range(q)]


def small_queries(rng: random.Random, q: int) -> list[tuple[int, ...]]:
    """愚直解で解ける大きさ。値は、狭い範囲 (同じ値だらけ)、端の値、ランダムを混ぜる。"""
    how = rng.choice(["tiny", "edge", "random"])
    p = rng.choice([0.2, 0.5, 0.8])
    return [
        (1, small_value(rng, how), small_value(rng, how)) if i == 0 or rng.random() >= p else (2,)
        for i in range(q)
    ]


def case_for(seed: int, rng: random.Random) -> list[tuple[int, ...]]:
    if seed >= 1000:
        return small_queries(rng, rng.randint(100, 5000) if seed >= 2000 else rng.randint(1, 60))
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    q, how = PLANS[seed - len(FIXED)]
    return plan_queries(rng, q, how)


def main() -> None:
    seed = int(sys.argv[1])
    queries = case_for(seed, random.Random(seed))
    assert 1 <= len(queries) <= MAX_Q and queries[0][0] == 1
    assert all(q == (2,) or (len(q) == 3 and q[0] == 1 and all(-MAX_V <= v <= MAX_V for v in q[1:])) for q in queries)
    out = [str(len(queries))] + [" ".join(map(str, q)) for q in queries]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
