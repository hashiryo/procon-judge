"""abc268-h (Taboo) の入力を作る。S と N と T_1 ... T_N を出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、|S| = Σ|T_i| = 5 × 10^5 前後のもの。
角のケースは、|S| = 1、T が S より長いもの、T = S、S = a...a で T = a や aa (答えは |S| と |S| / 2)。
大きいものは、2 文字の文字列を全部 T にしたもの (答えは |S| / 2)、短い T をたくさん (N = 10^5 前後)、
S の部分文字列を T にしたもの、フィボナッチ文字列、周期的な S、長い T、どれとも一致しないもの。
AhoCorasick は各頂点に一致する T の番号の列を持ち、fail の先の列を写すので、a, aa, ..., a^577 と
xy a^577 の形を 575 個並べると、列の長さの合計が 10^8 近くになる (手元で 450 MB ほど使う)。
fail を長くたどらせる形 (a^299 c のくり返しと a^300 b) も入れる。
seed が 1000 以上なら、出現を区間として全部探して * の位置を DP で決める愚直解で解ける小さい入力を出す
(pj testdata crosscheck 用)。2000 以上なら、|S| と Σ|T_i| が 2000 までの入力を出す。
"""

import itertools
import random
import string
import sys

MAX_S = 5 * 10**5
MAX_SUM = 5 * 10**5

Case = tuple[str, list[str]]

SAMPLES = [
    ("abcdefghijklmn", ["abcd", "ijk", "ghi"]),
    ("atcoderbeginnercontest", ["abc"]),
    ("aaaaaaaaa", ["aa", "xyz"]),
]
# 本番のケースのうち例のあとに並べるもの。
PLANS = [
    "min",  # S = a、T = a
    "min_miss",  # S = a、T = b
    "longer",  # T が S より長い
    "equal",  # T = S
    "unary_single",  # a...a と a。答えは |S|
    "unary_pair",  # a...a と aa。答えは |S| / 2
    "unary_lengths",  # a...a と a^k をいくつか。答えは |S| / (最短の k)
    "whole",  # T = S (5 × 10^5 文字)
    "all_pairs",  # 26^2 個の 2 文字を全部。答えは |S| / 2
    "many_short",
    "max_n",  # 10 文字の 4 文字列を全部と、26 文字の 5 文字列
    "blowup",
    "fail_heavy",
    "fibonacci",
    "substrings",
    "long_patterns",
    "periodic",
    "binary_random",
    "no_match",
    "random_mid",
    "random_mid",
]
COUNT = len(SAMPLES) + len(PLANS)


def random_string(rng: random.Random, n: int, alphabet: str) -> str:
    return "".join(rng.choice(alphabet) for _ in range(n))


def distinct(strings: list[str]) -> list[str]:
    """最初に出た順で重複を除く。"""
    return list(dict.fromkeys(strings))


def fill(make, budget: int) -> list[str]:
    """make() で作った文字列を、長さの合計が budget を超えない所まで重複なしで集める。
    重複か入らない長さが 1000 回続いたらやめる。
    """
    out, seen, total, misses = [], set(), 0, 0
    while misses < 1000:
        t = make()
        if t in seen or total + len(t) > budget:
            misses += 1
            continue
        seen.add(t)
        out.append(t)
        total += len(t)
        misses = 0
    return out


def substrings(rng: random.Random, s: str, lo: int, hi: int, budget: int) -> list[str]:
    def make() -> str:
        k = rng.randint(lo, hi)
        i = rng.randint(0, len(s) - k)
        return s[i : i + k]

    return fill(make, budget)


def fibonacci(n: int) -> str:
    a, b = "a", "ab"
    while len(b) < n:
        a, b = b, b + a
    return b[:n]


def plan_case(rng: random.Random, plan: str) -> Case:
    if plan == "min":
        return "a", ["a"]
    if plan == "min_miss":
        return "a", ["b"]
    if plan == "longer":
        return "ab", ["abc"]
    if plan == "equal":
        return "abcab", ["abcab"]
    if plan == "unary_single":
        return "a" * MAX_S, ["a"]
    if plan == "unary_pair":
        return "a" * MAX_S, ["aa"]
    if plan == "unary_lengths":
        return "a" * MAX_S, ["a" * k for k in (997, 7, 50000, 13, 100000)]
    if plan == "whole":
        s = random_string(rng, MAX_S, "ab")
        return s, [s]
    if plan == "all_pairs":
        return random_string(rng, MAX_S, string.ascii_lowercase), ["".join(p) for p in itertools.product(string.ascii_lowercase, repeat=2)]
    if plan == "many_short":
        t = fill(lambda: random_string(rng, rng.randint(5, 8), "abcdefgh"), MAX_SUM)
        return random_string(rng, MAX_S, "abcdefgh"), t
    if plan == "max_n":
        four = ["".join(p) for p in itertools.product("abcdefghij", repeat=4)]
        five = fill(lambda: random_string(rng, 5, string.ascii_lowercase), MAX_SUM - 4 * len(four))
        t = distinct(four + five)
        rng.shuffle(t)
        return random_string(rng, MAX_S, string.ascii_lowercase), t
    if plan == "blowup":
        # a^2, ..., a^k と、a を含まない 2 文字 w ごとの w a^k。
        k = 577
        t = ["a" * j for j in range(2, k + 1)]
        prefixes = ["".join(p) for p in itertools.product(string.ascii_lowercase[1:], repeat=2)]
        total = sum(map(len, t))
        for w in prefixes:
            if total + len(w) + k > MAX_SUM:
                break
            t.append(w + "a" * k)
            total += len(w) + k
        parts, length = [], 0
        while length < MAX_S:
            part = rng.choice(prefixes) + "a" * rng.choice([rng.randint(1, 20), rng.randint(1, k + 5)])
            parts.append(part)
            length += len(part)
        return "".join(parts)[:MAX_S], t
    if plan == "fail_heavy":
        parts, length = [], 0
        while length < MAX_S:
            part = "a" * 300 + "b" if rng.random() < 0.1 else "a" * 299 + "c"
            parts.append(part)
            length += len(part)
        return "".join(parts)[:MAX_S], ["a" * 300 + "b", "a" * 150 + "d", "cb"]
    if plan == "fibonacci":
        s = fibonacci(MAX_S)
        return s, substrings(rng, s, 2, 3000, 60000)
    if plan == "substrings":
        s = random_string(rng, MAX_S, "ab")
        return s, substrings(rng, s, 8, 40, MAX_SUM)
    if plan == "long_patterns":
        s = random_string(rng, MAX_S, "abc")
        t = [s[i : i + 90000] for i in range(0, MAX_S - 90000, 100000)]
        return s, distinct(t + substrings(rng, s, 30, 60, 50000))
    if plan == "periodic":
        s = ("abcab" * (MAX_S // 5 + 1))[:MAX_S]
        return s, ["cabab" * 3, "bcabca", "abcababca"[:7], "b" + "cabab" * 40, "aa"]
    if plan == "binary_random":
        t = fill(lambda: random_string(rng, rng.randint(12, 20), "ab"), 20000)
        return random_string(rng, MAX_S, "ab"), t
    if plan == "no_match":
        t = fill(lambda: random_string(rng, rng.randint(1, 30), "xyz"), MAX_SUM)
        return random_string(rng, MAX_S, "abc"), t
    assert plan == "random_mid"
    # 短い T を入れると、どの位置にも一致して答えが |S| に張り付くので、5 文字以上にする。
    alphabet = string.ascii_lowercase[: rng.randint(4, 6)]
    s = random_string(rng, rng.randint(MAX_S // 10, MAX_S), alphabet)
    budget = rng.randint(MAX_SUM // 10, MAX_SUM)
    if rng.random() < 0.5:
        return s, substrings(rng, s, 5, 30, budget)
    return s, fill(lambda: random_string(rng, rng.randint(5, 12), alphabet), budget)


def small_case(rng: random.Random, max_s: int, max_sum: int) -> Case:
    alphabet = rng.choice(["a", "ab", "ab", "abc", "abcd"])
    s = random_string(rng, rng.randint(1, max_s), alphabet)
    budget = rng.randint(1, max_sum)
    longest = rng.choice([2, 3, 5, max(1, max_s // 4)])
    if rng.random() < 0.5:
        t = substrings(rng, s, 1, min(len(s), longest), budget)
    else:
        t = fill(lambda: random_string(rng, rng.randint(1, longest), alphabet), budget)
    return s, t or [random_string(rng, 1, alphabet)]


def case_for(seed: int, rng: random.Random) -> Case:
    if seed >= 2000:
        return small_case(rng, 2000, 2000)
    if seed >= 1000:
        return small_case(rng, 30, 30)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    return plan_case(rng, PLANS[seed - len(SAMPLES)])


def main() -> None:
    seed = int(sys.argv[1])
    s, t = case_for(seed, random.Random(seed))
    assert 1 <= len(s) <= MAX_S and 1 <= len(t) and sum(map(len, t)) <= MAX_SUM
    assert all(len(x) >= 1 for x in t) and len(set(t)) == len(t)
    assert set(s) <= set(string.ascii_lowercase) and all(set(x) <= set(string.ascii_lowercase) for x in t)
    out = [s, str(len(t))] + t
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
