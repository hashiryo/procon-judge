"""abc167-d (Teleporter) の入力を作る。N K と A_1 ... A_N を出す。

seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース、N = 2 × 10^5 の行き先。
行き先の形は、ランダム、全部を通る 1 つの輪、町 1 から長い道のあとで自分に戻る町 (K が道の長さの
前後)、道と輪が半分ずつの ρ の形 (K が輪に入る所ちょうどと、輪を回りきる所ちょうど)、ランダムな順列
(町 1 の輪の長さがまちまち)、全部が町 1、町 1 から輪までの道に木がたくさんぶら下がる形。
K は、1、10^18、ランダム、道と輪の長さに合わせた値を混ぜる。
seed が 1000 以上なら、2^j 回先を倍々で表にする愚直解で解ける小さい入力を出す
(pj testdata crosscheck 用)。愚直解は N = 2 × 10^5 でも速いので、本番のケースも全部突き合わせられる。
"""

import random
import sys

MAX_N = 2 * 10**5
MAX_K = 10**18

SAMPLES = [
    (5, [3, 2, 4, 1]),
    (727202214173249351, [6, 5, 2, 5, 3, 2]),
]
FIXED = [
    (1, [1, 1]),  # 町 1 は自分に戻る
    (MAX_K, [2, 1]),  # 長さ 2 の輪で K が偶数
    (MAX_K - 1, [2, 1]),
    (1, [2, 2]),
    (MAX_K, [2, 2]),
    (4, [2, 3, 4, 5, 5]),  # 道を渡りきった所で止まる
    (3, [2, 3, 4, 5, 5]),
    (MAX_K, [2, 3, 4, 5, 3]),
]
# (形, K の選び方)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    ("random", "max"),
    ("random", "random"),
    ("cycle", "random"),
    ("tail", "before_end"),
    ("tail", "at_end"),
    ("rho", "entry"),
    ("rho", "lap"),
    ("permutation", "max"),
    ("all_one", "max"),
    ("hairy", "random"),
    ("hairy", "max"),
]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)


def relabel(rng: random.Random, nxt: list[int], start: int) -> list[int]:
    """0 始まりの行き先 nxt の頂点を並べ替え、start を町 1 にして 1 始まりで返す。"""
    n = len(nxt)
    rest = [v for v in range(n) if v != start]
    rng.shuffle(rest)
    label = {start: 1}
    for i, v in enumerate(rest):
        label[v] = i + 2
    a = [0] * n
    for v in range(n):
        a[label[v] - 1] = label[nxt[v]]
    return a


def shape(rng: random.Random, n: int, how: str) -> tuple[list[int], int, int]:
    """(1 始まりの A, 町 1 から輪に入るまでの歩数, 輪の長さ) を返す。道と輪の長さは作った形から分かる分だけ。"""
    if how == "random":
        return [rng.randint(1, n) for _ in range(n)], -1, -1
    if how == "cycle":
        order = list(range(n))
        rng.shuffle(order)
        nxt = [0] * n
        for i in range(n):
            nxt[order[i]] = order[(i + 1) % n]
        return relabel(rng, nxt, order[0]), 0, n
    if how == "tail":
        # 0 -> 1 -> ... -> n - 1 -> n - 1
        return relabel(rng, [min(v + 1, n - 1) for v in range(n)], 0), n - 1, 1
    if how == "rho":
        t = n // 2  # 道の長さ。t から n - 1 が輪
        return relabel(rng, [v + 1 if v < n - 1 else t for v in range(n)], 0), t, n - t
    if how == "permutation":
        p = list(range(1, n + 1))
        rng.shuffle(p)
        return p, -1, -1
    if how == "all_one":
        return [1] * n, 0, 1
    assert how == "hairy"
    # 長さ n / 4 の道の先に長さ n / 4 の輪。残りの頂点は、道か輪のどこかへ向かう木にする。
    t = c = max(1, n // 4)
    nxt = [v + 1 if v < t + c - 1 else t for v in range(t + c)]
    for v in range(t + c, n):
        nxt.append(rng.randrange(v))
    return relabel(rng, nxt, 0), t, c


def pick_k(rng: random.Random, how: str, tail: int, cycle: int) -> int:
    if how == "max":
        return MAX_K
    if how == "random":
        return rng.randint(1, MAX_K)
    if how == "before_end":
        return tail - 1
    if how == "at_end":
        return tail
    if how == "entry":
        return tail
    assert how == "lap"
    # 輪を回りきって、輪に入った所へ戻る K のうち大きいもの。
    return tail + (MAX_K - tail) // cycle * cycle


def small_case(rng: random.Random) -> tuple[int, list[int]]:
    n = rng.randint(2, 12)
    how = rng.choice(["random", "cycle", "tail", "rho", "permutation", "hairy"])
    a, tail, cycle = shape(rng, n, how)
    if tail >= 1 and rng.random() < 0.5:
        # 輪に入る所の前後。輪を何周かした分を足したものも混ぜる。
        laps = rng.choice([0, 1, MAX_K // cycle - 2])
        k = min(MAX_K, max(1, tail + rng.randint(-1, 1) + laps * cycle))
    else:
        k = rng.choice([rng.randint(1, 30), rng.randint(1, MAX_K), MAX_K])
    return k, a


def case_for(seed: int, rng: random.Random) -> tuple[int, list[int]]:
    if seed >= 1000:
        return small_case(rng)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    how_shape, how_k = PLANS[seed - len(FIXED)]
    a, tail, cycle = shape(rng, MAX_N, how_shape)
    return pick_k(rng, how_k, tail, cycle), a


def main() -> None:
    seed = int(sys.argv[1])
    k, a = case_for(seed, random.Random(seed))
    n = len(a)
    assert 2 <= n <= MAX_N and 1 <= k <= MAX_K and all(1 <= x <= n for x in a)
    sys.stdout.write(f"{n} {k}\n{' '.join(map(str, a))}\n")


if __name__ == "__main__":
    main()
