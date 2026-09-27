"""abc135-f (Strings of Eternity) の入力を作る。s と t を 1 行ずつ出す。

s をくり返した無限の文字列で、位置 i から t が現れるなら i から i + |t| (mod |s|) へ辺を張ると、答えは最も長い道の
辺の数で、輪があれば -1 になる。seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、
|s| や |t| が 5 × 10^5 のケース。角のケースは、1 文字どうし、回転 (ab と ba)、t が s より長いもの、答えが 0 と 1。
大きいケースは、a の並びの最後だけ b にした s と t = a (答え |s| - 1 で、lib.cpp の再帰が 5 × 10^5 段になる)、
全部 a で |s| と |t| が互いに素 (長さ |s| の輪で -1)、t = a a a、|s| = 1、t のくり返しとその回転 (-1)、t の
くり返しの 1 文字だけ変えたもの、ab の並びを c で区切ったもの、ランダム (2 文字と 26 文字)、フィボナッチ文字列、
t が s のくり返しの先頭の切れ端で |s| の倍数でないもの (答え 1) と倍数のもの (-1)。
seed が 1000 以上なら、始まりの位置ごとに t を何回つなげられるかを 1 文字ずつ比べて伸ばす愚直解で解ける短い入力を
出す (pj testdata crosscheck 用)。2000 以上なら、愚直解でまだ解ける 2000 文字までの入力を出す。
"""

import random
import string
import sys

MAX_LEN = 5 * 10**5
N = MAX_LEN


def rand(rng: random.Random, n: int, k: int = 26) -> str:
    return "".join(rng.choice(string.ascii_lowercase[:k]) for _ in range(n))


def fibonacci(n: int) -> str:
    a, b = "a", "ab"
    while len(b) < n:
        a, b = b, b + a
    return b[:n]


def mutate(rng: random.Random, s: str) -> str:
    i = rng.randrange(len(s))
    return s[:i] + ("b" if s[i] == "a" else "a") + s[i + 1:]


def power_of(u: str, n: int) -> str:
    """u をくり返した長さ n の文字列。"""
    return (u * (n // len(u) + 1))[:n]


def structured(rng: random.Random, n: int, m: int) -> tuple[str, str]:
    """短い u のくり返しから s と t を作る。回転や 1 文字の変更を混ぜて、-1、0、大きい答えが出るようにする。"""
    u = rand(rng, rng.randint(1, 4), rng.choice([1, 2, 3]))
    s = power_of(u, max(1, n // len(u)) * len(u) if rng.random() < 0.6 else n)
    r = rng.randrange(len(s))
    s = s[r:] + s[:r]
    t = power_of(u, max(1, m // len(u)) * len(u) if rng.random() < 0.6 else m)
    if rng.random() < 0.3:
        s = mutate(rng, s)
    if rng.random() < 0.2:
        t = mutate(rng, t)
    return s, t


SAMPLES = [("abcabab", "ab"), ("aa", "aaaaaaa"), ("aba", "baaab")]
FIXED = [
    ("a", "a"),  # -1
    ("a", "b"),  # 0
    ("ab", "ba"),  # 回転なので -1
    ("abc", "cab"),
    ("ab", "aba"),  # 1
    ("aab", "a"),  # 2
    ("abab", "ab"),  # -1
    ("ab" * 7 + "c", "ab"),  # 7
    ("b" + "a" * 9, "a" * 9),  # 1
    ("b", "a"),
]


def big_case(rng: random.Random, kind: str) -> tuple[str, str]:
    if kind == "chain":  # 答え N - 1。位置 0, 1, ..., N - 2 が 1 本の道になる
        return "a" * (N - 1) + "b", "a"
    if kind == "all_a_coprime":  # 長さ N の輪 1 つで -1
        return "a" * N, "a" * (N - 1)
    if kind == "all_a_long_t":
        return "a" * 3, "a" * N
    if kind == "aaa":  # 答え (N - 1) // 3
        return "a" * (N - 1) + "b", "aaa"
    if kind == "single_s":
        return "a", "a" * N
    if kind == "single_s_zero":
        return "b", "a" * N
    if kind == "power":  # s = t^500 で -1
        t = rand(rng, 1000)
        return t * 500, t
    if kind == "rotated_power":
        t = rand(rng, 1000)
        s = t * 500
        r = rng.randrange(1, 1000)
        return s[r:] + s[:r], t
    if kind == "power_mutated":  # 変えた所をまたげないので 499
        t = rand(rng, 1000, 2)
        return mutate(rng, t * 500), t
    if kind == "ab_blocks":  # ab が 1000 回続いて c で区切られる。答え 1000
        return ("ab" * 1000 + "c") * (N // 2001), "ab"
    if kind == "random2":
        return rand(rng, N, 2), rand(rng, 3, 2)
    if kind == "random26":
        s = rand(rng, N)
        i = rng.randrange(N - 10)
        return s, s[i:i + 10]
    if kind == "fibonacci":
        return fibonacci(N), fibonacci(233)
    if kind == "prefix_not_multiple":  # t は s^∞ の先頭の切れ端。|t| が |s| の倍数でないので答え 1
        s = rand(rng, 999, 2)
        return s, power_of(s, N)
    if kind == "prefix_multiple":  # |t| が |s| の倍数なので -1
        s = rand(rng, 1000, 2)
        return s, power_of(s, N)
    assert kind == "random_structured"
    return structured(rng, N, rng.randint(1, N))


BIG = ["chain", "all_a_coprime", "all_a_long_t", "aaa", "single_s", "single_s_zero", "power", "rotated_power",
       "power_mutated", "ab_blocks", "random2", "random26", "fibonacci", "prefix_not_multiple", "prefix_multiple",
       "random_structured"]
COUNT = len(SAMPLES) + len(FIXED) + len(BIG)


def case_for(seed: int, rng: random.Random) -> tuple[str, str]:
    if seed >= 1000:
        top = 2000 if seed >= 2000 else 12
        n = rng.randint(1, top)
        # t が s よりずっと短いと、1 文字変えた s の中で t が長く続き、大きい答えが出る。
        m = rng.randint(1, top) if rng.random() < 0.5 else rng.randint(1, max(1, n // 8))
        if rng.random() < 0.3:
            return rand(rng, n, rng.choice([1, 2, 3])), rand(rng, m, rng.choice([1, 2, 3]))
        return structured(rng, n, m)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    return big_case(rng, BIG[seed - len(FIXED)])


def main() -> None:
    seed = int(sys.argv[1])
    s, t = case_for(seed, random.Random(seed))
    assert 1 <= len(s) <= MAX_LEN and 1 <= len(t) <= MAX_LEN
    assert set(s) <= set(string.ascii_lowercase) and set(t) <= set(string.ascii_lowercase)
    sys.stdout.write(f"{s}\n{t}\n")


if __name__ == "__main__":
    main()
