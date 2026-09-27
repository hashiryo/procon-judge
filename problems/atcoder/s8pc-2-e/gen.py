"""s8pc-2-e (部分文字列) の入力を作る。英小文字の文字列 S を 1 行に出す。答えは、異なる部分文字列の長さの和。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、|S| = 10^5 のいろいろな形の文字列。
形は、ランダム (26 文字、2 文字)、全部同じ文字 (答えは N(N+1)/2)、周期の短い繰り返し、長い塊の繰り返し、
フィボナッチ文字列、Thue-Morse 列、半分ずつ違う文字、回文。繰り返しの多い文字列では、SA-IS の再帰が
深くなり、LCP も長くなる。
seed が 1000 以上なら、始まる位置ごとに前の位置との LCP を求める O(N^2) の愚直解で解ける短い文字列を出す
(pj testdata crosscheck 用)。2000 以上なら、愚直解でまだ解ける 3000 文字までの文字列を出す。
"""

import random
import string
import sys

MAX_N = 10**5
LETTERS = string.ascii_lowercase

FIXED = [
    "abc",  # 例 1
    "aaqqz",  # 例 2
    "atcoder",  # 例 3
    "a",
    "z",
    "aa",
    "ab",
    "aba",
    LETTERS,
    "a" * MAX_N,
]
# 本番のケースのうち角のケースのあとに並べる、|S| = 10^5 の形。
SHAPES = [
    "random26",
    "random2",
    "random3",
    "period2",
    "alphabet",
    "block",
    "fibonacci",
    "thue_morse",
    "halves",
    "palindrome",
    "almost_same",
]
COUNT = len(FIXED) + len(SHAPES)


def fibonacci(n: int) -> str:
    a, b = "a", "ab"
    while len(b) < n:
        a, b = b, b + a
    return b[:n]


def thue_morse(n: int) -> str:
    return "".join("ab"[bin(i).count("1") % 2] for i in range(n))


def make(rng: random.Random, shape: str, n: int) -> str:
    if shape == "random26":
        return "".join(rng.choice(LETTERS) for _ in range(n))
    if shape == "random2":
        return "".join(rng.choice("ab") for _ in range(n))
    if shape == "random3":
        return "".join(rng.choice("xyz") for _ in range(n))
    if shape == "period2":
        return ("ab" * n)[:n]
    if shape == "alphabet":
        return (LETTERS * (n // 26 + 1))[:n]
    if shape == "block":
        # ランダムな 1000 文字をくり返し、ところどころ 1 文字変える。
        base = "".join(rng.choice(LETTERS) for _ in range(min(n, 1000)))
        s = list((base * (n // len(base) + 1))[:n])
        for _ in range(5):
            s[rng.randrange(n)] = rng.choice(LETTERS)
        return "".join(s)
    if shape == "fibonacci":
        return fibonacci(n)
    if shape == "thue_morse":
        return thue_morse(n)
    if shape == "halves":
        return "a" * (n // 2) + "b" * (n - n // 2)
    if shape == "palindrome":
        half = "".join(rng.choice("abc") for _ in range(n // 2))
        return (half + rng.choice(LETTERS) + half[::-1])[:n] if n % 2 else half + half[::-1]
    assert shape == "almost_same"
    s = ["a"] * n
    for _ in range(3):
        s[rng.randrange(n)] = "b"
    return "".join(s)


def case_for(seed: int, rng: random.Random) -> str:
    if seed >= 1000:
        n = rng.randint(101, 3000) if seed >= 2000 else rng.randint(1, 100)
        return make(rng, rng.choice(sorted(set(SHAPES))), n)
    if seed < len(FIXED):
        return FIXED[seed]
    return make(rng, SHAPES[seed - len(FIXED)], MAX_N)


def main() -> None:
    seed = int(sys.argv[1])
    s = case_for(seed, random.Random(seed))
    assert 1 <= len(s) <= MAX_N and set(s) <= set(LETTERS)
    print(s)


if __name__ == "__main__":
    main()
