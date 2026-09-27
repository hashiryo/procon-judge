"""abc135-d (Digits Parade) の入力を作る。数字と ? からなる S を 1 行で出す。

seed が 0 から count - 1 までは本番のケース。例の 4 つ、角のケース、|S| = 10^5 のいろいろな形、
|S| が 5000 までのランダム。
提出は S の位置と 13 で割った余りの組を状態にしたオートマトンの上で数えるので、全部 ? のときに
状態が最も多くなる (位置ごとに 13 個)。? が無いもの (答えは 0 か 1)、? が端に寄ったもの、
0 だけのものも入れる。
seed が 1000 以上なら、? に入る数字を全部試す愚直解で解ける、? が 6 個までの短い S を出す
(pj testdata crosscheck 用)。
"""

import random
import sys

MAX_LEN = 10**5
MOD13 = 13

SAMPLES = [
    "??2??5",
    "?44",
    "7?4",
    "?6?42???8??2??06243????9??3???7258??5??7???????774????4?1??17???9?5?70???76???",
]
FIXED = [
    "5",
    "?",
    "0",
    "18",  # 18 ≡ 5
    "??",
    "0" * 17 + "5",
    "?????",
    "9" * 17 + "?",
]
# |S| = 10^5 の中身の作り方。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    "all_q",  # 全部 ?
    "random",  # ? と数字が半々
    "sparse_q",  # ? が 1% ほど
    "dense_q",  # 数字が 1% ほど
    "fixed_hit",  # ? が無く、13 で割って 5 余る数
    "fixed_miss",  # ? が無く、5 余らない数
    "zeros",  # 0 だけ (答えは 0)
    "zeros_5",  # 0 が並んで最後が 5 (答えは 1)
    "q_head",  # 前半が ?、後半が数字
    "q_tail",  # 前半が数字、後半が ?
]
MEDIUM = 3  # |S| が 100 から 5000 のランダム
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS) + MEDIUM


def digits(rng: random.Random, n: int) -> list[str]:
    return [str(rng.randrange(10)) for _ in range(n)]


def residue(s: list[str]) -> int:
    r = 0
    for c in s:
        r = (r * 10 + int(c)) % MOD13
    return r


def fixed_number(rng: random.Random, n: int, hit: bool) -> list[str]:
    """? の無い n 桁。hit なら 13 で割って 5 余るように、そうでなければ余りが 5 でないように、下 2 桁を決める。"""
    s = digits(rng, n - 2)
    head = residue(s) * 100 % MOD13
    tails = [t for t in range(100) if ((head + t) % MOD13 == 5) == hit]
    return s + list(f"{rng.choice(tails):02d}")


def plan_case(rng: random.Random, plan: str, n: int = MAX_LEN) -> str:
    if plan == "all_q":
        s = ["?"] * n
    elif plan == "random":
        s = [c if rng.random() < 0.5 else "?" for c in digits(rng, n)]
    elif plan == "sparse_q":
        s = [c if rng.random() < 0.99 else "?" for c in digits(rng, n)]
    elif plan == "dense_q":
        s = [c if rng.random() < 0.01 else "?" for c in digits(rng, n)]
    elif plan in ("fixed_hit", "fixed_miss"):
        s = fixed_number(rng, n, plan == "fixed_hit")
    elif plan == "zeros":
        s = ["0"] * n
    elif plan == "zeros_5":
        s = ["0"] * (n - 1) + ["5"]
    elif plan == "q_head":
        s = ["?"] * (n // 2) + digits(rng, n - n // 2)
    else:
        assert plan == "q_tail"
        s = digits(rng, n // 2) + ["?"] * (n - n // 2)
    return "".join(s)


def small_case(rng: random.Random) -> str:
    """愚直解が ? を全部試せる大きさ。長さは 18 まで、? は 6 個まで。"""
    n = rng.randint(1, 18)
    q = rng.randint(0, min(n, 6))
    pos = set(rng.sample(range(n), q))
    pool = rng.choice(["0123456789", "05", "0", "9", "59"])
    return "".join("?" if i in pos else rng.choice(pool) for i in range(n))


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 1000:
        s = small_case(rng)
    elif seed < len(SAMPLES):
        s = SAMPLES[seed]
    elif seed < len(SAMPLES) + len(FIXED):
        s = FIXED[seed - len(SAMPLES)]
    elif seed < COUNT - MEDIUM:
        s = plan_case(rng, PLANS[seed - len(SAMPLES) - len(FIXED)])
    else:
        s = plan_case(rng, rng.choice(["random", "sparse_q", "dense_q"]), rng.randint(100, 5000))
    assert 1 <= len(s) <= MAX_LEN and set(s) <= set("0123456789?")
    print(s)


if __name__ == "__main__":
    main()
