"""arc153-b (Grid Rotations) の入力を作る。H W と盤面、Q と Q 個の操作を出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、H = W = 2 などの角のケース、HW = 5 * 10^5 に
近い盤面で Q = 2 * 10^5 の大きいもの。形は、2 行だけ、2 列だけ、正方形に近いもの、横長と縦長。
操作は、ランダム、いつも端 (a = 1, b = 1 と a = H - 1, b = W - 1)、同じ操作のくり返し (偶数回なら元に
戻る)、2 つの操作の交互。盤面は文字をランダムに置くので、どのマスがどこへ動いたかが出力に残る。
seed が 1000 以上なら、4 つの長方形を実際に回す愚直解で解ける小さい入力を出す (pj testdata crosscheck 用)。
"""

import math
import random
import string
import sys

MAX_HW = 5 * 10**5
MAX_Q = 2 * 10**5

SAMPLES = [
    (["abcde", "fghij", "klmno", "pqrst"], [(3, 3)]),  # 例 1
    (["atcoder", "regular", "contest"], [(1, 1), (2, 5)]),  # 例 2
    (["ac", "wa"], [(1, 1), (1, 1), (1, 1)]),  # 例 3
]
# (H, W, Q, 操作の出し方)。本番のケースのうち例のあとに並べる。
PLANS = [
    (2, 2, 1, "random"),
    (2, 3, 1, "random"),
    (3, 2, 4, "random"),
    (2, 2, MAX_Q, "random"),  # 操作は (1, 1) しかなく、どの長方形も 1 マス
    (707, 707, 1, "random"),
    (2, MAX_HW // 2, MAX_Q, "random"),
    (MAX_HW // 2, 2, MAX_Q, "random"),
    (707, 707, MAX_Q, "random"),
    (500, 1000, MAX_Q, "low"),  # いつも a = 1, b = 1
    (1000, 500, MAX_Q, "high"),  # いつも a = H - 1, b = W - 1
    (700, 714, MAX_Q, "same"),  # 同じ操作を偶数回なので元に戻る
    (701, 713, MAX_Q - 1, "same"),  # 奇数回なので 1 回と同じ
    (100, 5000, MAX_Q, "alternate"),
    (None, None, None, "random"),  # H, W, Q はランダム
    (None, None, None, "alternate"),
]
COUNT = len(SAMPLES) + len(PLANS)


def operations(rng: random.Random, h: int, w: int, q: int, how: str) -> list[tuple[int, int]]:
    pick = lambda: (rng.randint(1, h - 1), rng.randint(1, w - 1))
    if how == "low":
        return [(1, 1)] * q
    if how == "high":
        return [(h - 1, w - 1)] * q
    if how == "same":
        return [pick()] * q
    if how == "alternate":
        first, second = pick(), pick()
        return [first if i % 2 == 0 else second for i in range(q)]
    if how == "edges":
        # 端の値を多めに混ぜる。
        side = lambda n: rng.choice([1, n - 1, rng.randint(1, n - 1)])
        return [(side(h), side(w)) for _ in range(q)]
    return [pick() for _ in range(q)]


def random_shape(rng: random.Random, most: int) -> tuple[int, int]:
    """2 <= H, W で HW <= most。H は対数で一様に選ぶので、縦長にも横長にも正方形にもなる。"""
    h = min(max(2, round(2 ** rng.uniform(1, math.log2(most // 2)))), most // 2)
    return h, rng.randint(max(2, most // h // 2), most // h)


def case_for(seed: int, rng: random.Random) -> tuple[list[str], list[tuple[int, int]]]:
    if seed >= 1000:
        # 愚直解が毎回盤面全体を動かしても間に合う大きさ。
        h, w = rng.randint(2, 12), rng.randint(2, 12)
        how = rng.choice(["random", "edges", "low", "high", "same", "alternate"])
        letters = rng.choice([string.ascii_lowercase, "ab"])
        grid = ["".join(rng.choices(letters, k=w)) for _ in range(h)]
        return grid, operations(rng, h, w, rng.randint(1, 30), how)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    h, w, q, how = PLANS[seed - len(SAMPLES)]
    if h is None:
        h, w = random_shape(rng, MAX_HW)
        q = rng.randint(1, MAX_Q)
    grid = ["".join(rng.choices(string.ascii_lowercase, k=w)) for _ in range(h)]
    return grid, operations(rng, h, w, q, how)


def main() -> None:
    seed = int(sys.argv[1])
    grid, ops = case_for(seed, random.Random(seed))
    h, w = len(grid), len(grid[0])
    assert 2 <= h and 2 <= w and h * w <= MAX_HW and 1 <= len(ops) <= MAX_Q
    assert all(len(row) == w and set(row) <= set(string.ascii_lowercase) for row in grid)
    assert all(1 <= a <= h - 1 and 1 <= b <= w - 1 for a, b in ops)
    out = [f"{h} {w}"] + grid + [str(len(ops))] + [f"{a} {b}" for a, b in ops]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
