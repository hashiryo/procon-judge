"""cf-1615-g (Maximum Adjacent Pairs) の入力を作る。n と、n 個の a_i (0 <= a_i <= min(n, 600)) を出す。

答えの配列は 1 つに決まらないので、checker.cpp が入力と提出の出力を確かめ、値を期待出力の値と比べる。
seed が 0 から count - 1 までは本番のケース。例の 7 つ、角のケース (n = 2、全部 0、0 が無い)、制約いっぱいのランダム、
マッチングの形を作るもの (長さ 1 の区間ばかり、長さ 2 の区間ばかり、その混ぜ、値の鎖、値の種類が少ないもの、
すでに隣り合う組のある値が多いもの) を並べる。長さ 1 の区間は 2 つの端の値のどちらかを、長さ 2 の区間は両端を
一度に満たすので、混ぜると奇閉路 (花) ができる。seed が 1000 以上なら、n <= 8 で 0 が 6 個以下の入力を出す
(全部の埋め方を試す愚直解 brute.cpp 用)。
"""

import random
import sys

N_MAX = 300000
EXAMPLES = [
    [1, 1, 0, 2],
    [0, 0, 0, 0, 0],
    [1, 2, 3, 4, 5],
    [1, 0, 0, 0, 0, 1],
    [3, 0, 2],
    [1, 0, 2, 0, 1],
    [1, 0, 2, 3, 1, 0, 2],
]


def vmax(n: int) -> int:
    return min(n, 600)


def small(rng: random.Random) -> list[int]:
    n = rng.randint(2, 8)
    z = rng.randint(0, min(n, 6))
    pos = set(rng.sample(range(n), z))
    hi = rng.choice([2, 3, vmax(n)])
    return [0 if i in pos else rng.randint(1, min(hi, vmax(n))) for i in range(n)]


def random_case(rng: random.Random, n: int, p0: float, hi: int) -> list[int]:
    return [0 if rng.random() < p0 else rng.randint(1, hi) for _ in range(n)]


def segments(rng: random.Random, n: int, lens: list[int], value) -> list[int]:
    """値、0 の区間、値、0 の区間、... と並べる。区間の長さは lens から選び、値は value() で決める。"""
    a = [value()]
    while len(a) < n:
        a.extend([0] * rng.choice(lens))
        a.append(value())
    return a[:n]


def chain(rng: random.Random, n: int) -> list[int]:
    """値 1, 2, ..., 600 を長さ 1 の区間でつないだ鎖を繰り返し、ところどころ長さ 2 の区間と値の飛びを混ぜる。
    長い増加路と、長さ 2 の区間による奇閉路ができる。"""
    a = []
    v = 1
    while len(a) < n:
        a.append(v)
        a.extend([0] * (2 if rng.random() < 0.1 else 1))
        v = v % 600 + 1 if rng.random() < 0.97 else rng.randint(1, 600)
    return a[:n]


def with_pairs(rng: random.Random, n: int) -> list[int]:
    """値の半分ほどに、0 でない隣り合う組 (x x) を先に置いておく。その値は区間で満たしても増えない。"""
    paired = set(rng.sample(range(1, 601), 300))
    a = []
    while len(a) < n:
        v = rng.randint(1, 600)
        a.append(v)
        if v in paired and rng.random() < 0.3:
            a.append(v)
        a.extend([0] * rng.choice([1, 1, 2, 3]))
    return a[:n]


PLANS = [
    lambda rng: [0, 0],
    lambda rng: [0] * N_MAX,
    lambda rng: random_case(rng, N_MAX, 0.0, 600),
    lambda rng: random_case(rng, N_MAX, 0.5, 600),
    lambda rng: segments(rng, N_MAX, [1], lambda: rng.randint(1, 600)),
    lambda rng: segments(rng, N_MAX, [2], lambda: rng.randint(1, 600)),
    lambda rng: segments(rng, N_MAX, [1, 1, 2], lambda: rng.randint(1, 600)),
    lambda rng: chain(rng, N_MAX),
    lambda rng: segments(rng, N_MAX, [1, 2], lambda: rng.randint(1, 5)),
    lambda rng: segments(rng, N_MAX, [1, 2, 3, 4, 5], lambda: rng.randint(1, 600)),
    lambda rng: with_pairs(rng, N_MAX),
    lambda rng: random_case(rng, 600, 0.4, 600),
    lambda rng: random_case(rng, 1000, 0.3, 600),
]


def case_for(seed: int) -> list[int]:
    rng = random.Random(seed)
    if seed >= 1000:
        return small(rng)
    if seed < len(EXAMPLES):
        return EXAMPLES[seed]
    return PLANS[seed - len(EXAMPLES)](rng)


def main() -> None:
    seed = int(sys.argv[1])
    a = case_for(seed)
    n = len(a)
    assert 2 <= n <= N_MAX and all(0 <= x <= vmax(n) for x in a)
    sys.stdout.write(f"{n}\n{' '.join(map(str, a))}\n")


if __name__ == "__main__":
    main()
