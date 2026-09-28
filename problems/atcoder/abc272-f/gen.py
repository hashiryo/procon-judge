"""abc272-f (Two Strings) の入力を作る。N と、長さ N の文字列 S と T を出す。

seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース、N = 2 × 10^5 の近くのいろいろな文字列。
文字列は、ランダム、1 文字だけのもの、a を並べて最後だけ b か c にしたもの (回したものが全部違い、
共通の接頭辞が長い)、周期のあるもの、Fibonacci 文字列、Thue-Morse 文字列、2 文字だけのランダムを混ぜる。
T は、S と同じもの、S を回したもの、S を 1 文字だけ変えたものも入れる。全部同じ文字なら答えは N^2 で、
int に収まらない。
seed が 1000 以上なら、回した文字列を全部作って組ごとに比べる愚直解で解ける小さい入力を出す (pj testdata crosscheck 用)。
2000 以上なら、愚直解でまだ解ける N = 400 までの入力を出す。
"""

import random
import sys

MAX_N = 2 * 10**5

SAMPLES = [
    ("adb", "cab"),
    ("wsiuhwijsl", "pwqoketvun"),
]
FIXED = [
    ("a", "a"),
    ("b", "a"),
    ("a", "z"),
    ("ab", "ba"),
    ("aaa", "aaa"),
    ("aab", "aba"),
    ("zzzz", "zzzy"),
]
# (N, S の作り方, T の作り方)。本番のケースのうち角のケースのあとに並べる。T の作り方の same、rotate、
# change は S から作る。
PLANS = [
    (MAX_N, "a", "a"),  # N^2
    (MAX_N, "z", "a"),  # 0
    (MAX_N, "a", "z"),  # N^2
    (MAX_N, "random", "random"),
    (MAX_N, "random", "same"),
    (MAX_N, "one_b", "same"),  # N(N + 1) / 2
    (MAX_N, "one_b", "one_c"),
    (MAX_N, "period_ab", "rotate"),
    (196418, "fibonacci", "rotate"),
    (131072, "thue_morse", "change"),
    (MAX_N, "binary", "binary"),
    (MAX_N, "random", "change"),
    (MAX_N, "period1000", "rotate"),
    (MAX_N - 1, "one_b", "binary"),
]


def make(rng: random.Random, n: int, how: str, base: str = "") -> str:
    if how in ("a", "z"):
        return how * n
    if how == "random":
        return "".join(rng.choice("abcdefghijklmnopqrstuvwxyz") for _ in range(n))
    if how == "binary":
        return "".join(rng.choice("ab") for _ in range(n))
    if how in ("one_b", "one_c"):
        return "a" * (n - 1) + how[-1]
    if how == "period_ab":
        return ("ab" * n)[:n]
    if how.startswith("period"):
        p = int(how[len("period"):])
        unit = "".join(rng.choice("ab") for _ in range(p))
        return (unit * (n // p + 1))[:n]
    if how == "fibonacci":
        a, b = "a", "ab"
        while len(b) < n:
            a, b = b, b + a
        return b[:n]
    if how == "thue_morse":
        return "".join("ab"[bin(i).count("1") % 2] for i in range(n))
    if how == "same":
        return base
    if how == "rotate":
        k = rng.randrange(n)
        return base[k:] + base[:k]
    assert how == "change"
    i = rng.randrange(n)
    return base[:i] + rng.choice([c for c in "abz" if c != base[i]]) + base[i + 1:]


def small_case(rng: random.Random, n: int) -> tuple[str, str]:
    """愚直解が解ける大きさ。文字の種類を少なくして、長い共通の接頭辞を作る。"""
    alphabet = rng.choice(["a", "ab", "abc", "abcdefghijklmnopqrstuvwxyz"])
    s = "".join(rng.choice(alphabet) for _ in range(n))
    how = rng.choice(["random", "same", "rotate", "change", "period"])
    if how == "random":
        t = "".join(rng.choice(alphabet) for _ in range(n))
    elif how == "period":
        divisors = [p for p in range(1, n + 1) if n % p == 0]
        p = rng.choice(divisors)
        s = s[:p] * (n // p)
        t = make(rng, n, "rotate", s)
    else:
        t = make(rng, n, how, s)
    return s, t


def case_for(seed: int, rng: random.Random) -> tuple[str, str]:
    if seed >= 1000:
        return small_case(rng, rng.randint(50, 400) if seed >= 2000 else rng.randint(1, 30))
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    n, how_s, how_t = PLANS[seed - len(FIXED)]
    s = make(rng, n, how_s)
    return s, make(rng, n, how_t, s)


def main() -> None:
    seed = int(sys.argv[1])
    s, t = case_for(seed, random.Random(seed))
    n = len(s)
    assert 1 <= n <= MAX_N and len(t) == n
    assert set(s + t) <= set("abcdefghijklmnopqrstuvwxyz")
    sys.stdout.write(f"{n}\n{s}\n{t}\n")


if __name__ == "__main__":
    main()
