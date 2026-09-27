"""abc256-h (I like Query Problem) の入力を作る。N Q、a_1 ... a_N、Q 個のクエリを出す。

クエリは 1 L R x (区間を x で割って切り捨て)、2 L R y (区間を y にする)、3 L R (区間の和を出す)。
seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース、N = 5 × 10^5 と 10^5 のいろいろなクエリの列。
lib.cpp のセグメント木は、区間が全部同じ値のときだけ割り算をその場で済ませ、違う値が混ざると子へ降りる。
そこで、全体を 2 で割るクエリと 1 点を 10^5 にするクエリを交互に出すもの、短い区間をばらばらの値にしてから
大きい区間を割るもの、全体を y にしてから割り算を何回も重ねて遅延の合成 (割る数の積は 2^30 で打ち切る) を
使わせるもの、を入れる。ほかに、ランダム、小さい x だけ、大きい x だけ、代入が多いもの、和だけで値が全部 10^5
のもの (和が 5 × 10^10 になり int に収まらない)、N = 1000 に 10^5 個のクエリ。
入力が大きいので、N = 5 × 10^5 かつ Q = 10^5 は 3 ケースにして、残りは N = 10^5 か Q = 5 × 10^4 にする。
seed が 1000 以上なら、配列をそのまま書き換える愚直解で解ける小さい入力を出す (pj testdata crosscheck 用)。
2000 以上なら、愚直解でまだ解ける N, Q <= 2000 の入力を出す。
"""

import random
import sys

MAX_N = 5 * 10**5
MAX_Q = 10**5
MAX_V = 10**5

SAMPLES = [
    ([2, 5, 6], ["3 1 3", "1 2 3 2", "3 1 2", "2 1 2 3", "3 1 3"]),
    ([10, 3, 5, 20, 6, 7], ["3 1 6", "1 2 4 3", "3 1 3", "2 1 4 10", "3 3 6", "1 3 6 2", "2 1 4 5", "3 1 6",
                             "2 1 3 100", "1 2 5 6", "3 1 4"]),
]
FIXED = [
    ([1], ["3 1 1"]),
    ([MAX_V], ["1 1 1 2", "3 1 1", "1 1 1 100000", "3 1 1", "2 1 1 100000", "3 1 1"]),
    ([MAX_V] * 3, ["1 1 3 100000", "3 1 3", "1 2 2 2", "3 1 3", "2 1 3 1", "1 1 3 2", "3 1 3"]),
    ([7, 7, 7, 7], ["1 1 4 2", "1 2 3 3", "3 1 4", "2 2 2 5", "1 1 4 2", "3 1 4", "3 2 2"]),
]
# (N, Q, クエリの出し方)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    (MAX_N, MAX_Q, "random"),
    (MAX_N, MAX_Q, "div_all_point_assign"),
    (MAX_N, MAX_Q // 2, "sum_only_max"),
    (MAX_N, MAX_Q, "stripes"),
    (10**5, MAX_Q // 2, "small_x"),
    (10**5, MAX_Q // 2, "big_x"),
    (10**5, MAX_Q // 2, "assign_heavy"),
    (10**5, MAX_Q // 2, "lazy_compose"),
    (10**5, MAX_Q, "div_all_point_assign"),
    (1000, MAX_Q, "random"),
]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)
KINDS = ["random", "div_all_point_assign", "sum_only_max", "stripes", "small_x", "big_x", "assign_heavy",
         "lazy_compose"]


def interval(rng: random.Random, n: int, how: str = "random") -> tuple[int, int]:
    if how == "all":
        return 1, n
    if how == "point":
        i = rng.randint(1, n)
        return i, i
    if how == "short":
        l = rng.randint(1, n)
        return l, min(n, l + rng.randint(0, 10))
    if how == "wide":
        return rng.randint(1, max(1, n // 10)), rng.randint(n - n // 10, n)
    l, r = sorted(rng.randint(1, n) for _ in range(2))
    return l, r


def pick_interval(rng: random.Random, n: int) -> tuple[int, int]:
    return interval(rng, n, rng.choice(["random", "random", "random", "all", "point", "short", "wide"]))


def small_x(rng: random.Random) -> int:
    return rng.choice([2, 2, 2, 3, rng.randint(2, 10)])


def queries(rng: random.Random, n: int, q: int, kind: str) -> tuple[list[int], list[str]]:
    a = [rng.randint(1, MAX_V) for _ in range(n)]
    out: list[str] = []

    def div(l: int, r: int, x: int) -> None:
        out.append(f"1 {l} {r} {x}")

    def assign(l: int, r: int, y: int) -> None:
        out.append(f"2 {l} {r} {y}")

    def total(l: int, r: int) -> None:
        out.append(f"3 {l} {r}")

    if kind == "sum_only_max":
        a = [MAX_V] * n
        for _ in range(q):
            total(*interval(rng, n, rng.choice(["all", "wide", "random"])))
        return a, out
    if kind == "lazy_compose":
        # 全体を y にしてから割り算を重ね、そのあと 1 点や短い区間を読んで遅延を押し下げさせる。
        while len(out) < q:
            assign(1, n, rng.choice([MAX_V, MAX_V - 1, rng.randint(1, MAX_V)]))
            for _ in range(rng.randint(1, 6)):
                div(*interval(rng, n, rng.choice(["all", "wide"])), rng.choice([2, 3, 7, rng.randint(2, MAX_V)]))
            for _ in range(rng.randint(1, 6)):
                total(*interval(rng, n, rng.choice(["point", "short", "random"])))
        return a, out[:q]
    for _ in range(q):
        t = rng.random()
        if kind == "div_all_point_assign":
            if t < 0.35:
                div(1, n, 2)
            elif t < 0.7:
                assign(*interval(rng, n, "point"), MAX_V)
            else:
                total(*pick_interval(rng, n))
        elif kind == "stripes":
            if t < 0.5:
                assign(*interval(rng, n, "short"), rng.randint(1, MAX_V))
            elif t < 0.75:
                div(*interval(rng, n, rng.choice(["all", "wide", "random"])), small_x(rng))
            else:
                total(*pick_interval(rng, n))
        elif kind == "small_x":
            if t < 0.45:
                div(*pick_interval(rng, n), small_x(rng))
            elif t < 0.55:
                assign(*pick_interval(rng, n), rng.randint(1, MAX_V))
            else:
                total(*pick_interval(rng, n))
        elif kind == "big_x":
            if t < 0.4:
                div(*pick_interval(rng, n), rng.randint(10**4, MAX_V))
            elif t < 0.6:
                assign(*pick_interval(rng, n), rng.randint(1, MAX_V))
            else:
                total(*pick_interval(rng, n))
        elif kind == "assign_heavy":
            if t < 0.6:
                assign(*pick_interval(rng, n), rng.randint(1, MAX_V))
            elif t < 0.7:
                div(*pick_interval(rng, n), rng.randint(2, MAX_V))
            else:
                total(*pick_interval(rng, n))
        else:
            assert kind == "random"
            if t < 1 / 3:
                div(*pick_interval(rng, n), rng.choice([small_x(rng), rng.randint(2, MAX_V)]))
            elif t < 2 / 3:
                assign(*pick_interval(rng, n), rng.randint(1, MAX_V))
            else:
                total(*pick_interval(rng, n))
    return a, out


def case_for(seed: int, rng: random.Random) -> tuple[list[int], list[str]]:
    if seed >= 1000:
        top = 2000 if seed >= 2000 else 20
        n, q = rng.randint(1, top), rng.randint(1, top + 10)
        a, qs = queries(rng, n, q, rng.choice(KINDS))
        if rng.random() < 0.3:
            a = [rng.randint(1, 10) for _ in range(n)]
        if not any(line.startswith("3") for line in qs):
            qs[-1] = "3 1 {}".format(n)
        return a, qs
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    n, q, kind = PLANS[seed - len(FIXED)]
    return queries(rng, n, q, kind)


def main() -> None:
    seed = int(sys.argv[1])
    a, qs = case_for(seed, random.Random(seed))
    n = len(a)
    assert 1 <= n <= MAX_N and 1 <= len(qs) <= MAX_Q and all(1 <= v <= MAX_V for v in a)
    sums = 0
    for line in qs:
        f = list(map(int, line.split()))
        assert f[0] in (1, 2, 3) and len(f) == (3 if f[0] == 3 else 4) and 1 <= f[1] <= f[2] <= n
        assert f[0] != 1 or 2 <= f[3] <= MAX_V
        assert f[0] != 2 or 1 <= f[3] <= MAX_V
        sums += f[0] == 3
    assert sums >= 1
    sys.stdout.write(f"{n} {len(qs)}\n{' '.join(map(str, a))}\n" + "\n".join(qs) + "\n")


if __name__ == "__main__":
    main()
