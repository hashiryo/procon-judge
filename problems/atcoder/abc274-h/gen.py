"""abc274-h (XOR Sum of Arrays) の入力を作る。N Q、A_1 ... A_N、Q 個の質問 a b c d e f を出す。

seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース、N = 5 × 10^5 などの大きいケース。
提出はローリングハッシュ (ニム積) で S(A(a, b), A(c, d)) と A(e, f) の共通接頭辞の長さを二分探索するので、
共通接頭辞が長い質問を多めに入れる。A の作り方と、そのときの質問は次のとおり。
- 全部 0。S も A(e, f) も 0 だけなので、答えは長さだけで決まる。
- 周期 3 の (x, y, x xor y) を並べて、数か所だけ値を変えたもの (変えないものも入れる)。a, c, e を
  3 で割った余りが全部違う質問では、変えた所までずっと一致する。
- ほとんど 0 で、まばらに大きい値を置いたもの。e = a (か e = c) の質問では、A(c, d) (か A(a, b)) の
  最初の 0 でない所まで一致する。
- ランダム (10^18 まで、0 から 3、0 と 1)、10^18 と 2^59 の近くの値 (xor が 10^18 を超える)。
長さは、S が A(e, f) より短い、同じ、長いものを混ぜる (一致したときは長さで答えが決まる)。
10^18 までのランダムな N = 5 × 10^5 は入力が 10 MB ほどになるので、1 ケースにして質問を減らす。
seed が 1000 以上なら、質問ごとに先頭から 1 つずつ比べる愚直解で解ける小さい入力を出す
(pj testdata crosscheck 用)。2000 以上なら、N と Q が 2000 までの入力を出す。
"""

import random
import sys

MAX_N = 5 * 10**5
MAX_Q = 5 * 10**4
MAX_A = 10**18

SAMPLES = [
    (
        [1, 2, 3, 1],
        [(1, 3, 2, 4, 1, 4), (1, 2, 2, 3, 3, 4), (1, 1, 2, 2, 3, 4), (1, 2, 2, 3, 3, 3), (1, 4, 1, 4, 1, 1)],
    ),
    (
        [725560240, 9175925348, 9627229768, 7408031479, 623321125, 4845892509, 8712345300, 1026746010, 4844359340, 2169008582],
        [
            (5, 6, 5, 6, 2, 6), (5, 6, 1, 2, 1, 1), (3, 8, 3, 8, 1, 6), (5, 10, 1, 6, 1, 7), (3, 4, 1, 2, 5, 5),
            (7, 10, 4, 7, 2, 3), (3, 6, 1, 4, 7, 9), (4, 5, 3, 4, 8, 9), (2, 6, 1, 5, 5, 8), (4, 8, 1, 5, 1, 9),
        ],
    ),
]
# (N, Q, A の作り方, 質問の出し方)。本番のケースのうち例と角のケースのあとに並べる。
# Q = 0 は、全部の質問を 1 回ずつ聞くという意味。
PLANS = [
    (1, 0, "zero", "random"),
    (1, 0, "random", "random"),
    (2, 0, "zero", "random"),
    (3, 0, "small", "random"),
    (4, 0, "period", "random"),
    (6, 0, "period_clean", "random"),
    (7, 0, "small", "random"),
    (10, 0, "bits", "random"),  # 21175 個
    (MAX_N, 10**4, "random", "random"),
    (MAX_N, 3 * 10**4, "zero", "random"),
    (MAX_N, 3 * 10**4, "bits", "random"),
    (MAX_N, 3 * 10**4, "period", "aligned"),
    (MAX_N, MAX_Q, "sparse", "near"),
    (5 * 10**4, 3 * 10**4, "extreme", "random"),
    (5 * 10**4, 2 * 10**4, "period_big", "aligned"),
    (1000, MAX_Q, "small", "random"),
    (1000, MAX_Q, "period", "aligned"),
    (1000, MAX_Q, "sparse", "near"),
    (2 * 10**5, 3 * 10**4, "sparse", "near"),
]
KINDS = ["zero", "random", "small", "bits", "period", "period_big", "period_clean", "sparse", "extreme"]
EXTREME = [0, 1, MAX_A, MAX_A - 1, 2**59, 2**59 - 1, 2**59 + 1, 2**58, MAX_A ^ (2**59 - 1)]


def array(rng: random.Random, n: int, how: str) -> list[int]:
    if how == "zero":
        return [0] * n
    if how == "random":
        return [rng.randint(0, MAX_A) for _ in range(n)]
    if how == "small":
        return [rng.randint(0, 3) for _ in range(n)]
    if how == "bits":
        return [rng.randint(0, 1) for _ in range(n)]
    if how == "extreme":
        return [rng.choice(EXTREME) for _ in range(n)]
    if how in ("period", "period_big", "period_clean"):
        top = MAX_A if how == "period_big" else 1000
        x, y = rng.randint(0, top), rng.randint(0, top)
        while x ^ y > MAX_A:
            y = rng.randint(0, top)
        out = [(x, y, x ^ y)[i % 3] for i in range(n)]
        if how != "period_clean":
            for i in rng.sample(range(n), min(n // 3, rng.randint(1, 60))):
                out[i] = rng.randint(0, top)
        return out
    assert how == "sparse"
    out = [0] * n
    for i in rng.sample(range(n), max(1, min(n, n // rng.choice([100, 1000, 10000])))):
        out[i] = rng.randint(1, MAX_A)
    return out


def length(rng: random.Random, n: int) -> int:
    return rng.choice([1, n, rng.randint(1, n), rng.randint(1, min(n, 10)), rng.randint(max(1, n - 10), n)])


def target(rng: random.Random, n: int, m: int, residue: int | None = None) -> tuple[int, int]:
    """A(e, f) を選ぶ。長さは S の長さ m の近くを多めにする。residue があれば e - 1 をその余りにする。"""
    k = min(n, max(1, rng.choice([m, m, m - 1, m + 1, rng.randint(1, n)])))
    e = start(rng, n, k, residue)
    if e is None:
        k = m
        e = start(rng, n, k, residue)
    return e, e + k - 1


def start(rng: random.Random, n: int, m: int, residue: int | None) -> int | None:
    """長さ m の区間の始まり (1 始まり) を選ぶ。residue があれば始まり - 1 を 3 で割った余りをそれにする。"""
    if residue is None:
        return rng.randint(1, n - m + 1)
    if n - m < residue:
        return None
    return 3 * rng.randint(0, (n - m - residue) // 3) + residue + 1


def query(rng: random.Random, a_list: list[int], how: str) -> tuple[int, int, int, int, int, int]:
    n = len(a_list)
    m = length(rng, n)
    if how == "aligned" and n >= 3:
        # a, c, e の余りを全部違うものにすると、周期 3 の (x, y, x xor y) ではずっと一致する。
        r = [0, 1, 2]
        rng.shuffle(r)
        m = min(m, n - 2)
        a, c = start(rng, n, m, r[0]), start(rng, n, m, r[1])
        if a is not None and c is not None:
            e, f = target(rng, n, m, r[2])
            return a, a + m - 1, c, c + m - 1, e, f
    a, c = start(rng, n, m, None), start(rng, n, m, None)
    if how == "near" and rng.random() < 0.8:
        # e = a か e = c にすると、もう一方の最初の 0 でない所まで一致する。
        e = a if rng.random() < 0.5 else c
        k = min(n - e + 1, max(1, rng.choice([m, m, m - 1, m + 1])))
        return a, a + m - 1, c, c + m - 1, e, e + k - 1
    e, f = target(rng, n, m)
    return a, a + m - 1, c, c + m - 1, e, f


def all_queries(n: int) -> list[tuple[int, int, int, int, int, int]]:
    out = []
    for m in range(1, n + 1):
        for a in range(1, n - m + 2):
            for c in range(1, n - m + 2):
                for e in range(1, n + 1):
                    for f in range(e, n + 1):
                        out.append((a, a + m - 1, c, c + m - 1, e, f))
    return out


def case_for(seed: int, rng: random.Random) -> tuple[list[int], list[tuple[int, int, int, int, int, int]]]:
    if seed >= 1000:
        limit = 2000 if seed >= 2000 else 30
        n = rng.randint(1, limit)
        a_list = array(rng, n, rng.choice(KINDS))
        how = rng.choice(["random", "aligned", "near"])
        return a_list, [query(rng, a_list, how) for _ in range(rng.randint(1, limit))]
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    n, q, kind, how = PLANS[seed - len(SAMPLES)]
    a_list = array(rng, n, kind)
    return a_list, all_queries(n) if q == 0 else [query(rng, a_list, how) for _ in range(q)]


COUNT = len(SAMPLES) + len(PLANS)


def main() -> None:
    seed = int(sys.argv[1])
    a_list, qs = case_for(seed, random.Random(seed))
    n = len(a_list)
    assert 1 <= n <= MAX_N and 1 <= len(qs) <= MAX_Q and all(0 <= x <= MAX_A for x in a_list)
    for a, b, c, d, e, f in qs:
        assert 1 <= a <= b <= n and 1 <= c <= d <= n and 1 <= e <= f <= n and b - a == d - c
    out = [f"{n} {len(qs)}", " ".join(map(str, a_list))] + [" ".join(map(str, x)) for x in qs]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
