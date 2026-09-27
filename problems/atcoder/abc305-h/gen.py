"""abc305-h (Shojin) の入力を作る。N X と、N 行の A_i B_i を出す。

seed が 0 から count - 1 までは本番のケース。例の 5 つ、角のケース、N = 2 × 10^5 のいろいろな列、
愚直解でも解ける N = 300 までのランダム。提出は A = 1 の問題を先に取り除き、残りを 1 日に解くときの
疲労を区間ごとに求めてから、1 日ごとに p を足した最小 f(p) を使って (f(p) - X) / p を
long double のフィボナッチ探索で最大にする。
列は、全部 A = 1 (取り除くと空になる分岐)、全部 (2, 1) (1 日に 26 問まで解けて区間が最も長い)、
全部 (2, 1) で X = ΣB (毎日 1 問)、X = ΣB + 1000 (1000 組だけ 2 問の日にできる)、大きい A のランダム
(D がほぼ N)、小さい A と B のランダム、A = 1 と 2 が混ざったもの、ほとんど A = 1 で D = 1 になりうるもの、
B / (A - 1) が全部同じもの (並べ替えの比べ方が同点になる)。同じ (A, B) が並ぶ列では、k 日に分けたときの
最小の疲労が釣り合った分け方の閉じた式になるので、X をちょうどそれにしたもの (M = X) と、1 小さくしたもの
(D が 1 増える) も入れる。
one_cheap は、(10^5, b) が並ぶ中に A が 10^5 より小さい問題 (a, b) が 1 つだけある列。2 問を 1 日に
まとめたときの疲労の増分は、(a, b) を含む組の p* = b (a - 1) が最も小さい。X を ΣB + p* - 1 にすると
1 日も減らせないので、D = N、M = ΣB になる。このとき (f(p) - X) / p は p* の右でほとんど平らになり、
浮動小数点で比べる探索は p* の右で止まることがある。そこでは N 日が最適でないので M が小さく出る
(2026-09-27 に、long double が double になる arm64 の macOS では N = 3 でも、80 ビットの x86-64 でも
N = 3000 から、M が正しくならなかった)。
seed が 1000 以上なら、区間の中の並べ方を部分集合の DP で求め、日数ごとの最小を DP で求める愚直解で
解ける N = 10 までの入力を出す (pj testdata crosscheck 用)。2000 以上なら、A を 4 以上にして区間を
13 問までに抑え、愚直解でまだ解ける N = 300 までの入力を出す。
"""

import random
import sys

MAX_N = 2 * 10**5
MAX_X = 10**8
MAX_A = 10**5

Case = tuple[int, list[tuple[int, int]]]

SAMPLES: list[Case] = [
    (100, [(2, 2), (3, 4), (5, 7)]),
    (30, [(2, 2), (3, 4), (5, 7)]),
    (50000000, [(100000, 10000000)] * 5),
    (100000000, [(5, 88), (66, 4), (52, 1), (3, 1), (12, 1), (53, 25), (11, 12), (12, 2), (1, 20), (47, 10)]),
    (
        100000000,
        [
            (2387, 3178), (2369, 5772), (1, 29), (36, 3), (52, 2981), (196, 1), (36, 704), (3, 3), (1501, 5185),
            (23, 628), (3623, 810), (80, 101), (6579, 15), (681, 7), (183, 125),
        ],
    ),
]
FIXED: list[Case] = [
    (1, [(1, 1)]),
    (1, [(2, 1)]),
    (MAX_X, [(MAX_A, MAX_X)]),
    (2, [(2, 1), (2, 1)]),  # 毎日 1 問
    (3, [(2, 1), (2, 1)]),  # 1 日で 3
    (6, [(1, 1), (1, 2), (1, 3)]),  # 全部 A = 1
    (8, [(2, 1), (1, 5), (2, 1)]),  # A = 1 を取り除いた残りを 1 日で解く
    (7, [(2, 1), (1, 5), (2, 1)]),
    (MAX_X, [(MAX_A, 1)] + [(1, 1)] * 9 + [(MAX_A, 1)]),
    # D = 3、M = ΣB = 867629。2 日にする最小の疲労は X + 1。
    (90375697, [(38400, 18490), (99999, 2331), (100000, 846808)]),
]
# (N, 列の作り方)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    (MAX_N, "all_one"),
    (MAX_N, "twos"),
    (MAX_N, "twos_tight"),
    (MAX_N, "twos_slack"),
    (MAX_N, "random_big"),
    (MAX_N, "random_small"),
    (MAX_N, "mixed_ones"),
    (MAX_N, "sparse"),
    (MAX_N, "equal_ratio"),
    (MAX_N, "identical_exact"),
    (MAX_N, "identical_minus"),
    (MAX_N, "one_cheap"),
    (20000, "one_cheap"),
    (3000, "one_cheap"),
    (300, "medium"),
    (300, "medium"),
]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)


def day_cost(a: int, b: int, length: int) -> int:
    """(a, b) を length 問解く 1 日の疲労。b (a^length - 1) / (a - 1)。"""
    return b * length if a == 1 else b * (a**length - 1) // (a - 1)


def identical_min(n: int, a: int, b: int, k: int) -> int:
    """同じ (a, b) の n 問を k 日に分けたときの最小の疲労。1 日の疲労は問題数について凸なので、釣り合った分け方が最小。"""
    q, r = divmod(n, k)
    return r * day_cost(a, b, q + 1) + (k - r) * day_cost(a, b, q)


def one_cheap(n: int, a: int, b: int, pos: int) -> Case:
    """(10^5, b) の n 問のうち pos 番目だけ (a, b) にした列と、1 日減らすとちょうど 1 だけ超える X。
    最も安いまとめ方は (a, b) を含む組で、増分は b (a - 1)。答えは D = n、M = n b。
    """
    rows = [(MAX_A, b)] * n
    rows[pos] = (a, b)
    return n * b + b * (a - 1) - 1, rows


def with_budget(rng: random.Random, rows: list[tuple[int, int]]) -> Case:
    """X を ΣB から 10^8 までのどこかにする。余裕の大きさは桁ごとに選び、小さい余裕を多めにする。"""
    total = sum(b for _, b in rows)
    slack = rng.choice([0, 1, 2, rng.randint(1, 10), rng.randint(1, 100), rng.randint(1, 1000), rng.randint(1, 10**5), MAX_X])
    return min(MAX_X, total + slack), rows


def plan_case(rng: random.Random, n: int, how: str) -> Case:
    if how == "all_one":
        rows = [(1, rng.randint(1, 500)) for _ in range(n)]
        return rng.randint(sum(b for _, b in rows), MAX_X), rows
    if how == "twos":
        return MAX_X, [(2, 1)] * n
    if how == "twos_tight":
        return n, [(2, 1)] * n
    if how == "twos_slack":
        return n + 1000, [(2, 1)] * n
    if how == "random_big":
        rows = [(rng.randint(2, MAX_A), rng.randint(1, MAX_X // n)) for _ in range(n)]
        return MAX_X, rows
    if how == "random_small":
        return MAX_X, [(rng.randint(2, 3), rng.randint(1, 3)) for _ in range(n)]
    if how == "mixed_ones":
        return MAX_X, [(rng.choice([1, 2]), rng.randint(1, 3)) for _ in range(n)]
    if how == "sparse":
        # ほとんど A = 1。A の大きい問題が数個だけ混ざる。
        rows = [(1, rng.randint(1, 100)) for _ in range(n)]
        for i in rng.sample(range(n), 8):
            rows[i] = (rng.randint(2, 30), rng.randint(1, 100))
        return MAX_X, rows
    if how == "equal_ratio":
        # B / (A - 1) = 3 がどれも同じ。
        rows = []
        for _ in range(n):
            a = rng.choice([2, 3, 5, 11])
            rows.append((a, 3 * (a - 1)))
        return MAX_X, rows
    if how in ("identical_exact", "identical_minus"):
        a, b, k = 2, 1, 20000  # 1 日 10 問で、1 日の疲労は 1023
        x = identical_min(n, a, b, k)
        return (x if how == "identical_exact" else x - 1), [(a, b)] * n
    if how == "one_cheap":
        a = 99990
        return one_cheap(n, a, (MAX_X + 1) // (n + a - 1), n // 2)
    assert how == "medium"
    return medium_case(rng, n)


def small_case(rng: random.Random) -> Case:
    """愚直解が区間の中の並べ方を部分集合の DP で求められる大きさ。A = 1 や同じ比も混ぜる。"""
    n = rng.randint(1, 10)
    style = rng.choice(["tiny", "wide", "ones", "ratio", "big", "one_cheap"])
    if style == "one_cheap" and n >= 2:
        a = rng.randint(2, MAX_A - 1)
        b = rng.randint(1, (MAX_X + 1) // (n + a - 1))
        return one_cheap(n, a, b, rng.randrange(n))
    rows = []
    for _ in range(n):
        if style == "tiny":
            rows.append((rng.randint(1, 3), rng.randint(1, 3)))
        elif style == "wide":
            rows.append((rng.randint(1, MAX_A), rng.randint(1, 10**4)))
        elif style == "ones":
            rows.append((rng.choice([1, 1, 2, 7]), rng.randint(1, 20)))
        elif style == "ratio":
            a = rng.choice([2, 3, 4, 5])
            rows.append((a, rng.choice([1, 2]) * (a - 1)))
        else:
            rows.append((rng.choice([2, MAX_A]), rng.choice([1, MAX_X // 20])))
    return with_budget(rng, rows)


def medium_case(rng: random.Random, n: int) -> Case:
    """A を 4 以上にして 1 日を 13 問までに抑えた入力。"""
    style = rng.choice(["small", "wide", "ratio"])
    rows = []
    for _ in range(n):
        if style == "small":
            rows.append((rng.randint(4, 6), rng.randint(1, 5)))
        elif style == "wide":
            rows.append((rng.randint(4, MAX_A), rng.randint(1, 1000)))
        else:
            a = rng.choice([4, 5, 9])
            rows.append((a, 2 * (a - 1)))
    return with_budget(rng, rows)


def case_for(seed: int, rng: random.Random) -> Case:
    if seed >= 2000:
        return medium_case(rng, rng.randint(50, 300))
    if seed >= 1000:
        return small_case(rng)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    return plan_case(rng, *PLANS[seed - len(FIXED)])


def main() -> None:
    seed = int(sys.argv[1])
    x, rows = case_for(seed, random.Random(seed))
    n = len(rows)
    assert 1 <= n <= MAX_N and 1 <= x <= MAX_X
    assert all(1 <= a <= MAX_A and 1 <= b for a, b in rows) and sum(b for _, b in rows) <= x
    out = [f"{n} {x}"] + [f"{a} {b}" for a, b in rows]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
