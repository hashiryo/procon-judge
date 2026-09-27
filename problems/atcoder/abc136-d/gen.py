"""abc136-d (Gathering Children) の入力を作る。L と R からなる S を 1 行で出す (先頭は R、末尾は L)。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、|S| = 10^5 前後のいろいろな形。
子供は RL の境目の 2 マスに集まり、どちらに着くかは境目までの距離の偶奇で決まる。形は、RL のくり返し、
R だけ長いもの、L だけ長いもの、半分ずつ (長さの偶奇を変える)、同じ長さの塊のくり返し、ランダム。
Period は関数グラフを HLD で持ち、10^100 を BigInt で渡して周期で割るので、境目までの道が
10^5 近くになるもの (R...RL と RL...L) も入れる。
seed が 1000 以上なら、10^100 回の代わりに |S| 以上の偶数回だけ動かす愚直解で解ける小さい S を出す
(pj testdata crosscheck 用)。2000 以上なら、愚直解でまだ解ける |S| ≤ 5000 の S を出す。
"""

import random
import sys

MAX = 10**5
HALF = MAX // 2

FIXED = [
    "RRLRL",  # 例 1
    "RRLLLLRLRRLL",  # 例 2
    "RRRLLRLLRRRLLLLL",  # 例 3
    "RL",
    "RRL",
    "RLL",
    "RLRL",
    "R" * (MAX - 1) + "L",
    "R" + "L" * (MAX - 1),
    "RL" * HALF,
    "R" * HALF + "L" * HALF,
    "R" * (HALF + 1) + "L" * (HALF - 1),
    "R" * HALF + "L" * (HALF - 1),  # |S| が奇数
    "RRLL" * (MAX // 4),
    ("R" * 317 + "L" * 317) * (MAX // 634),
    "RRRL" * (MAX // 4),
]
RANDOM = 8


def runs(rng: random.Random, n: int, longest: int) -> str:
    """R の塊と L の塊を交互に並べる。塊の長さは 1 から longest。"""
    out, c = [], "R"
    while len(out) < n:
        out += [c] * rng.randint(1, longest)
        c = "L" if c == "R" else "R"
    s = out[:n]
    s[0], s[-1] = "R", "L"
    return "".join(s)


def random_case(rng: random.Random, i: int) -> str:
    kind = i % 4
    if kind == 0:
        s = [rng.choice("RL") for _ in range(MAX)]
        s[0], s[-1] = "R", "L"
        return "".join(s)
    if kind == 1:
        return runs(rng, MAX, 30)
    if kind == 2:
        return runs(rng, MAX, 20000)
    return runs(rng, rng.randint(2, MAX), rng.choice([2, 100, 5000]))


def small_case(rng: random.Random, limit: int) -> str:
    n = rng.randint(2, limit)
    if rng.random() < 0.3:
        s = [rng.choice("RL") for _ in range(n)]
        s[0], s[-1] = "R", "L"
        return "".join(s)
    return runs(rng, n, rng.choice([1, 2, 3, 5, 10, limit]))


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 2000:
        s = small_case(rng, 5000)
    elif seed >= 1000:
        s = small_case(rng, 40)
    elif seed < len(FIXED):
        s = FIXED[seed]
    else:
        s = random_case(rng, seed - len(FIXED))
    assert 2 <= len(s) <= MAX and set(s) <= set("RL") and s[0] == "R" and s[-1] == "L"
    print(s)


if __name__ == "__main__":
    main()
