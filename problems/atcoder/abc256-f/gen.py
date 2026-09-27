"""abc256-f (Cumulative Cumulative Cumulative Sum) の入力を作る。N Q、A、Q 行のクエリを出す。

seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース、N = Q = 2 × 10^5 のランダム。
クエリは、更新と質問が半々、質問だけ、更新は x = 1 (後ろ全部に効く) で質問は x = N だけ、
x を 1 から順に動かすもの (スプレー木に有利な並び) を混ぜる。値が 998244353 の倍数や 10^9 の
ケースも入れる。N = 1 で Q = 2 × 10^5 と、N = 2 × 10^5 で Q = 1 の端も入れる。答えの無い入力に
ならないよう、質問は必ず 1 つは入れる。出力が大きいので、N = Q = 2 × 10^5 は 4 ケースにする。
seed が 1000 以上なら、質問のたびに累積和を 3 回取り直す愚直解で解ける小さい入力を出す
(pj testdata crosscheck 用)。
"""

import random
import sys

MAX_N = 2 * 10**5
MAX_Q = 2 * 10**5
MAX_V = 10**9
P = 998244353

Query = tuple[int, ...]

SAMPLES = [
    ([1, 2, 3], [(2, 3), (1, 2, 0), (2, 3)]),
    ([P, P], [(2, 1)]),
]
FIXED = [
    ([0], [(2, 1)]),
    ([MAX_V], [(2, 1), (1, 1, 0), (2, 1), (1, 1, MAX_V), (2, 1)]),
    ([MAX_V] * 5, [(2, 5), (1, 1, 0), (2, 5), (1, 5, MAX_V), (2, 1), (2, 5)]),
    ([P, 0, P, 0], [(2, 4), (1, 2, P), (2, 4), (2, 1), (1, 1, 1), (2, 4)]),
    ([1] * 10, [(2, x) for x in range(1, 11)]),  # D_x = C(x + 2, 3)
    ([0, 0, 0], [(1, 3, 5), (2, 3), (2, 2), (1, 1, 1), (2, 1), (2, 3)]),
]
# (N, Q, クエリの出し方, 値の出し方)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    (MAX_N, MAX_Q, "mixed", "random"),
    (MAX_N, MAX_Q, "query_only", "random"),
    (MAX_N, MAX_Q, "ends", "random"),
    (MAX_N, MAX_Q, "mixed", "small"),
    (10**5, 10**5, "sequential", "random"),
    (1, MAX_Q, "mixed", "random"),
    (MAX_N, 1, "last", "max"),
    (5000, 5000, "mixed", "mixed"),
    (1000, 1000, "mixed", "mod"),
    (1000, 1000, "update_heavy", "random"),
    (1000, 1000, "sequential", "small"),
    (2, 1000, "mixed", "mod"),
]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)


def value(rng: random.Random, how: str) -> int:
    if how == "random":
        return rng.randint(0, MAX_V)
    if how == "small":
        return rng.randint(0, 9)
    if how == "mod":
        return rng.choice([0, P, 1, P - 1, P + 1, MAX_V])
    if how == "max":
        return MAX_V
    assert how == "mixed"
    return value(rng, rng.choice(["random", "small", "mod"]))


def queries(rng: random.Random, how: str, n: int, q: int, how_v: str) -> list[Query]:
    out: list[Query] = []
    for i in range(q):
        if how == "query_only":
            out.append((2, rng.randint(1, n)))
        elif how == "last":
            out.append((2, n))
        elif how == "ends":
            out.append((1, 1, value(rng, how_v)) if rng.random() < 0.5 else (2, n))
        elif how == "sequential":
            x = i // 2 % n + 1
            out.append((1, x, value(rng, how_v)) if i % 2 == 0 else (2, x))
        else:
            update = rng.random() < (0.9 if how == "update_heavy" else 0.5)
            x = rng.randint(1, n)
            out.append((1, x, value(rng, how_v)) if update else (2, x))
    if all(t[0] == 1 for t in out):
        out[-1] = (2, rng.randint(1, n))
    return out


def small_case(rng: random.Random) -> tuple[list[int], list[Query]]:
    """愚直解が O(NQ) で解ける大きさ。x は端を多めに選ぶ。"""
    n = rng.randint(1, 8) if rng.random() < 0.3 else rng.randint(1, 60)
    q = rng.randint(1, 8) if rng.random() < 0.3 else rng.randint(1, 300)
    how_v = rng.choice(["random", "small", "mod", "mixed"])
    a = [value(rng, how_v) for _ in range(n)]
    out: list[Query] = []
    for _ in range(q):
        x = rng.choice([1, n, rng.randint(1, n)])
        out.append((1, x, value(rng, how_v)) if rng.random() < 0.5 else (2, x))
    if all(t[0] == 1 for t in out):
        out[-1] = (2, rng.randint(1, n))
    return a, out


def case_for(seed: int, rng: random.Random) -> tuple[list[int], list[Query]]:
    if seed >= 1000:
        return small_case(rng)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    if seed < len(SAMPLES) + len(FIXED):
        return FIXED[seed - len(SAMPLES)]
    n, q, how, how_v = PLANS[seed - len(SAMPLES) - len(FIXED)]
    return [value(rng, how_v) for _ in range(n)], queries(rng, how, n, q, how_v)


def main() -> None:
    seed = int(sys.argv[1])
    a, qs = case_for(seed, random.Random(seed))
    n = len(a)
    assert 1 <= n <= MAX_N and 1 <= len(qs) <= MAX_Q and all(0 <= v <= MAX_V for v in a)
    assert all(
        (t[0] == 1 and len(t) == 3 and 1 <= t[1] <= n and 0 <= t[2] <= MAX_V)
        or (t[0] == 2 and len(t) == 2 and 1 <= t[1] <= n)
        for t in qs
    )
    out = [f"{n} {len(qs)}", " ".join(map(str, a))] + [" ".join(map(str, t)) for t in qs]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
