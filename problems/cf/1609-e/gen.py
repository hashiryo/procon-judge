"""cf-1609-e (William The Oblivious) の入力を作る。n q と s、続く q 行の pos c を出す。

seed が 0 から count - 1 までは本番のケース。例の 1 つ、角のケース (n = 1)、n = q = 10^5 のケース。
大きいケースは、s が全部 'a'、全部 'c'、"abc" の繰り返し、"cba" の繰り返し、ランダムなもの。更新の位置は、ランダム、
先頭だけ、末尾だけ、1 から n の順。更新の文字は、ランダムと 'b' だけ。n は 2 の冪 (65536)、その 1 つ上 (65537)、
奇数 (99999) も入れる。
seed が 1000 以上なら、3^n 通りの列を全部試す愚直解で解ける小さい入力 (n, q ≤ 7) を出す
(pj testdata crosscheck --brute brute.hpp 用)。
"""

import random
import sys

MAX = 10**5
CH = "abc"

SAMPLES = [
    "9 12\naaabccccc\n4 a\n4 b\n2 b\n5 a\n1 b\n6 b\n5 c\n2 a\n1 a\n5 a\n6 b\n7 b\n",
]


def make(rng: random.Random, n: int, q: int, s_how: str, pos_how: str, ch_how: str) -> str:
    if s_how == "alla":
        s = "a" * n
    elif s_how == "allc":
        s = "c" * n
    elif s_how == "abc":
        s = "".join("abc"[i % 3] for i in range(n))
    elif s_how == "cba":
        s = "".join("cba"[i % 3] for i in range(n))
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
        c = "b" if ch_how == "b" else rng.choice(CH)
        lines.append(f"{x} {c}")
    return "\n".join(lines) + "\n"


PLANS = [
    (1, 1, "random", "random", "random"),
    (1, 6, "alla", "random", "random"),
    (MAX, MAX, "alla", "random", "random"),
    (MAX, MAX, "allc", "random", "b"),
    (MAX, MAX, "abc", "random", "random"),
    (MAX, MAX, "cba", "random", "random"),
    (MAX, MAX, "random", "random", "random"),
    (MAX, MAX, "random", "first", "random"),
    (MAX, MAX, "random", "last", "random"),
    (MAX, MAX, "random", "seq", "random"),
    (MAX, MAX, "random", "random", "b"),
    (99999, MAX, "random", "random", "random"),
    (65537, MAX, "random", "last", "random"),
    (65536, MAX, "abc", "random", "random"),
    (MAX, MAX, "abc", "seq", "random"),
]
COUNT = len(SAMPLES) + len(PLANS)


def main() -> None:
    seed = int(sys.argv[1])
    if seed >= 1000:
        rng = random.Random(seed)
        n, q = rng.randint(1, 7), rng.randint(1, 7)
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
