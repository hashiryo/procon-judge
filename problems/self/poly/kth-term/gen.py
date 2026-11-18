#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""定数係数の線形漸化式の第 k 項の入力を作る (mod 998244353)。

seed ごとに (次数, ケース数) を決める。0 は小さい例、1〜7 は次数ごとの大きさ、8 と 9 は角のケース。
1000 以上の seed は愚直解 (brute.hpp) と突き合わせるための小さい入力 (次数 6 以下、k ≤ 3000)。
"""
import random
import sys

P = 998244353
BIG = 10**18
# seed: (次数の範囲, ケース数)
SIZES = {
    1: ((1, 1), 100000),
    2: ((2, 8), 30000),
    3: ((16, 64), 2000),
    4: ((100, 100), 200),
    5: ((300, 300), 20),
    6: ((1000, 1000), 3),
    7: ((3000, 3000), 1),
}


def case(rng: random.Random, d: int, k: int, zero_c: bool = False, zero_a: bool = False) -> str:
    a = [0 if zero_a else rng.randrange(P) for _ in range(d)]
    c = [0 if zero_c else rng.randrange(P) for _ in range(d)]
    return f"{d} {k}\n{' '.join(map(str, a))}\n{' '.join(map(str, c))}"


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed * 1_000_003 + 7)
    cs = []
    if seed == 0:
        cs.append("2 10\n0 1\n1 1")  # フィボナッチ数の第 10 項 = 55
        cs.append("2 0\n0 1\n1 1")
        cs.append("3 5\n1 2 3\n0 0 1")
        cs.append("1 1000000000000000000\n1\n2")
    elif seed in SIZES:
        (lo, hi), t = SIZES[seed]
        for i in range(t):
            d = rng.randint(lo, hi)
            k = rng.randrange(d) if i % 50 == 49 else rng.randint(BIG // 2, BIG)
            cs.append(case(rng, d, k))
    elif seed == 8:  # k < d、k = 0、c がすべて 0、a がすべて 0、最大の k
        for d in (1, 2, 5, 3000):
            cs.append(case(rng, d, 0))
            cs.append(case(rng, d, d - 1))
            cs.append(case(rng, d, BIG, zero_c=True))
            cs.append(case(rng, d, BIG, zero_a=True))
            cs.append(case(rng, d, BIG))
    elif seed == 9:  # c_d = 0 (漸化式が見かけより短い) と、c が 1 か P - 1 だけ
        for d in (3, 64, 1500):
            a = [rng.randrange(P) for _ in range(d)]
            c = [rng.randrange(P) for _ in range(d)]
            c[-1] = 0
            cs.append(f"{d} {BIG}\n{' '.join(map(str, a))}\n{' '.join(map(str, c))}")
            c = [rng.choice((1, P - 1)) for _ in range(d)]
            cs.append(f"{d} {BIG - 1}\n{' '.join(map(str, a))}\n{' '.join(map(str, c))}")
    else:  # 1000 以上: 愚直解と突き合わせる小さい入力
        for _ in range(60):
            d = rng.randint(1, 6)
            cs.append(case(rng, d, rng.randint(0, 3000)))
    sys.stdout.write(f"{len(cs)}\n" + "\n".join(cs) + "\n")


if __name__ == "__main__":
    main()
