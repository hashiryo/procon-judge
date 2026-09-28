"""abc275-h (Monster) の入力を作る。N と A_1 ... A_N と B_1 ... B_N を出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、N = 10^5 と 2 × 10^4 のいろいろな A と B。
提出は B の最大値で分けるデカルト木 (同じ値なら左が上) をたどり、区分線形凸関数を足し合わせる。
B が全部同じか単調なら木はパスになり、再帰が N 段になる。そのときの答えは、B が全部同じなら B × max A、
B が増加なら Σ B_i × max(0, A_i - max_{j > i} A_j)。A が全部同じなら A × max B。
B を 10^9 - 1 と 10^9 の交互にすると、どの頂点も子の B の和が 2 × 10^9 近くになり、傾きが int の端に近づく。
ほかに、B が狭い範囲 (同じ値だらけ)、10^9 の近く、山と谷の形、ビット反転の並び (釣り合った木)、A が 1 と 10^9 の交互など。
seed が 1000 以上なら、残りの体力の組を状態にした最短路の愚直解で解ける小さい入力を出す (pj testdata crosscheck 用)。
"""

import random
import sys

MAX_N = 10**5
MAX_V = 10**9
MID_N = 2 * 10**4

SAMPLES = [
    ([4, 3, 5, 1, 2], [10, 40, 20, 60, 50]),
    ([MAX_V], [MAX_V]),
    (
        [522, 4575, 6426, 9445, 8772, 81, 3447, 629, 3497, 7202],
        [7775, 4325, 3982, 4784, 8417, 2156, 1932, 5902, 5728, 8537],
    ),
]
FIXED = [
    ([1], [1]),
    ([MAX_V, MAX_V], [MAX_V, MAX_V]),
    ([2, 1, 2], [1, 10, 1]),  # 両端だけ別に削るほうが安い
    ([5, 5, 5, 5], [3, 1, 4, 1]),
]
# (N, A の作り方, B の作り方)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    (MAX_N, "random", "random"),
    (MAX_N, "max", "max"),  # 答えは 10^18
    (MAX_N, "random", "max"),  # B が全部同じ。パスで、答えは 10^9 × max A
    (MAX_N, "random", "increasing"),
    (MAX_N, "random", "decreasing"),
    (MAX_N, "random", "alternate_top"),
    (MAX_N, "random", "near_max"),
    (MAX_N, "random", "ties"),
    (MAX_N, "random", "bit_reversal"),
    (MAX_N, "random", "mountain"),
    (MID_N, "same", "random"),  # 答えは A × max B
    (MID_N, "small", "random"),
    (MID_N, "random", "valley"),
    (MID_N, "alternate", "random"),
    (MID_N, "random", "zigzag"),
]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)


def make_a(how: str, n: int, rng: random.Random) -> list[int]:
    if how == "random":
        return [rng.randint(1, MAX_V) for _ in range(n)]
    if how == "max":
        return [MAX_V] * n
    if how == "same":
        return [rng.randint(1, MAX_V)] * n
    if how == "small":
        return [rng.randint(1, 3) for _ in range(n)]
    assert how == "alternate"
    return [MAX_V if i % 2 == 0 else 1 for i in range(n)]


def make_b(how: str, n: int, rng: random.Random) -> list[int]:
    if how == "random":
        return [rng.randint(1, MAX_V) for _ in range(n)]
    if how == "max":
        return [MAX_V] * n
    if how == "increasing":
        return sorted(rng.sample(range(1, MAX_V + 1), n))
    if how == "decreasing":
        return sorted(rng.sample(range(1, MAX_V + 1), n), reverse=True)
    if how == "alternate_top":
        return [MAX_V - 1 if i % 2 == 0 else MAX_V for i in range(n)]
    if how == "near_max":
        return [rng.randint(MAX_V - 1000, MAX_V) for _ in range(n)]
    if how == "ties":
        return [rng.randint(1, 3) for _ in range(n)]
    if how == "bit_reversal":
        bits = (n - 1).bit_length()
        return [int(format(i, f"0{bits}b")[::-1], 2) + 1 for i in range(n)]
    if how == "mountain":
        half = sorted(rng.sample(range(1, MAX_V + 1), n))
        return half[0::2] + half[1::2][::-1]
    if how == "valley":
        half = sorted(rng.sample(range(1, MAX_V + 1), n))
        return half[0::2][::-1] + half[1::2]
    assert how == "zigzag"
    return [rng.randint(MAX_V // 2, MAX_V) if i % 2 else rng.randint(1, 1000) for i in range(n)]


def small_case(rng: random.Random) -> tuple[list[int], list[int]]:
    """愚直解の状態数 (A_1 + 1)...(A_N + 1) が 2 万を超えない大きさ。"""
    n = rng.randint(1, 8)
    cap = {1: 8, 2: 8, 3: 8, 4: 6, 5: 4, 6: 3, 7: 3, 8: 2}[n]
    a = [rng.randint(1, cap) for _ in range(n)]
    how = rng.choice(["random", "small", "max", "increasing", "decreasing", "ties"])
    if how == "random":
        b = [rng.randint(1, MAX_V) for _ in range(n)]
    elif how == "small":
        b = [rng.randint(1, 10) for _ in range(n)]
    elif how == "max":
        b = [rng.choice([MAX_V - 1, MAX_V]) for _ in range(n)]
    elif how == "ties":
        b = [rng.randint(1, 2) for _ in range(n)]
    else:
        b = sorted(rng.randint(1, 20) for _ in range(n))
        if how == "decreasing":
            b.reverse()
    return a, b


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 1000:
        a, b = small_case(rng)
    elif seed < len(SAMPLES):
        a, b = SAMPLES[seed]
    elif seed < len(SAMPLES) + len(FIXED):
        a, b = FIXED[seed - len(SAMPLES)]
    else:
        n, how_a, how_b = PLANS[seed - len(SAMPLES) - len(FIXED)]
        a, b = make_a(how_a, n, rng), make_b(how_b, n, rng)
    n = len(a)
    assert 1 <= n <= MAX_N and len(b) == n
    assert all(1 <= v <= MAX_V for v in a) and all(1 <= v <= MAX_V for v in b)
    sys.stdout.write(f"{n}\n{' '.join(map(str, a))}\n{' '.join(map(str, b))}\n")


if __name__ == "__main__":
    main()
