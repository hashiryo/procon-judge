"""atcoder-abc191-d (Circle Lattice Points) の入力を作る。

中心 (X, Y)、半径 R の円の内部または周上にある格子点の数を答える。X, Y, R は小数第 4 位まで。
seed が 0 から count - 1 までが本番 (例、整数、端点、ランダム、最大)。
seed が 1000 以上なら、半径 3 以下の小さい円を出す (pj testdata crosscheck 用)。
"""

import random
import sys

SAMPLES = [
    ("0.2 0.8 1.1\n",),
    ("100 100 1\n",),
    ("42782.4720 31949.0192 99999.99\n",),
]


def fmt(v, rng):
    # 小数第 4 位までの文字列。整数のこともある。
    if rng.random() < 0.5:
        return str(v)
    return f"{v + rng.randint(0, 9999) / 10000:.4f}"


def main():
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 1000:
        s = f"{rng.uniform(-10, 10):.4f} {rng.uniform(-10, 10):.4f} {rng.uniform(0.1, 3):.4f}\n"
    else:
        plans = [
            SAMPLES[0][0],
            SAMPLES[1][0],
            SAMPLES[2][0],
            "0 0 1\n",
            "0 0 100000\n",
            "-100000 -100000 1\n",
            "0.0001 0.0001 0.0001\n",
            f"{rng.uniform(-1000, 1000):.4f} {rng.uniform(-1000, 1000):.4f} {rng.uniform(1, 1000):.4f}\n",
            f"{rng.uniform(-100000, 100000):.4f} {rng.uniform(-100000, 100000):.4f} {rng.uniform(1000, 100000):.4f}\n",
        ]
        s = plans[seed % len(plans)]
    sys.stdout.write(s)


main()
