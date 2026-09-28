"""agc047-b (First Second) の入力を作る。N と、互いに違う N 個の文字列を出す。

seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース、長さの和が 10^6 の近くのいろいろな文字列。
短い T が長い S から作れるのは、T から先頭を除いたものが S の末尾で、T の先頭の文字が S の前の方にあるとき。
そういう組が多くなるように、a を 1 個から 1413 個まで並べたもの (どの組も作れる)、1 つの文字列 B の末尾に
いろいろな先頭の文字を付けたもの、2 文字や 3 文字で書ける短い文字列を全部並べたものを入れる。
長い文字列の位置ごとに 26 文字ぶん map を引く提出が遅くなるように、26 文字が早く出そろう長い文字列と、
10^5 個の短い文字列を混ぜたものも入れる。
seed が 1000 以上なら、文字列ごとに操作をそのまま幅優先でたどって作れる文字列を全部求める愚直解で解ける
小さい入力を出す (pj testdata crosscheck 用)。2000 以上なら、愚直解でまだ解ける 300 個までの入力を出す。
"""

import itertools
import random
import string
import sys

MAX_N = 200000
MAX_TOTAL = 10**6
LETTERS = string.ascii_lowercase

SAMPLES = [
    ["abcxyx", "cyx", "abc"],
    ["b", "a", "abc", "c", "d", "ab"],
]
FIXED = [
    ["a", "b"],
    ["ab", "b"],
    ["ab", "a"],
    ["ba", "a"],
    ["a", "aa"],
    ["abc", "ac", "bc", "c", "a", "b", "cc", "ab"],
    list(LETTERS) + [LETTERS],  # 26
    ["z" + "a" * 20, "a", "za", "b" + "a" * 5, "zb" + "a" * 5],
]
# (文字列の作り方, 引数)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    ("a_chain", 1413),  # 長さの和 998991。答えは 1413 * 1412 / 2
    ("suffix_family", 270),  # 26 * 270 個
    ("all_words", "ab", 12),  # 8190 個
    ("all_words", "abc", 8),  # 9840 個
    ("short_random", 5),  # 長さ 5 を 2 * 10^5 個
    ("short_random", 3),  # 長さ 1 から 5 を 2 * 10^5 個
    ("long_and_short", 5),  # 長い 5 個と短い 10^5 個
    ("long_and_short", 1),
    ("one_long", 0),
    ("common_tail", 0),
    ("suffix_family", 60),
]


def a_chain(k: int) -> list[str]:
    return ["a" * i for i in range(1, k + 1)]


def suffix_family(rng: random.Random, length: int) -> list[str]:
    """ランダムな B の末尾 B[j:] に、26 通りの先頭の文字を付けたもの。"""
    b = "".join(rng.choice(LETTERS) for _ in range(length))
    return [c + b[j:] for j in range(1, length + 1) for c in LETTERS]


def all_words(alphabet: str, max_len: int) -> list[str]:
    return ["".join(w) for k in range(1, max_len + 1) for w in itertools.product(alphabet, repeat=k)]


def distinct_random(rng: random.Random, count: int, lengths: tuple[int, int], alphabet: str = LETTERS) -> list[str]:
    seen: set[str] = set()
    while len(seen) < count:
        seen.add("".join(rng.choice(alphabet) for _ in range(rng.randint(*lengths))))
    out = sorted(seen)
    rng.shuffle(out)
    return out


def long_word(rng: random.Random, length: int) -> str:
    """26 文字が先頭の 26 文字で出そろう長い文字列。"""
    head = list(LETTERS)
    rng.shuffle(head)
    return "".join(head) + "".join(rng.choice(LETTERS) for _ in range(length - 26))


def planned(rng: random.Random, plan: tuple) -> list[str]:
    how, arg = plan[0], plan[1]
    if how == "a_chain":
        return a_chain(arg)
    if how == "suffix_family":
        return suffix_family(rng, arg)
    if how == "all_words":
        return all_words(arg, plan[2])
    if how == "short_random":
        return distinct_random(rng, MAX_N, (5, 5) if arg == 5 else (1, 5))
    if how == "long_and_short":
        short = distinct_random(rng, 100000, (1, 4))
        rest = MAX_TOTAL - sum(map(len, short))
        words = short + [long_word(rng, rest // arg) for _ in range(arg)]
        rng.shuffle(words)
        return words
    if how == "one_long":
        # z を含まない長い文字列と、z と a。答えは 1 (a だけが作れる)。
        return ["".join(rng.choice(LETTERS[:-1]) for _ in range(MAX_TOTAL - 2)), "z", "a"]
    assert how == "common_tail"
    # 先頭の 1 文字から 6 文字を 8 種類の文字でいろいろに変え、末尾を共通にする。
    tail = "".join(rng.choice("ab") for _ in range(3))
    heads = distinct_random(rng, 110000, (1, 6), "abcdefgh")
    return [h + tail for h in heads]


def small_case(rng: random.Random, count: int, max_len: int) -> list[str]:
    alphabet = rng.choice(["a", "ab", "ab", "abc", "abc", LETTERS])
    max_len = max(max_len, 2)
    available = sum(len(alphabet) ** k for k in range(1, max_len + 1))
    count = min(count, available)
    return distinct_random(rng, count, (1, max_len), alphabet)


def case_for(seed: int, rng: random.Random) -> list[str]:
    if seed >= 2000:
        return small_case(rng, rng.randint(2, 300), rng.randint(3, 30))
    if seed >= 1000:
        return small_case(rng, rng.randint(2, 15), rng.randint(1, 7))
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    return planned(rng, PLANS[seed - len(FIXED)])


def main() -> None:
    seed = int(sys.argv[1])
    words = case_for(seed, random.Random(seed))
    n = len(words)
    assert 2 <= n <= MAX_N and len(set(words)) == n
    assert all(len(w) >= 1 and set(w) <= set(LETTERS) for w in words) and sum(map(len, words)) <= MAX_TOTAL
    sys.stdout.write(f"{n}\n" + "\n".join(words) + "\n")


if __name__ == "__main__":
    main()
