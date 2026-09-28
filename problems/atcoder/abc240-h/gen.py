"""abc240-h (Sequence of Substrings) の入力を作る。N と 0 と 1 の文字列 S を出す。

seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース、N = 25000 を中心にいろいろな文字列。
提出は長さが B 以下の部分文字列だけを見る (B は B(B+1)/2 <= N となる最大)。全部 0 の文字列の答えはちょうど B なので、
B(B+1)/2 = N となる N とその 1 つ手前も入れる。文字列は、全部同じ文字、0 の塊と 1 の塊、01 のくり返し、
長さ 11 の文字列を辞書順に全部つないだもの (答えが 2048 を超える)、長さ 11 までの文字列を辞書順につないだもの、
Thue-Morse 列、Fibonacci 文字列、ランダム (1 の割合を変える)、ランダムな塊のくり返し。
seed が 1000 以上なら、部分文字列の組を全部比べる愚直解で解ける N = 30 までの入力を出す (pj testdata crosscheck 用)。
2000 以上なら、愚直解でまだ解ける N = 80 までの入力を出す。
"""

import itertools
import random
import sys

MAX_N = 25000

SAMPLES = ["0101010", "000011001110101001011110001001"]
FIXED = ["0", "1", "10", "01", "000"]
# N = 25000 (や B の境目) の文字列の作り方。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    ("zeros", MAX_N),
    ("ones", MAX_N),
    ("zeros", 223 * 224 // 2),  # B = 223 になる最小の N。答えは 223
    ("zeros", 223 * 224 // 2 - 1),  # B = 222。答えは 222
    ("zeros_then_ones", MAX_N),
    ("ones_then_zeros", 5000),
    ("alternate", MAX_N),
    ("all_length_11", MAX_N),
    ("sorted_short", MAX_N),
    ("thue_morse", MAX_N),
    ("fibonacci", MAX_N),
    ("random_half", MAX_N),
    ("random_sparse", 10000),
    ("random_dense", 10000),
    ("block_7", 5000),
    ("block_100", 10000),
    ("random_runs", MAX_N),
    ("random_half", 1000),
]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)


def make(plan: str, n: int, rng: random.Random) -> str:
    if plan == "zeros":
        return "0" * n
    if plan == "ones":
        return "1" * n
    if plan == "zeros_then_ones":
        return "0" * (n // 2) + "1" * (n - n // 2)
    if plan == "ones_then_zeros":
        return "1" * (n // 2) + "0" * (n - n // 2)
    if plan == "alternate":
        return ("01" * n)[:n]
    if plan == "all_length_11":
        # 長さ 11 の 2048 個を辞書順に並べてつなぎ、残りはランダム。
        s = "".join(format(i, "011b") for i in range(2048))
        return s + "".join(rng.choice("01") for _ in range(n - len(s)))
    if plan == "sorted_short":
        words = sorted("".join(w) for k in range(1, 12) for w in itertools.product("01", repeat=k))
        return "".join(words)[:n]
    if plan == "thue_morse":
        return "".join(str(bin(i).count("1") % 2) for i in range(n))
    if plan == "fibonacci":
        a, b = "0", "01"
        while len(b) < n:
            a, b = b, b + a
        return b[:n]
    if plan == "random_half":
        return "".join(rng.choice("01") for _ in range(n))
    if plan == "random_sparse":
        return "".join("1" if rng.random() < 0.05 else "0" for _ in range(n))
    if plan == "random_dense":
        return "".join("0" if rng.random() < 0.05 else "1" for _ in range(n))
    if plan in ("block_7", "block_100"):
        block = "".join(rng.choice("01") for _ in range(int(plan.split("_")[1])))
        return (block * (n // len(block) + 1))[:n]
    assert plan == "random_runs"
    out: list[str] = []
    while len(out) < n:
        out.extend(rng.choice("01") * rng.randint(1, 300))
    return "".join(out[:n])


def small_case(rng: random.Random, n: int) -> str:
    kind = rng.randrange(5)
    if kind == 0:
        return make("random_half", n, rng)
    if kind == 1:
        p = rng.choice([0.1, 0.3, 0.7, 0.9])
        return "".join("1" if rng.random() < p else "0" for _ in range(n))
    if kind == 2:
        block = "".join(rng.choice("01") for _ in range(rng.randint(1, 4)))
        return (block * n)[:n]
    if kind == 3:
        return make(rng.choice(["zeros", "ones", "zeros_then_ones", "ones_then_zeros", "thue_morse", "fibonacci"]), n, rng)
    out: list[str] = []
    while len(out) < n:
        out.extend(rng.choice("01") * rng.randint(1, 6))
    return "".join(out[:n])


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 1000:
        s = small_case(rng, rng.randint(31, 80) if seed >= 2000 else rng.randint(1, 30))
    elif seed < len(SAMPLES):
        s = SAMPLES[seed]
    elif seed < len(SAMPLES) + len(FIXED):
        s = FIXED[seed - len(SAMPLES)]
    else:
        plan, n = PLANS[seed - len(SAMPLES) - len(FIXED)]
        s = make(plan, n, rng)
    assert 1 <= len(s) <= MAX_N and set(s) <= set("01")
    print(len(s))
    print(s)


if __name__ == "__main__":
    main()
