"""abc223-h (Xor Query) の入力を作る。N Q と A_1 ... A_N、続く Q 行の L R X を出す。

seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース (N = 1)、N = 4 × 10^5、Q = 2 × 10^5 のケース。
60 bit の乱数だけだと、区間が 60 個を超えると全空間を張って答えが Yes ばかりになるので、張る空間の次元を絞った列
(40 本の元の組み合わせ、20 bit 未満の値、同じ値、2 の冪の並び)、短い区間、長さ 1 の区間、全体の区間を混ぜる。
X は半分を区間の元のいくつかの xor (Yes になりやすい)、半分を乱数にする。最上位の bit がどれも 59 の列は、
挿入のたびに要の入れ替えが下まで続きやすい。
seed が 1000 以上なら、区間の元で毎回掃き出す愚直解で解ける小さい入力 (N, Q ≤ 30、値は 6 bit 未満) を出す
(pj testdata crosscheck --brute brute.hpp 用)。
"""

import random
import sys

MAX_N = 4 * 10**5
MAX_Q = 2 * 10**5
LIM = 1 << 60

SAMPLES = [
    "5 2\n3 1 4 1 5\n1 3 7\n2 5 7\n",
    "10 10\n8 45 56 9 38 28 33 5 15 19\n10 10 53\n3 8 60\n1 10 29\n5 7 62\n3 7 51\n8 8 52\n1 4 60\n6 8 32\n4 8 58\n5 9 2\n",
]


def values(rng: random.Random, n: int, how: str, bits: int) -> list[int]:
    lim = 1 << bits
    if how == "lowrank":
        base = [rng.randrange(1, lim) for _ in range(40)]
        out = []
        for _ in range(n):
            v = 0
            while v == 0:
                for b in base:
                    if rng.random() < 0.5:
                        v ^= b
            out.append(v)
        return out
    if how == "same":
        v = rng.randrange(1, lim)
        return [v] * n
    if how == "pow2":
        return [1 << (i % bits) for i in range(n)]
    if how == "top59":
        return [(1 << 59) | rng.randrange(0, 1 << 59) for _ in range(n)]
    return [rng.randrange(1, lim) for _ in range(n)]


def make(rng: random.Random, n: int, q: int, how: str, bits: int, range_how: str) -> str:
    a = values(rng, n, how, bits)
    lim = 1 << bits
    lines = [f"{n} {q}", " ".join(map(str, a))]
    for _ in range(q):
        if range_how == "short":
            ln = rng.randint(1, min(n, 70))
            l = rng.randint(1, n - ln + 1)
            r = l + ln - 1
        elif range_how == "single":
            l = r = rng.randint(1, n)
        elif range_how == "full":
            l, r = 1, n
        else:
            l, r = sorted((rng.randint(1, n), rng.randint(1, n)))
        if rng.random() < 0.5:
            x = 0
            for _ in range(rng.randint(1, 4)):
                x ^= a[rng.randint(l, r) - 1]
            if x == 0:
                x = a[l - 1]
        else:
            x = rng.randrange(1, lim)
        lines.append(f"{l} {r} {x}")
    return "\n".join(lines) + "\n"


PLANS = [
    (1, 1, "random", 60, "random"),
    (1, 5, "random", 3, "random"),
    (MAX_N, MAX_Q, "random", 60, "random"),
    (MAX_N, MAX_Q, "random", 60, "short"),
    (MAX_N, MAX_Q, "lowrank", 60, "random"),
    (MAX_N, MAX_Q, "lowrank", 60, "short"),
    (MAX_N, MAX_Q, "random", 20, "random"),
    (MAX_N, MAX_Q, "same", 60, "random"),
    (MAX_N, MAX_Q, "pow2", 60, "random"),
    (MAX_N, MAX_Q, "random", 60, "single"),
    (MAX_N, MAX_Q, "lowrank", 60, "full"),
    (MAX_N, MAX_Q, "top59", 60, "random"),
    (MAX_N, MAX_Q, "top59", 60, "short"),
    (MAX_N - 1, MAX_Q, "random", 60, "short"),
]
COUNT = len(SAMPLES) + len(PLANS)


def main() -> None:
    seed = int(sys.argv[1])
    if seed >= 1000:
        rng = random.Random(seed)
        n, q = rng.randint(1, 30), rng.randint(1, 30)
        sys.stdout.write(make(rng, n, q, rng.choice(["random", "lowrank", "same", "pow2"]), 6, rng.choice(["random", "short", "single"])))
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
