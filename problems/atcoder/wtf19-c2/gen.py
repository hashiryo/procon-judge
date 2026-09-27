"""wtf19-c2 (Triangular Lamps Hard) の入力を作る。N と、点灯しているランプ x_i y_i を出す。

どのケースも、最初に点いていたランプ (X, Y) を決めてから、3 点 (x, y), (x + 2^k, y), (x, y + 2^k) を
切り替える操作をくり返して作る。k = 0 は問題の操作そのもので、2^k 倍の三角形は、GF(2) の上で
(1 + z + w)^(2^k) = 1 + z^(2^k) + w^(2^k) となるので、元の操作を何回か行ったものになる。
なので答えは作るときに決めた (X, Y) で、case_for がそれも返す。
seed が 0 から count - 1 までは本番のケース。例の 1 つ、角のケース、座標が ±10^17 に広がった N = 10^4 近くの
ケース。角のケースは、N = 1、元のランプが消えているもの、X や Y が入力の範囲 (±10^17) の外にあるもの
(X は 10^17 + 2^57 まで)、元のランプを 1 行ずつ下や左へ押し出して、2^13 個のランプが並んだもの。
seed が 1000 以上なら、ランプを 1 行ずつ下へ押し出す愚直解で解ける、広がりが 60 までの入力を出す
(pj testdata crosscheck 用)。座標全体は ±10^17 の端へずらすこともある。2000 以上なら、愚直解でまだ解ける
広がりが 1000 までの入力を出す。
"""

import random
import sys

MAX_N = 10**4
LIM = 10**17

Point = tuple[int, int]
Case = tuple[Point, list[Point]]


def toggle(lamps: set[Point], x: int, y: int, k: int) -> None:
    for p in ((x, y), (x + (1 << k), y), (x, y + (1 << k))):
        lamps ^= {p}


def push_down(x: int, y: int, m: int) -> list[Point]:
    """(x, y) を m 行下へ押し出したランプ。(x + j, y - m) のうち C(m, j) が奇数のもの。"""
    return [(x + j, y - m) for j in range(m + 1) if j & m == j]


def push_left(x: int, y: int, m: int) -> list[Point]:
    """(x, y) を m 列左へ押し出したランプ。(x - m, y + j) のうち C(m, j) が奇数のもの。"""
    return [(x - m, y + j) for j in range(m + 1) if j & m == j]


def sparse_down(x: int, y: int, m: int) -> list[Point]:
    """push_down と同じだが、m の立っているビットの部分集合だけを並べる (m が大きくても速い)。"""
    bits = [1 << b for b in range(m.bit_length()) if m >> b & 1]
    out = []
    for s in range(1 << len(bits)):
        out.append((x + sum(bits[i] for i in range(len(bits)) if s >> i & 1), y - m))
    return out


SAMPLE: Case = ((-1, 0), [(-2, 1), (-2, 2), (0, 1), (1, 0)])
FIXED: list[Case] = [
    ((0, 0), [(0, 0)]),
    ((LIM, LIM), [(LIM, LIM)]),
    ((-LIM, -LIM), [(-LIM, -LIM)]),
    ((0, 0), [(1, 0), (0, 1)]),  # 元のランプは消えている
    ((LIM + 1, 0), [(LIM, 0), (LIM, 1)]),  # X が範囲の外
    ((0, LIM + 1), [(0, LIM), (1, LIM)]),
    ((LIM + 2**57, -LIM), [(LIM, -LIM), (LIM, -LIM + 2**57)]),
    ((-LIM, LIM + 2**57), [(-LIM, LIM), (-LIM + 2**57, LIM)]),
    ((-LIM, -LIM), [(-LIM + 2**57, -LIM), (-LIM, -LIM + 2**57)]),
    ((12345, -678), push_down(12345, -678, 2**13 - 1)),  # 横に 2^13 個
    ((LIM - 8191, -LIM + 8191), push_down(LIM - 8191, -LIM + 8191, 2**13 - 1)),
    ((LIM, -5), push_left(LIM, -5, 2**13 - 1)),  # 縦に 2^13 個
    ((-LIM, LIM - 5), sparse_down(-LIM, LIM - 5, 2**56 + 2**40 + 2**23 + 7)),  # 遠く離れた 2^6 個
]
# (三角形の大きさ 2^k の k の範囲, 最初のランプの置き方, 目標の N)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    ((0, 57), "random", MAX_N),
    ((0, 57), "random", MAX_N),
    ((0, 5), "random", MAX_N),  # 小さい三角形ばかりで、元のランプのまわりに固まる
    ((50, 57), "random", MAX_N),
    ((0, 57), "corner", MAX_N),  # 元のランプが (10^17, -10^17)
    ((0, 57), "erased", MAX_N),  # 最初の三角形で元のランプを消す
    ((0, 20), "random", 3000),
    ((0, 8), "medium", 3000),  # 愚直解でも解ける広がり
]
COUNT = 1 + len(FIXED) + len(PLANS)


def spread(rng: random.Random, lamps: set[Point], k_range: tuple[int, int], lo: int, hi: int, near: Point | None, target: int) -> None:
    """ランプが target 個に近づくまで三角形を切り替える。三角形は [lo, hi] の正方形に収める。
    near があれば、三角形の角をそこから 150 以内に置く。
    """
    for _ in range(20 * target):
        if len(lamps) + 3 > target:
            break
        k = rng.randint(*k_range)
        size = 1 << k
        if near is None:
            x, y = rng.randint(lo, hi - size), rng.randint(lo, hi - size)
        else:
            x = min(max(near[0] + rng.randint(-150, 150), lo), hi - size)
            y = min(max(near[1] + rng.randint(-150, 150), lo), hi - size)
        toggle(lamps, x, y, k)


def plan_case(rng: random.Random, k_range: tuple[int, int], how: str, target: int) -> Case:
    if how == "medium":
        lo, hi = -500, 500
    else:
        lo, hi = -LIM, LIM
    if how == "corner":
        start = (LIM, -LIM)
    else:
        start = (rng.randint(lo, hi), rng.randint(lo, hi))
    lamps = {start}
    if how == "erased":
        k = rng.randint(*k_range)
        size = 1 << k
        # start を 3 つの角のどれかに持つ三角形。
        corner = rng.randrange(3)
        x, y = start[0] - (size if corner == 1 else 0), start[1] - (size if corner == 2 else 0)
        if lo <= x and x + size <= hi and lo <= y and y + size <= hi:
            toggle(lamps, x, y, k)
    near = start if k_range[1] <= 5 else None
    spread(rng, lamps, k_range, lo, hi, near, target)
    return start, sorted(lamps)


def small_case(rng: random.Random, width: int, k_max: int, toggles: int) -> Case:
    """広がりが width までの入力。座標全体を ±10^17 の端へずらすこともある。"""
    shift = rng.choice([
        (0, 0),
        (LIM - width, LIM - width),
        (-LIM, -LIM),
        (LIM - width, -LIM),
        (rng.randint(-LIM, LIM - width), rng.randint(-LIM, LIM - width)),
    ])
    start = (rng.randint(0, width), rng.randint(0, width))
    lamps = {start}
    for _ in range(rng.randint(0, toggles)):
        k = rng.randint(0, k_max)
        size = 1 << k
        x, y = rng.randint(0, width - size), rng.randint(0, width - size)
        if rng.random() < 0.2:
            x, y = min(start[0], width - size), min(start[1], width - size)
        toggle(lamps, x, y, k)
    sx, sy = shift
    return (start[0] + sx, start[1] + sy), sorted((x + sx, y + sy) for x, y in lamps)


def case_for(seed: int, rng: random.Random) -> Case:
    if seed >= 2000:
        return small_case(rng, 1000, 8, 1500)
    if seed >= 1000:
        return small_case(rng, 60, 4, 30)
    if seed == 0:
        return SAMPLE
    if seed - 1 < len(FIXED):
        return FIXED[seed - 1]
    return plan_case(rng, *PLANS[seed - 1 - len(FIXED)])


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    _, lamps = case_for(seed, rng)
    if seed != 0:
        rng.shuffle(lamps)
    assert 1 <= len(lamps) <= MAX_N and len(set(lamps)) == len(lamps)
    assert all(-LIM <= x <= LIM and -LIM <= y <= LIM for x, y in lamps)
    out = [str(len(lamps))] + [f"{x} {y}" for x, y in lamps]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
