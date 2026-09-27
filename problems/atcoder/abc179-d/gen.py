"""abc179-d (Leaping Tak) の入力を作る。N K と、交わらない K 個の区間 [L_i, R_i] を出す。

seed が 0 から count - 1 までは本番のケース。例の 4 つ、角のケース、N = 2 × 10^5 のいろいろな区間、
N が 5000 までのランダム。
提出は 1 / (1 - Σ x^d) を疎な分母で割って求め、分母の項は区間の端 L と R + 1 に立つ。区間が
隣り合うと R_i + 1 = L_j の項が打ち消し合って消えるので、隣り合う区間も入れる。
答えが閉じた形になるものも入れる。S = [1, N] なら 2^(N-2)、S = {1} なら 1、
S = {N} や、N - 1 が奇数で S が偶数だけなら 0。
seed が 1000 以上なら、マス i に来る方法の数を S の要素ごとに足す愚直解で解ける小さい入力を出す
(pj testdata crosscheck 用)。2000 以上なら、愚直解でまだ解ける N = 5000 までの入力を出す。
"""

import random
import sys

MAX_N = 2 * 10**5
MAX_K = 10

SAMPLES = [
    (5, [(1, 1), (3, 4)]),
    (5, [(3, 3), (5, 5)]),
    (5, [(1, 2)]),
    (60, [(5, 8), (1, 3), (10, 15)]),
]
FIXED = [
    (2, [(1, 1)]),
    (2, [(2, 2)]),  # 1 から 2 は外に出る
    (2, [(2, 2), (1, 1)]),
    (10, [(i, i) for i in range(1, 11)]),  # K = N = 10
    (MAX_N, [(1, 1)]),  # 答えは 1
    (MAX_N, [(1, MAX_N)]),  # 答えは 2^(N-2)
    (MAX_N, [(MAX_N, MAX_N)]),  # 答えは 0
    (MAX_N, [(MAX_N - 1, MAX_N - 1)]),  # 1 から N へ 1 回で跳ぶだけ
    (MAX_N, [(2 * i, 2 * i) for i in range(1, 11)]),  # 偶数だけ。N - 1 が奇数なので 0
    # [1, N] を隣り合う 10 個の区間に分けたもの。答えは 2^(N-2)。
    (MAX_N, [(1 + i * MAX_N // 10, (i + 1) * MAX_N // 10) for i in range(10)]),
]
# 区間の選び方。random_k は K もランダム、medium は N が 1000 から 5000 で愚直解でも解ける。
# ほかは N = 2 × 10^5、K = 10。
PLANS = ["random", "adjacent", "short", "far", "random_k", "medium", "medium"]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)


def segments(rng: random.Random, n: int, k: int, how: str) -> list[tuple[int, int]]:
    """[1, n] の中の交わらない k 個の区間を、順番を混ぜて返す。"""
    if how == "adjacent":
        # 隣り合う区間の列。端が打ち消し合う。
        cuts = sorted(rng.sample(range(2, n // 100), k))
        segs = [(cuts[i], cuts[i + 1] - 1) for i in range(k - 1)] + [(cuts[-1], cuts[-1] + rng.randint(0, 5))]
    elif how == "short":
        # 小さい d ばかり。長さ 1 から 3 の区間を 20 までに詰める。
        segs, lo = [], 1
        for _ in range(k):
            lo += rng.randint(0, 1)
            hi = lo + rng.randint(0, 1)
            segs.append((lo, hi))
            lo = hi + 1
    elif how == "far":
        # 1 と、N に近い長い区間。
        segs = [(1, 1)]
        points = sorted(rng.sample(range(n // 2, n + 1), 2 * (k - 1)))
        segs += [(points[2 * i], points[2 * i + 1]) for i in range(k - 1)]
    else:
        # 2k 個の異なる点を取り、隣どうしを組にする。組の間は 1 以上空くとは限らないので、隣り合う区間も出る。
        points = sorted(rng.sample(range(1, n + 1), 2 * k))
        segs = [(points[2 * i], points[2 * i + 1]) for i in range(k)]
        if rng.random() < 0.5:
            segs = [(l, l + rng.randint(0, 3)) if l + 3 < r else (l, r) for l, r in segs]
    rng.shuffle(segs)
    return segs


def small_case(rng: random.Random, n: int) -> tuple[int, list[tuple[int, int]]]:
    k = rng.randint(1, min(n, MAX_K))
    if 2 * k > n:
        return n, [(i, i) for i in rng.sample(range(1, n + 1), k)]
    hows = ["random"] + (["short"] if n >= 40 else []) + (["far"] if n >= 300 and k >= 2 else [])
    return n, segments(rng, n, k, rng.choice(hows))


def case_for(seed: int, rng: random.Random) -> tuple[int, list[tuple[int, int]]]:
    if seed >= 1000:
        return small_case(rng, rng.randint(1000, 5000) if seed >= 2000 else rng.randint(2, 60))
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    how = PLANS[seed - len(FIXED)]
    if how == "medium":
        return small_case(rng, rng.randint(1000, 5000))
    if how == "random_k":
        k = rng.randint(1, MAX_K - 1)
        return MAX_N, segments(rng, MAX_N, k, "random")
    return MAX_N, segments(rng, MAX_N, MAX_K, how)


def main() -> None:
    seed = int(sys.argv[1])
    n, segs = case_for(seed, random.Random(seed))
    k = len(segs)
    assert 2 <= n <= MAX_N and 1 <= k <= min(n, MAX_K)
    assert all(1 <= l <= r <= n for l, r in segs)
    ordered = sorted(segs)
    assert all(ordered[i][1] < ordered[i + 1][0] for i in range(k - 1))
    out = [f"{n} {k}"] + [f"{l} {r}" for l, r in segs]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
