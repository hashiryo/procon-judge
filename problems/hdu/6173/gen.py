"""hdu-6173 (Boring Game) の入力を作る。T と、各テストの N M と M 個の矩形 x1 y1 x2 y2 を出す。

答えは、表のマス (矩形の和集合) の lowbit(x) ⊗ lowbit(y) (nim 積) の xor が 0 でなければ先手 (Yong Chol) の勝ち。
1 <= x < 2^k なら x に 2^k の倍数を足しても lowbit は変わらないので、[1, 2^k - 1]^2 に収まる配置を 2^k の倍数だけずらして
重ならないように 2 つ置くと xor が 0 になる。これで後手 (Brother) の勝ちのテストを作る (3 つ置くと元の配置と同じ値になる)。
seed 0 は例、seed 1 は角のケース (N = 1、盤全体、同じ矩形の繰り返し、入れ子、接する矩形、1 行や 1 列、2 マスで打ち消すもの、
2 つ置いて打ち消すもの、3 つ置くもの)、seed 2 は M < 600 のテスト 200 個 (半分ほどは 2 つ置いて打ち消す)、
seed 3 から 7 は M = 10^5 の大きいテスト (ランダム、細長い、入れ子、1 マス、2 つ置いて打ち消す)。どのファイルも M の和は 6 × 10^5 以下。
seed が 1000 以上なら、マスを全部数える愚直解で解ける N <= 64 の入力を出す (pj testdata crosscheck 用)。
"""

import random
import sys

MAX_N = 10**9
MAX_M = 10**5
MAX_SUM_M = 6 * 10**5

Rect = tuple[int, int, int, int]
Test = tuple[int, list[Rect]]


def rect_in(rng: random.Random, lo: int, hi: int, shape: str = "random") -> Rect:
    """[lo, hi]^2 に収まる矩形。"""
    if shape == "cell":
        x, y = rng.randint(lo, hi), rng.randint(lo, hi)
        return x, y, x, y
    if shape == "thin":
        x, y1 = rng.randint(lo, hi), rng.randint(lo, hi)
        y2 = rng.randint(y1, hi)
        return (x, y1, x, y2) if rng.random() < 0.5 else (y1, x, y2, x)
    x1, y1 = rng.randint(lo, hi), rng.randint(lo, hi)
    return x1, y1, rng.randint(x1, hi), rng.randint(y1, hi)


def config(rng: random.Random, k: int, m: int, shape: str = "random") -> list[Rect]:
    return [rect_in(rng, 1, (1 << k) - 1, shape) for _ in range(m)]


def copies(rects: list[Rect], k: int, shifts: list[tuple[int, int]]) -> list[Rect]:
    s = 1 << k
    return [(x1 + dx * s, y1 + dy * s, x2 + dx * s, y2 + dy * s) for dx, dy in shifts for x1, y1, x2, y2 in rects]


def two_copies(rng: random.Random, k: int, m: int, blocks: int, shape: str = "random") -> list[Rect]:
    """[1, 2^k - 1]^2 の配置を、blocks × blocks の区画のうち別々の 2 つに置く (xor は 0 になる)。"""
    cells = rng.sample([(i, j) for i in range(blocks) for j in range(blocks)], 2)
    out = copies(config(rng, k, m, shape), k, cells)
    rng.shuffle(out)
    return out


def nested(rng: random.Random, n: int, m: int) -> list[Rect]:
    cx, cy = rng.randint(1, n), rng.randint(1, n)
    out = []
    for _ in range(m):
        out.append((rng.randint(1, cx), rng.randint(1, cy), rng.randint(cx, n), rng.randint(cy, n)))
    return out


SAMPLE = [(3, [(1, 2, 1, 3), (2, 1, 3, 1)]), (2, [(1, 1, 2, 2)])]


def corners(rng: random.Random) -> list[Test]:
    k = 5
    base = config(rng, k, 6)
    return [
        (1, [(1, 1, 1, 1)]),
        (MAX_N, [(1, 1, MAX_N, MAX_N)]),
        (MAX_N, [(5, 7, 10**8, 10**9 - 3)] * 3),
        (100, [(i, i, 101 - i, 101 - i) for i in range(1, 51)]),
        (MAX_N, [(1, 1, 10, 10), (11, 1, 20, 10), (1, 11, 20, 20), (21, 21, 21, 21)]),
        (8, [(1, 1, 8, 1)]),
        (MAX_N, [(1 << 29, 1, 1 << 29, MAX_N)]),
        (3, [(1, 1, 1, 1), (3, 1, 3, 1)]),
        (MAX_N, [(1, 1, 1, 1), (3, 3, 3, 3), (5, 1, 5, 1), (7, 3, 7, 3)]),
        (64, copies(base, k, [(0, 0), (1, 1)])),
        (96, copies(base, k, [(0, 0), (1, 0), (2, 2)])),
        (1 << 20, [(1, 1, 1 << 20, 1 << 20)]),
        (MAX_N, copies(config(rng, 28, 40), 28, [(0, 1), (2, 0)])),
        (MAX_N, copies(config(rng, 28, 40, "thin"), 28, [(1, 1), (2, 2)])),
    ]


def many_small(rng: random.Random) -> list[Test]:
    out = []
    for _ in range(200):
        m = rng.randint(1, 599)
        r = rng.random()
        if r < 0.5:
            k = rng.randint(1, 28)
            out.append((MAX_N, two_copies(rng, k, max(1, m // 2), 3 if k == 28 else 4, rng.choice(["random", "cell", "thin"]))))
        else:
            n = rng.choice([MAX_N, rng.randint(1, MAX_N), rng.randint(1, 1000)])
            out.append((n, [rect_in(rng, 1, n, rng.choice(["random", "cell", "thin"])) for _ in range(m)]))
    return out


def big(rng: random.Random, kind: str) -> list[Test]:
    if kind == "random":
        return [(MAX_N, [rect_in(rng, 1, MAX_N) for _ in range(MAX_M)]) for _ in range(2)]
    if kind == "thin":
        return [(MAX_N, [rect_in(rng, 1, MAX_N, "thin") for _ in range(MAX_M)]) for _ in range(2)]
    if kind == "nested":
        return [(MAX_N, nested(rng, MAX_N, MAX_M)), (MAX_N, nested(rng, MAX_N, MAX_M))]
    if kind == "cell":
        return [(MAX_N, [rect_in(rng, 1, MAX_N, "cell") for _ in range(MAX_M)]), (1000, [rect_in(rng, 1, 1000, "cell") for _ in range(MAX_M)])]
    assert kind == "copies"
    return [(MAX_N, two_copies(rng, 28, MAX_M // 2, 3, shape)) for shape in ("random", "thin", "cell")]


PLANS = ["random", "thin", "nested", "cell", "copies"]
COUNT = 3 + len(PLANS)


def small_case(rng: random.Random) -> list[Test]:
    out = []
    for _ in range(rng.randint(1, 5)):
        m = rng.randint(1, 12)
        if rng.random() < 0.4:
            k = rng.randint(1, 5)
            out.append((64, two_copies(rng, k, max(1, m // 2), 2)))
        else:
            n = rng.randint(1, 64)
            out.append((n, [rect_in(rng, 1, n, rng.choice(["random", "cell", "thin"])) for _ in range(m)]))
    return out


def case_for(seed: int) -> list[Test]:
    rng = random.Random(seed)
    if seed >= 1000:
        return small_case(rng)
    if seed == 0:
        return SAMPLE
    if seed == 1:
        return corners(rng)
    if seed == 2:
        return many_small(rng)
    return big(rng, PLANS[seed - 3])


def main() -> None:
    seed = int(sys.argv[1])
    tests = case_for(seed)
    assert 1 <= len(tests) <= 200 and sum(len(r) for _, r in tests) <= MAX_SUM_M
    out = [str(len(tests))]
    for n, rects in tests:
        assert 1 <= n <= MAX_N and 1 <= len(rects) <= MAX_M
        assert all(1 <= x1 <= x2 <= n and 1 <= y1 <= y2 <= n for x1, y1, x2, y2 in rects)
        out.append(f"{n} {len(rects)}")
        out += [f"{x1} {y1} {x2} {y2}" for x1, y1, x2, y2 in rects]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
