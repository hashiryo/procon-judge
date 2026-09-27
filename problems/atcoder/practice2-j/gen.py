"""practice2-j (Segment Tree) の入力を作る。N Q、A と Q 個のクエリを出す。

seed が 0 から count - 1 までは本番のケース。例の 1 つ、N = 1 などの角のケース、N と Q が 2 * 10^5 の
大きいもの。A は、ランダム、小さい値 (同じ値だらけ)、全部 0、全部最大、増える列、減る列、階段、
最後の 1 つだけ大きいものを混ぜる。クエリは、3 種類を同じくらいずつ、種類 3 だけ、更新が半分、
全体の最大を聞き続けるもの。種類 3 の V は、A にある値やその前後、0、どの A より大きい値を混ぜるので、
答えが X のすぐ近くのものも、遠くのものも、N + 1 のものも出る。最後の 1 つだけ大きい A に種類 3
だけを聞くと、答えがいつも N か N + 1 になり、探す範囲がいちばん長くなる。
seed が 1000 以上なら、配列をそのまま持って毎回端から調べる愚直解で解ける小さい入力を出す
(pj testdata crosscheck 用)。
"""

import random
import sys

MAX_N = 2 * 10**5
MAX_Q = 2 * 10**5
MAX_V = 10**9

SAMPLES = [
    ([1, 2, 3, 2, 1], [(2, 1, 5), (3, 2, 3), (1, 3, 1), (2, 2, 4), (3, 1, 3)]),  # 例 1
]
FIXED = [
    ([0], [(2, 1, 1)]),
    ([5], [(3, 1, 5), (3, 1, 6), (1, 1, MAX_V), (2, 1, 1), (3, 1, 0), (3, 1, MAX_V)]),
    ([MAX_V, 0], [(3, 2, 1), (3, 1, MAX_V), (2, 1, 2), (2, 2, 2), (1, 2, MAX_V), (3, 2, MAX_V)]),
    ([0] * 5, [(3, x, 0) for x in range(1, 6)] + [(3, 1, 1), (1, 5, 1), (3, 1, 1), (2, 1, 4)]),
]
# (N, Q, A の出し方, クエリの出し方)。本番のケースのうち FIXED のあとに並べる。
# N と Q がともに最大のものは、データが大きくなるので 4 つにしてある。
PLANS = [
    (MAX_N, MAX_Q, "random", "mixed"),
    (MAX_N, MAX_Q, "small", "updates"),
    (MAX_N, MAX_Q, "staircase", "search"),
    (MAX_N, MAX_Q, "far", "search"),  # 答えがいつも N か N + 1
    (MAX_N, 1000, "decreasing", "search"),
    (50000, 50000, "increasing", "mixed"),
    (1000, 1000, "max", "mixed"),
    (1000, 50000, "zero", "mixed"),
    (1000, 50000, "small", "whole"),
    (None, None, "random", "mixed"),  # N と Q は 5 * 10^4 までのランダム
    (None, None, "small", "search"),
    (None, None, "increasing", "updates"),
]
ARRAYS = ["random", "small", "zero", "max", "increasing", "decreasing", "staircase", "far"]
QUERIES = ["mixed", "search", "updates", "whole"]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)


def array(rng: random.Random, n: int, how: str, top: int) -> list[int]:
    """0 <= A_i <= top の列。"""
    if how == "small":
        return [rng.randint(0, min(top, 10)) for _ in range(n)]
    if how == "zero":
        return [0] * n
    if how == "max":
        return [top] * n
    if how in ("increasing", "decreasing"):
        a = sorted(rng.randint(0, top) for _ in range(n))
        return a if how == "increasing" else a[::-1]
    if how == "staircase":
        step = max(n // 200, 1)
        return [min(i // step, top) for i in range(n)]
    if how == "far":
        return [0] * (n - 1) + [top]
    return [rng.randint(0, top) for _ in range(n)]


def queries(rng: random.Random, a: list[int], q: int, how: str, top: int) -> list[tuple[int, int, int]]:
    n, hi = len(a), max(a)

    def value() -> int:
        kind = rng.randrange(5)
        if kind == 0:
            return rng.randint(0, top)
        if kind == 1:
            return 0
        if kind == 2:
            return min(hi + rng.randint(1, 3), MAX_V)  # どの A より大きい (A が最大でなければ)
        return min(max(rng.choice(a) + rng.randint(-1, 1), 0), MAX_V)  # A にある値の前後

    types = {"mixed": [1, 2, 3], "search": [3], "updates": [1, 1, 2, 3], "whole": [1, 2, 2]}[how]
    # 最後の 1 つだけ大きい A に種類 3 だけを聞くとき。V を 1 から top にすると答えはいつも N、
    # top + 1 なら N + 1 になる。
    far = how == "search" and n > 1 and a[-1] == top > 0 and hi == top and a.count(0) == n - 1
    out = []
    for _ in range(q):
        t = rng.choice(types)
        if t == 2:
            if how == "whole":
                out.append((2, 1, n))
            else:
                left, right = sorted((rng.randint(1, n), rng.randint(1, n)))
                out.append((2, left, right))
        elif far:
            out.append((3, rng.randint(1, n), rng.choice([1, top, rng.randint(1, top), min(top + 1, MAX_V)])))
        else:
            out.append((t, rng.randint(1, n), value()))
    return out


def case_for(seed: int, rng: random.Random) -> tuple[list[int], list[tuple[int, int, int]]]:
    if seed >= 1000:
        # 愚直解が毎回配列を端から調べても間に合う大きさ。値の幅は小さいものも混ぜる。
        n, q = rng.randint(1, 30), rng.randint(1, 30)
        top = rng.choice([1, 3, 10, MAX_V])
        a = array(rng, n, rng.choice(ARRAYS), top)
        return a, queries(rng, a, q, rng.choice(QUERIES), top)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    n, q, how_a, how_q = PLANS[seed - len(FIXED)]
    n, q = n or rng.randint(1, 50000), q or rng.randint(1, 50000)
    top = 1 if how_a == "far" else MAX_V  # far は A_N = 1 で、V = 2 なら答えが N + 1
    a = array(rng, n, how_a, top)
    return a, queries(rng, a, q, how_q, top)


def main() -> None:
    seed = int(sys.argv[1])
    a, qs = case_for(seed, random.Random(seed))
    n = len(a)
    assert 1 <= n <= MAX_N and 1 <= len(qs) <= MAX_Q and all(0 <= x <= MAX_V for x in a)
    for t, x, y in qs:
        assert (t in (1, 3) and 1 <= x <= n and 0 <= y <= MAX_V) or (t == 2 and 1 <= x <= y <= n)
    out = [f"{n} {len(qs)}", " ".join(map(str, a))] + [f"{t} {x} {y}" for t, x, y in qs]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
