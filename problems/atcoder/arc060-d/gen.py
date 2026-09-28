"""arc060-d (Best Representation) の入力を作る。英小文字の文字列 w を 1 行で出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、|w| = 5 × 10^5 前後のいろいろな文字列。
提出は w の最小の周期で 3 通りに分ける。w がよい文字列なら 1 と 1、全部同じ文字なら |w| と 1、
それ以外は 2 と、前と後ろに分けて両方よい文字列になる切れ目の数。3 通りとも大きい文字列を入れる。
周期のある文字列は、ab のくり返し (切れ目は奇数の位置だけで、数は |w| / 2)、aab のくり返し、長さ 7 のランダムな塊のくり返し、
a^999 b のくり返し、Fibonacci 文字列のくり返し、長い塊 2 つ (uu) など。
seed が 1000 以上なら、分け方を全部数える DP の愚直解で解ける |w| = 30 までの入力を出す (pj testdata crosscheck 用)。
2000 以上なら、愚直解でまだ解ける |w| = 400 までの入力を出す。
"""

import random
import sys

MAX_N = 5 * 10**5
ALPHA = "abcdefghijklmnopqrstuvwxyz"

SAMPLES = ["aab", "bcbc", "ddd"]
FIXED = ["a", "aa", "ab", "aba", "abab", "zzzzzy", "abcabcabc"]
PLANS = [
    "random26",
    "random2",
    "same",
    "same_prime",
    "ab",
    "aab",
    "block7",
    "a999b",
    "fibonacci",
    "uu",
    "almost_periodic",
    "one_off",
]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)


def fib_word(n: int) -> str:
    a, b = "a", "ab"
    while len(b) < n:
        a, b = b, b + a
    return b[:n]


def repeat_to(block: str, n: int) -> str:
    """block をくり返して、n 以下で最長の block の倍数の長さにする。"""
    return block * (n // len(block))


def make(plan: str, rng: random.Random) -> str:
    n = MAX_N
    if plan == "random26":
        return "".join(rng.choice(ALPHA) for _ in range(n))
    if plan == "random2":
        return "".join(rng.choice("ab") for _ in range(n))
    if plan == "same":
        return "z" * n
    if plan == "same_prime":
        return "q" * 499979  # 素数の長さ
    if plan == "ab":
        return repeat_to("ab", n)
    if plan == "aab":
        return repeat_to("aab", n)
    if plan == "block7":
        return repeat_to("".join(rng.choice("ab") for _ in range(7)), n)
    if plan == "a999b":
        return repeat_to("a" * 999 + "b", n)
    if plan == "fibonacci":
        return repeat_to(fib_word(1000), n)  # 塊は長さ 1000 の Fibonacci 文字列
    if plan == "uu":
        u = "a" * (n // 2 - 1) + "b"
        return u + u
    if plan == "almost_periodic":
        # ab のくり返しの最後だけ変える。よい文字列なので 1 と 1。
        return repeat_to("ab", n - 2) + "aa"
    assert plan == "one_off"
    # 周期 3 のくり返しの 1 文字だけ変える。
    s = list(repeat_to("abc", n))
    s[rng.randrange(len(s))] = "z"
    return "".join(s)


def small_case(rng: random.Random, n: int) -> str:
    """愚直解用。周期のある文字列を多めに混ぜる。"""
    kind = rng.randrange(5)
    if kind == 0:
        return "".join(rng.choice("ab"[: rng.randint(1, 2)] if rng.random() < 0.5 else "abc") for _ in range(n))
    if kind == 1:
        return rng.choice("ab") * n
    block = "".join(rng.choice("ab") for _ in range(rng.randint(1, max(1, n // 2))))
    s = (block * (n // len(block) + 1))[:n]
    if kind == 2:
        s = block * max(1, n // len(block))  # 周期がちょうど割り切れる長さ
    elif kind == 3 and len(s) >= 2:
        i = rng.randrange(len(s))
        s = s[:i] + rng.choice("abc") + s[i + 1:]
    return s


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 1000:
        w = small_case(rng, rng.randint(31, 400) if seed >= 2000 else rng.randint(1, 30))
    elif seed < len(SAMPLES):
        w = SAMPLES[seed]
    elif seed < len(SAMPLES) + len(FIXED):
        w = FIXED[seed - len(SAMPLES)]
    else:
        w = make(PLANS[seed - len(SAMPLES) - len(FIXED)], rng)
    assert 1 <= len(w) <= MAX_N and set(w) <= set(ALPHA)
    print(w)


if __name__ == "__main__":
    main()
