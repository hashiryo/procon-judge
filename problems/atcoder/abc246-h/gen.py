"""abc246-h (01? Queries) の入力を作る。N Q と S、続く Q 行の x c を出す。

seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース (N = 1)、N = Q = 10^5 のケース。
大きいケースは、S が全部 '?'、全部 '0'、ランダム、'0' と '1' の交互、'?' が 9 割のもの。更新の位置は、ランダム、
先頭だけ、末尾だけ、1 から N の順。更新の文字は、ランダムと '?' だけ。N は 2 の冪 (65536)、その 1 つ上 (65537)、
奇数 (99999) も入れる。セグメント木の葉の数の切り上げ方で、末尾の扱いが変わるため。
seed が 1000 以上なら、'?' の置き換え方と部分列を全部並べて数える愚直解で解ける小さい入力 (N, Q ≤ 8) を出す
(pj testdata crosscheck --brute brute.hpp 用)。
"""

import random
import sys

MAX = 10**5
CH = "01?"

SAMPLES = [
    "3 3\n100\n2 1\n2 ?\n3 ?\n",
    "40 10\n011?0??001??10?0??0?0?1?11?1?00?11??0?01\n5 0\n2 ?\n30 ?\n7 1\n11 1\n3 1\n25 1\n40 0\n12 1\n18 1\n",
]


def make(rng: random.Random, n: int, q: int, s_how: str, pos_how: str, ch_how: str) -> str:
    if s_how == "all?":
        s = "?" * n
    elif s_how == "all0":
        s = "0" * n
    elif s_how == "alt":
        s = "".join("01"[i % 2] for i in range(n))
    elif s_how == "mostly?":
        s = "".join("?" if rng.random() < 0.9 else rng.choice("01") for _ in range(n))
    else:
        s = "".join(rng.choice(CH) for _ in range(n))
    lines = [f"{n} {q}", s]
    for i in range(q):
        if pos_how == "first":
            x = 1
        elif pos_how == "last":
            x = n
        elif pos_how == "seq":
            x = i % n + 1
        else:
            x = rng.randint(1, n)
        c = "?" if ch_how == "q" else rng.choice(CH)
        lines.append(f"{x} {c}")
    return "\n".join(lines) + "\n"


PLANS = [
    (1, 1, "random", "random", "random"),
    (1, 5, "all?", "random", "random"),
    (MAX, MAX, "all?", "random", "q"),
    (MAX, MAX, "all0", "random", "random"),
    (MAX, MAX, "random", "random", "random"),
    (MAX, MAX, "random", "first", "random"),
    (MAX, MAX, "random", "last", "random"),
    (MAX, MAX, "alt", "random", "random"),
    (MAX, MAX, "random", "random", "q"),
    (MAX, MAX, "mostly?", "random", "random"),
    (MAX, MAX, "random", "seq", "random"),
    (99999, MAX, "random", "random", "random"),
    (65537, MAX, "random", "last", "random"),
    (65536, MAX, "random", "random", "random"),
]
COUNT = len(SAMPLES) + len(PLANS)


def main() -> None:
    seed = int(sys.argv[1])
    if seed >= 1000:
        rng = random.Random(seed)
        n, q = rng.randint(1, 8), rng.randint(1, 8)
        sys.stdout.write(make(rng, n, q, "random", "random", "random"))
        return
    if seed >= COUNT:
        raise SystemExit(f"seed {seed} is out of range (count = {COUNT})")
    if seed < len(SAMPLES):
        sys.stdout.write(SAMPLES[seed])
        return
    rng = random.Random(seed)
    sys.stdout.write(make(rng, *PLANS[seed - len(SAMPLES)]))


if __name__ == "__main__":
    main()
