"""abc213-h (Stroll) の入力を作る。N M T と、M 本の道 a_i b_i と p_(i,1) ... p_(i,T) を出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、T = 4 × 10^4 のいろいろな道の置き方。
角のケースは、T = 1 (1 回で戻れないので答え 0)、家に繋がらない道だけ (答え 0)、p が全部 998244352。
大きいケースは、ランダム、5 点の完全グラフ、家を中心にした星、道、2 点だけ、p がところどころだけ 0 でないもの、
p が全部 998244352、全部 1、長さ 1 の道だけ (答えは隣接行列の T 乗の (1, 1) 成分になる)、長い道だけ、
家からは 1 本だけのもの。lib.cpp の RelaxedConvolution は長さが 2 のべきの所で大きい畳み込みをするので、
T = 32767、32768、32769 も入れる。p を 10^9 近くまでにすると 1 ケースで 4 MB になるので、半分ほどは p を小さくする。
seed が 1000 以上なら、長さの和の小さい順に道順を数える O(M T^2) の愚直解で解ける小さい T の入力を出す
(pj testdata crosscheck 用)。2000 以上なら T を 2000 までにする。愚直解は T = 4 × 10^4 でも 10 秒ほどなので、
本番のケースも全部突き合わせられる。
"""

import random
import sys

P = 998244353
MAX_T = 4 * 10**4

Road = tuple[int, int, list[int]]

SAMPLES: list[tuple[int, int, list[Road]]] = [
    (3, 2, [(1, 2, [1, 0]), (1, 3, [2, 0])]),
    (3, 4, [(1, 2, [3, 0, 0, 0]), (1, 3, [0, 1, 0, 0]), (2, 3, [2, 0, 0, 0])]),
    (2, 5, [(1, 2, [31415, 92653, 58979, 32384, 62643])]),
]
FIXED: list[tuple[int, int, list[Road]]] = [
    (2, 1, [(1, 2, [1])]),  # 答え 0
    (2, 2, [(1, 2, [1, 0])]),  # 1 → 2 → 1 の 1 通り
    (2, 2, [(1, 2, [0, 5])]),  # 長さ 2 の道で行くと戻れない
    (3, 5, [(2, 3, [1, 1, 1, 1, 1])]),  # 家に繋がらない
    (3, 6, [(1, 2, [P - 1] * 6), (2, 3, [P - 1] * 6)]),
]
# (N, T, 道の置き方, p の出し方)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    (10, MAX_T, "random", "full"),
    (5, MAX_T, "complete", "full"),
    (10, MAX_T, "star", "small"),
    (10, MAX_T, "path", "small"),
    (2, MAX_T, "random", "full"),
    (10, MAX_T, "random", "sparse"),
    (10, MAX_T, "random", "max"),
    (10, MAX_T, "random", "ones"),
    (10, MAX_T, "random", "length1"),
    (10, MAX_T - 1, "random", "length1"),
    (10, MAX_T, "random", "long_only"),
    (10, MAX_T, "one_from_home", "small"),
    (10, 32767, "random", "small"),
    (10, 32768, "random", "small"),
    (10, 32769, "random", "small"),
]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)
LAYOUTS = ["random", "complete", "star", "path", "one_from_home"]
VALUES = ["full", "small", "sparse", "max", "ones", "length1", "long_only"]


def roads(rng: random.Random, n: int, layout: str) -> list[tuple[int, int]]:
    """M 本の道 (a < b)。M は 10 と n(n - 1) / 2 の小さいほうまで。"""
    pairs = [(a, b) for a in range(1, n + 1) for b in range(a + 1, n + 1)]
    limit = min(10, len(pairs))
    if layout == "complete":
        chosen = pairs[:limit]
    elif layout == "star":
        chosen = [(1, b) for b in range(2, n + 1)][:limit]
        chosen += [e for e in pairs if e not in chosen][: limit - len(chosen)]
    elif layout == "path":
        chosen = [(i, i + 1) for i in range(1, n)][:limit]
    elif layout == "one_from_home":
        others = [e for e in pairs if e[0] != 1]
        chosen = [(1, 2)] + rng.sample(others, min(limit - 1, len(others)))
    else:
        chosen = rng.sample(pairs, rng.randint(1, limit) if n <= 4 else limit)
        if all(a != 1 for a, _ in chosen):
            # 家に道が無いと答えが 0 に決まり、畳み込みも走らないので、1 本を家からの道にする。
            chosen[0] = (1, rng.randint(2, n))
    rng.shuffle(chosen)
    return chosen


def lengths(rng: random.Random, t: int, how: str) -> list[int]:
    """p_(i,1) ... p_(i,T)。"""
    if how == "full":
        return [rng.randrange(P) for _ in range(t)]
    if how == "small":
        return [rng.randrange(10) for _ in range(t)]
    if how == "max":
        return [P - 1] * t
    if how == "ones":
        return [1] * t
    p = [0] * t
    if how == "sparse":
        # 長い長さだけだと和がちょうど T になる組がまず無いので、50 までの長さも 3 つ入れる。
        for d in rng.sample(range(min(t, 50)), min(t, 3)) + rng.sample(range(t), min(t, 3)):
            p[d] = rng.randrange(1, P)
    elif how == "length1":
        p[0] = rng.randrange(1, P)
    else:
        assert how == "long_only"
        for d in range(t // 3, t):
            p[d] = rng.randrange(3)
    return p


def case_for(seed: int, rng: random.Random) -> tuple[int, int, list[Road]]:
    if seed >= 1000:
        n = rng.randint(2, 10)
        t = rng.randint(1, 2000) if seed >= 2000 else rng.randint(1, 30)
        how = rng.choice(VALUES)
        return n, t, [(a, b, lengths(rng, t, how)) for a, b in roads(rng, n, rng.choice(LAYOUTS))]
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    n, t, layout, how = PLANS[seed - len(FIXED)]
    return n, t, [(a, b, lengths(rng, t, how)) for a, b in roads(rng, n, layout)]


def main() -> None:
    seed = int(sys.argv[1])
    n, t, rs = case_for(seed, random.Random(seed))
    m = len(rs)
    assert 2 <= n <= 10 and 1 <= m <= min(10, n * (n - 1) // 2) and 1 <= t <= MAX_T
    assert len({(a, b) for a, b, _ in rs}) == m and all(1 <= a < b <= n for a, b, _ in rs)
    assert all(len(p) == t and all(0 <= v < P for v in p) for _, _, p in rs)
    out = [f"{n} {m} {t}"]
    for a, b, p in rs:
        out += [f"{a} {b}", " ".join(map(str, p))]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
