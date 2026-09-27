"""abc292-h (Rating Estimator) の入力を作る。N B Q、見積もり a、Q 個の変更 c x を出す。

seed が 0 から count - 1 までは本番のケース。例の 1 つ、角のケース、N = 5 * 10^5 あたりのいろいろな形。
角のケースは、N = 1、1 回目でレートが B に届くもの、ちょうど B になるもの、最後まで届かないもの。
形は、ランダム、終わりの近くでやっと届くもの (max_right が右まで降りる)、値が小さいもの、先頭を
0 と 10^9 に交互に変えるもの (届く位置が先頭と末尾を行き来する)、全部 0 で B = 1、増加列、
届く位置がばらばらになるもの。
出力は小数点以下 20 桁なので、Q = 10^5 のケースは 1 つにする (データが大きくなりすぎるため)。

B が大きく、最後まで B に届かず、レートが B よりずっと小さいケースは入れない。lib.cpp は
(a - B の和) / n + B を double で計算するので、B の桁で丸めた誤差がそのまま残り、たとえば
N = 3, B = 10^9, a = (0, 0, 1) では 1/3 が 0.33333337... になって、許される誤差 10^-9 を超える。
参照実装はこの lib.cpp を固定したものなので、そういうケースでは期待出力のほうが正しくない。
小さい入力では lib.cpp と同じ計算をして誤差を確かめ、10^-10 を超える入力は作り直す。
大きい入力は形で避ける (B <= 2^20 にするか、どの時点でも a の平均が B に比べて十分大きい)。
seed が 1000 以上なら、質問ごとにコンテストを順にたどる愚直解で解ける小さい入力を出す
(pj testdata crosscheck 用)。2000 以上なら、N, Q <= 500 の入力を出す。
"""

import random
import sys
from fractions import Fraction

MAX_N = 5 * 10**5
MAX_Q = 10**5
MAX_V = 10**9

SAMPLES = [
    (6, [5, 1, 9, 3, 8], [(4, 9), (2, 10), (1, 0), (3, 0), (3, 30), (5, 100), (1, 100)]),
]
# 小さい角のケース (B, a, 変更の並び)。
FIXED = [
    (1, [0], [(1, 0)]),
    (MAX_V, [MAX_V], [(1, MAX_V)]),
    (MAX_V, [0], [(1, MAX_V - 1), (1, MAX_V), (1, 0)]),
    (5, [4, 6, 100], [(3, 100), (1, 5), (1, 3), (2, 7), (2, 8)]),  # 2 回目でちょうど B
    (MAX_V, [MAX_V - 1] * 5, [(3, MAX_V - 1), (5, MAX_V), (5, MAX_V - 1), (1, MAX_V)]),
    (1, [MAX_V] * 4, [(1, 0), (2, 0), (3, 0), (4, 0), (1, 1)]),
]
# (N, Q, 形)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    (MAX_N, MAX_Q, "random"),
    (MAX_N, 30000, "late"),
    (MAX_N, 50000, "small"),
    (MAX_N, 20000, "toggle_first"),
    (MAX_N, 30000, "zeros"),
    (MAX_N, 30000, "spread"),
    (200000, 20000, "increasing"),
    (1, 30000, "small"),
    (65536, 20000, "random"),
    (65537, 20000, "late"),
    (1000, 1000, "random"),
]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)


def unsafe(b: int, a: list[int], changes: list[tuple[int, int]]) -> bool:
    """lib.cpp の答えの誤差が 10^-10 (絶対か相対) を超える質問があるか。lib.cpp と同じ順に double で計算し
    (Python の float も double で、四則演算の丸めは同じ)、分数で求めた答えと比べる。
    """
    a = a[:]
    for c, x in changes:
        a[c - 1] = x
        total, n = 0, 0
        for v in a:
            total, n = total + v, n + 1
            if total >= b * n:
                break
        exact = Fraction(total, n)
        approx = float(total - b * n) / n + b
        if abs(Fraction(approx) - exact) > Fraction(1, 10**10) * max(1, exact):
            return True
    return False


def make(rng: random.Random, n: int, q: int, shape: str) -> tuple[int, list[int], list[tuple[int, int]]]:
    if shape == "random":
        b = rng.randint(1, MAX_V)
        a = [rng.randint(0, MAX_V) for _ in range(n)]
        changes = [(rng.randint(1, n), rng.randint(0, MAX_V)) for _ in range(q)]
    elif shape == "late":
        # 最後の方まで B - 1 が続き、終わりの近くを大きくすると届く。
        b = 1000
        a = [b - 1] * n
        changes = [(rng.randint(max(1, n - 1000), n), rng.choice([0, b - 1, b, MAX_V])) for _ in range(q)]
    elif shape == "small":
        b = rng.randint(1, 100)
        a = [rng.randint(0, 100) for _ in range(n)]
        changes = [(rng.randint(1, n), rng.randint(0, 100)) for _ in range(q)]
    elif shape == "toggle_first":
        b = 1000
        a = [rng.randint(0, b - 1) for _ in range(n)]
        changes = [(1, MAX_V if i % 2 == 0 else 0) for i in range(q)]
    elif shape == "zeros":
        # 全部 0 で B = 1。位置 c を c - 1, c, c + 1 にすると、c 回目で届く境目 (c なら平均がちょうど B)。
        b = 1
        a = [0] * n
        changes = []
        for _ in range(q):
            c = rng.randint(1, n)
            changes.append((c, rng.choice([c - 1, c, c + 1, 0])))
    elif shape == "spread":
        # B - 1 が並び、最後だけ 10^9。ランダムな位置を 10^9 にしては戻すので、届く位置が全体に散らばる。
        b = 1000
        a = [b - 1] * (n - 1) + [MAX_V]
        changes = []
        while len(changes) < q:
            c = rng.randint(1, n - 1)
            changes += [(c, MAX_V), (c, b - 1)]
        changes = changes[:q]
    else:
        assert shape == "increasing"
        # 前から増えていくので、平均が B に届くのは後ろの方。
        b = 49 * 10**7
        a = sorted(rng.randint(0, MAX_V) for _ in range(n))
        changes = [(rng.randint(1, n), rng.randint(0, MAX_V)) for _ in range(q)]
    return b, a, changes


def small_case(rng: random.Random, limit: int) -> tuple[int, list[int], list[tuple[int, int]]]:
    """値の範囲をいろいろに変えた小さい入力。lib.cpp の誤差が大きくなる入力は作り直す。"""
    while True:
        n, q = rng.randint(1, limit), rng.randint(1, limit)
        top = rng.choice([1, 10, 1000, MAX_V])
        b = rng.randint(1, top)

        def value() -> int:
            return rng.choice([0, top, rng.randint(0, top), rng.randint(0, top), b, max(0, b - 1)])

        a = [value() for _ in range(n)]
        changes = [(rng.randint(1, n), value()) for _ in range(q)]
        if not unsafe(b, a, changes):
            return b, a, changes


def case_for(seed: int, rng: random.Random) -> tuple[int, list[int], list[tuple[int, int]]]:
    if seed >= 1000:
        return small_case(rng, 500 if seed >= 2000 else 12)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    n, q, shape = PLANS[seed - len(FIXED)]
    return make(rng, n, q, shape)


def main() -> None:
    seed = int(sys.argv[1])
    b, a, changes = case_for(seed, random.Random(seed))
    n = len(a)
    assert 1 <= n <= MAX_N and 1 <= b <= MAX_V and 1 <= len(changes) <= MAX_Q
    assert all(0 <= v <= MAX_V for v in a) and all(1 <= c <= n and 0 <= x <= MAX_V for c, x in changes)
    if n * len(changes) <= 10**6:
        assert not unsafe(b, a, changes)
    out = [f"{n} {b} {len(changes)}", " ".join(map(str, a))] + [f"{c} {x}" for c, x in changes]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
