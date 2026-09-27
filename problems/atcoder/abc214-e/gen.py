"""abc214-e (Packing Under Range Regulations) の入力を作る。T と、T 個のテストケース (N と N 個の L R) を出す。

seed が 0 から count - 1 までは本番のケース。例の 1 つ、角のケース、N の合計が 2 × 10^5 のもの。
Yes になる入力は、玉ごとに違う箱を先に決めてから、その箱を含む区間を付けて作る (planted)。No になる
入力は、そこから幅 w の窓を 1 つ選び、窓に収まる区間を w + 1 個にする (ほかの区間を窓の中へ置き直す)。
ほかに、全部同じ区間 (ちょうど入るものと 1 個あふれるもの)、10^9 に接する区間、入れ子で余りの無い区間
(一番外の 2 個を縮めると No)、箱に余りの無いもの、T = 2 × 10^5 で N = 1 のもの、を入れる。
RangeSet に離れた点をたくさん入れてから、それを端から順につなげていく形も入れる。
seed が 1000 以上なら、Hall の条件を区間ごとに確かめる愚直解で解ける小さい入力を出す
(pj testdata crosscheck 用)。2000 以上なら、愚直解でまだ解ける N ≤ 200 の入力を出す。
"""

import random
import sys

MAX_T = 2 * 10**5
MAX_SUM = 2 * 10**5
MAX_V = 10**9
HALF = MAX_SUM // 2

Test = list[tuple[int, int]]

SAMPLE = [
    [(1, 2), (2, 3), (3, 3)],
    [(1, 2), (2, 3), (3, 3), (1, 3), (999999999, 1000000000)],
]
# 手で選んだ小さいテストケース。答えは順に Yes Yes No No No Yes Yes No No Yes Yes。
HANDPICKED = [
    [(1, 1)],
    [(MAX_V, MAX_V)],
    [(5, 5), (5, 5)],
    [(1, 2), (1, 2), (1, 2)],
    [(1, 3), (2, 2), (2, 2)],
    [(1, 1), (3, 3), (2, 3)],  # 2 は 1 を飛ばして入る
    [(2, 3), (1, 1), (3, 3)],
    [(1, 2), (2, 2), (1, 1)],
    [(MAX_V - 1, MAX_V), (MAX_V, MAX_V), (MAX_V - 1, MAX_V - 1)],  # 10^9 + 1 に入れたくなる
    [(MAX_V - 1, MAX_V), (MAX_V, MAX_V)],
    [(1, MAX_V)] * 3,
]
# 本番のケースのうち例のあとに並べるもの。
PLANS = [
    "one",
    "handpicked",
    "same",  # [1, 10^5] が 10^5 個 (Yes) と、[1, 10^5 - 1] が 10^5 個 (No)
    "same_top",  # 10^9 に接する同じ区間。ちょうど入るもの (Yes) と 1 個あふれるもの (No)
    "many_tests",  # T = 2 × 10^5、N = 1
    "pairs",  # T = 10^5、N = 2 で値は 1 から 3
    "nested",  # Yes
    "nested_shrunk",  # No
    "planted",
    "planted_violated",
    "planted_tight",  # 箱に余りが無い
    "multi",  # T = 2000 の planted、半分は No
    "join",  # 離れた点を先に入れ (R が小さい)、あとの [1, 10^9] が端からつなげていく
    "random_small_tests",
    "random",
]
COUNT = 1 + len(PLANS)


def planted(rng: random.Random, n: int, lo: int, hi: int, slack: int) -> Test:
    """[lo, hi] から違う箱を n 個選び、それぞれを含む区間を付ける。必ず Yes。"""
    boxes = rng.sample(range(lo, hi + 1), n)
    return [(max(1, p - rng.randint(0, slack)), min(MAX_V, p + rng.randint(0, slack))) for p in boxes]


def violate(rng: random.Random, test: Test, width: int) -> Test:
    """幅 width の窓を 1 つ選び、窓に収まる区間を width + 1 個にする。必ず No。"""
    test = test[:]
    lo = min(l for l, _ in test)
    hi = max(r for _, r in test)
    a = rng.randint(lo, max(lo, hi - width + 1))
    b = a + width - 1
    assert b <= MAX_V
    inside = [i for i, (l, r) in enumerate(test) if a <= l and r <= b]
    outside = [i for i, (l, r) in enumerate(test) if not (a <= l and r <= b)]
    need = width + 1 - len(inside)
    assert need <= len(outside)
    for i in rng.sample(outside, max(0, need)):
        l = rng.randint(a, b)
        test[i] = (l, rng.randint(l, b))
    return test


def nested(n: int) -> Test:
    """[i, n + 1 - i] を 2 個ずつ。内側の k 段は 2k 個で、幅もちょうど 2k。"""
    return [(i, n + 1 - i) for i in range(1, n // 2 + 1) for _ in range(2)]


def random_test(rng: random.Random, n: int, lo: int, hi: int) -> Test:
    return [tuple(sorted((rng.randint(lo, hi), rng.randint(lo, hi)))) for _ in range(n)]


def shuffled(rng: random.Random, test: Test) -> Test:
    test = test[:]
    rng.shuffle(test)
    return test


def plan_case(rng: random.Random, plan: str) -> list[Test]:
    if plan == "one":
        return [[(1, 1)]]
    if plan == "handpicked":
        return HANDPICKED
    if plan == "same":
        return [[(1, HALF)] * HALF, [(1, HALF - 1)] * HALF]
    if plan == "same_top":
        return [[(MAX_V - HALF + 1, MAX_V)] * HALF, [(MAX_V - HALF + 2, MAX_V)] * HALF]
    if plan == "many_tests":
        return [random_test(rng, 1, 1, 10**5) for _ in range(MAX_T)]
    if plan == "pairs":
        return [random_test(rng, 2, 1, 3) for _ in range(MAX_T // 2)]
    if plan == "nested":
        return [shuffled(rng, nested(MAX_SUM))]
    if plan == "nested_shrunk":
        return [shuffled(rng, [(2, MAX_SUM)] * 2 + nested(MAX_SUM)[2:])]
    if plan == "planted":
        return [planted(rng, MAX_SUM, 1, 2 * MAX_SUM, 5)]
    if plan == "planted_violated":
        return [violate(rng, planted(rng, MAX_SUM, 1, 2 * MAX_SUM, 5), 30)]
    if plan == "planted_tight":
        return [planted(rng, MAX_SUM, 1, MAX_SUM, 1000)]
    if plan == "multi":
        out = []
        for i in range(2000):
            n = 50
            base = rng.randint(0, MAX_V - 2 * n)
            test = planted(rng, n, base + 1, base + n + n // 2, rng.choice([0, 1, 3, n]))
            out.append(violate(rng, test, rng.randint(1, n // 2)) if i % 2 == 1 else test)
        return out
    if plan == "join":
        return [shuffled(rng, [(2 * i, 2 * i) for i in range(1, HALF + 1)] + [(1, MAX_V)] * HALF)]
    if plan == "random_small_tests":
        return [random_test(rng, 1000, 1, 1000) for _ in range(50)]
    assert plan == "random"
    return [random_test(rng, 50000, 1, 50000)]


def small_case(rng: random.Random, max_t: int, max_n: int) -> list[Test]:
    """愚直解で解ける大きさ。箱の場所は 1 の近く、10^9 の近く、その間のどこか。"""
    out = []
    for _ in range(rng.randint(1, max_t)):
        n = rng.randint(1, max_n)
        width = max(1, n + rng.randint(-1, 3))
        span = max(n, width)
        base = rng.choice([0, 0, rng.randint(0, MAX_V - span), MAX_V - span])
        kind = rng.randrange(3)
        if kind == 0:
            test = random_test(rng, n, base + 1, base + width)
        else:
            test = planted(rng, n, base + 1, base + span, rng.choice([0, 1, 2, n]))
            if kind == 2 and n >= 2:
                test = violate(rng, test, rng.randint(1, n - 1))
        out.append(test)
    return out


def case_for(seed: int, rng: random.Random) -> list[Test]:
    if seed >= 2000:
        return small_case(rng, 3, 200)
    if seed >= 1000:
        return small_case(rng, 5, 8)
    if seed == 0:
        return SAMPLE
    return plan_case(rng, PLANS[seed - 1])


def main() -> None:
    seed = int(sys.argv[1])
    tests = case_for(seed, random.Random(seed))
    assert 1 <= len(tests) <= MAX_T and sum(len(t) for t in tests) <= MAX_SUM
    assert all(len(t) >= 1 and all(1 <= l <= r <= MAX_V for l, r in t) for t in tests)
    out = [str(len(tests))]
    for t in tests:
        out.append(str(len(t)))
        out += [f"{l} {r}" for l, r in t]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
