"""abc213-f (Common Prefixes) の入力を作る。N と S を出す。

seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース、N = 10^6 のランダム、
接尾辞どうしの LCP が長くなる形 (1 文字だけ、1 か所だけ違う文字、周期的な文字列、
フィボナッチ文字列、Thue-Morse 列)。出力が N 行あって大きいので、N = 10^6 は 2 ケースだけにする。
1 文字だけの N = 2 × 10^5 では答えが 2^32 を超える。
seed が 1000 以上なら、O(N^2) の愚直解で解ける小さい入力を出す (pj testdata crosscheck 用)。
"""

import random
import string
import sys

MAX_N = 10**6
LETTERS = string.ascii_lowercase

SAMPLES = ["abb", "mississippi"]
FIXED = [
    "a",
    "z",
    "aa",
    "ab",
    "ba",
    LETTERS[::-1],
    "a" * 9 + "b",
    "b" + "a" * 9,
    "ab" * 5 + "a",
]
# (N, 形)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    (MAX_N, "random26"),
    (MAX_N, "random2"),
    (2 * 10**5, "fibonacci"),
    (2 * 10**5, "same"),
    (2 * 10**5, "same_but_one"),
    (2 * 10**5, "period_short"),
    (10**5, "period_long"),
    (10**5, "thue_morse"),
    (10**5, "random3"),
    (1000, "random2"),
]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)


def fibonacci(n: int) -> str:
    x, y = "a", "ab"
    while len(y) < n:
        x, y = y, y + x
    return y[:n]


def thue_morse(n: int) -> str:
    return "".join("ab"[i.bit_count() & 1] for i in range(n))


def periodic(rng: random.Random, n: int, period: int, letters: str) -> str:
    block = "".join(rng.choices(letters, k=period))
    return (block * (n // period + 1))[:n]


def make(rng: random.Random, n: int, shape: str) -> str:
    if shape.startswith("random"):
        return "".join(rng.choices(LETTERS[: int(shape[len("random"):])], k=n))
    if shape == "fibonacci":
        return fibonacci(n)
    if shape == "thue_morse":
        return thue_morse(n)
    if shape == "same":
        return rng.choice(LETTERS) * n
    if shape == "same_but_one":
        # 1 文字だけの列に 1 か所だけ別の文字を入れる。LCP がそこで切れる。
        s = ["a"] * n
        s[rng.randrange(n)] = "b"
        return "".join(s)
    if shape == "period_short":
        return periodic(rng, n, rng.randint(2, 8), LETTERS[:3])
    assert shape == "period_long"
    return periodic(rng, n, rng.randint(500, 2000), LETTERS)


def small_case(rng: random.Random) -> str:
    """愚直解が O(N^2) で解ける大きさ。形は本番と同じものから選ぶ。"""
    n = rng.randint(1, 12) if rng.random() < 0.3 else rng.randint(1, 300)
    shape = rng.choice([
        "random1", "random2", "random3", "random26",
        "fibonacci", "thue_morse", "same", "same_but_one", "period_short",
    ])
    return make(rng, n, shape)


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 1000:
        s = small_case(rng)
    elif seed < len(SAMPLES):
        s = SAMPLES[seed]
    elif seed < len(SAMPLES) + len(FIXED):
        s = FIXED[seed - len(SAMPLES)]
    else:
        n, shape = PLANS[seed - len(SAMPLES) - len(FIXED)]
        s = make(rng, n, shape)
    assert 1 <= len(s) <= MAX_N and set(s) <= set(LETTERS)
    sys.stdout.write(f"{len(s)}\n{s}\n")


if __name__ == "__main__":
    main()
