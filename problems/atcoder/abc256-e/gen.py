"""abc256-e (Takahashi's Anguish) の入力を作る。N と、X_1 ... X_N、C_1 ... C_N を出す。

i から X_i へ辺を張ると、各頂点から 1 本ずつ出るグラフになり、答えは閉路ごとの C の最小の和になる。
seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース、N = 10^5 か 2 × 10^5 のいろいろな形と、
小さいランダム。
形は、全体で 1 つの閉路、長さ 2 の閉路だけ、ランダム、長い尾の先に小さい閉路、全員が 1 を指すもの、
ランダムな置換 (全員が閉路の上)、長さ 3 の閉路に木が付いたもの。長さ 2 の閉路だけで C = 10^9 なら
答えは 10^14 で、32 ビットに収まらない。入力が大きくなりすぎないよう、C の幅はケースで変える。
seed が 1000 以上なら、配る順 P を全部試す愚直解で解ける N = 8 までの入力を出す (pj testdata crosscheck 用)。
"""

import random
import sys

MAX_N = 2 * 10**5
MAX_C = 10**9

Case = tuple[list[int], list[int]]

SAMPLES: list[Case] = [
    ([2, 3, 2], [1, 10, 100]),
    ([7, 3, 5, 5, 8, 4, 1, 2], [36, 49, 73, 38, 30, 85, 27, 45]),
]
FIXED: list[Case] = [
    ([2, 1], [5, 3]),
    ([2, 1], [MAX_C, MAX_C]),
    ([2, 3, 1], [1, 1, 1]),
    ([2, 1, 1], [7, 5, 1]),
]
# (N, 形, C の作り方)。本番のケースのうち角のケースのあとに並べる。
# C は、wide が 10^9 までのランダム、narrow が 1000 まで、one が全部 1、max が全部 10^9。
PLANS = [
    (MAX_N, "cycle", "wide"),
    (MAX_N, "pairs", "max"),
    (MAX_N, "random", "wide"),
    (MAX_N, "rho", "narrow"),
    (MAX_N, "funnel", "narrow"),
    (MAX_N, "perm", "narrow"),
    (MAX_N, "random", "one"),
    (10**5, "triangles", "wide"),
    (10**5, "perm", "max"),
    (10**5, "cycle_sorted", "narrow"),
    (1000, "random", "wide"),
    (1000, "triangles", "narrow"),
    (8, "random", "wide"),
    (8, "perm", "narrow"),
    (8, "triangles", "wide"),
]
SHAPES = ["cycle", "pairs", "random", "rho", "funnel", "perm", "triangles", "cycle_sorted"]


def functional(rng: random.Random, n: int, shape: str) -> list[int]:
    """頂点 0 から n - 1 の行き先 f (f[i] != i) を返す。番号は混ぜる前のもの。"""
    if shape in ("cycle", "cycle_sorted"):
        return [(i + 1) % n for i in range(n)]
    if shape == "pairs":
        f = [i ^ 1 for i in range(n)]
        if n % 2 == 1:
            f[n - 1] = rng.randrange(n - 1)
        return f
    if shape == "random":
        f = [rng.randrange(n - 1) for _ in range(n)]
        return [j + (j >= i) for i, j in enumerate(f)]
    if shape == "rho":
        # 0 -> 1 -> ... -> n - 1 と進み、最後の数個で閉路に戻る。
        k = min(n, rng.randint(2, 5))
        f = [i + 1 for i in range(n)]
        f[n - 1] = n - k
        return f
    if shape == "funnel":
        return [1] + [0] * (n - 1)
    if shape == "perm":
        # ランダムな置換の不動点を、ほかの頂点と行き先を入れ替えて消す。入れ替えで新しい不動点はできない。
        p = list(range(n))
        rng.shuffle(p)
        for i in range(n):
            if p[i] == i:
                j = rng.randrange(n - 1)
                j += j >= i
                p[i], p[j] = p[j], p[i]
        return p
    assert shape == "triangles"
    f = []
    cycles = n // 3
    for i in range(n):
        if i < 3 * cycles:
            f.append(i - i % 3 + (i + 1) % 3)
        else:
            f.append(rng.randrange(3 * cycles))
    # 半分の閉路を木に付け替える。付け替えた頂点はそれより前の閉路を指す。
    for c in range(1, cycles):
        if rng.random() < 0.5:
            v = 3 * c
            f[v] = rng.randrange(3 * c)
    return f


def costs(rng: random.Random, n: int, how: str) -> list[int]:
    if how == "wide":
        return [rng.randint(1, MAX_C) for _ in range(n)]
    if how == "narrow":
        return [rng.randint(1, 1000) for _ in range(n)]
    if how == "one":
        return [1] * n
    assert how == "max"
    return [MAX_C] * n


def build(rng: random.Random, n: int, shape: str, how: str) -> Case:
    f = functional(rng, n, shape)
    c = costs(rng, n, how)
    if shape == "cycle_sorted":
        return [v + 1 for v in f], c
    perm = list(range(n))
    rng.shuffle(perm)
    x, cc = [0] * n, [0] * n
    for i in range(n):
        x[perm[i]], cc[perm[i]] = perm[f[i]] + 1, c[i]
    return x, cc


def small_case(rng: random.Random) -> Case:
    """愚直解が N! 通りを試せる大きさ。C は同じ値や端の値を多めに混ぜる。"""
    n = rng.randint(2, 8)
    shape = rng.choice([s for s in SHAPES if n >= 3 or s != "triangles"])
    x, c = build(rng, n, shape, rng.choice(["wide", "narrow", "one", "max"]))
    if rng.random() < 0.3:
        c = [rng.choice([1, 2, 3, MAX_C, rng.randint(1, 100)]) for _ in range(n)]
    return x, c


def case_for(seed: int, rng: random.Random) -> Case:
    if seed >= 1000:
        return small_case(rng)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    return build(rng, *PLANS[seed - len(FIXED)])


def main() -> None:
    seed = int(sys.argv[1])
    x, c = case_for(seed, random.Random(seed))
    n = len(x)
    assert 2 <= n <= MAX_N and len(c) == n
    assert all(1 <= x[i] <= n and x[i] != i + 1 for i in range(n)) and all(1 <= v <= MAX_C for v in c)
    sys.stdout.write(f"{n}\n{' '.join(map(str, x))}\n{' '.join(map(str, c))}\n")


if __name__ == "__main__":
    main()
