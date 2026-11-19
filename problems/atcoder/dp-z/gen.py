#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""atcoder-dp-z (Frog 3) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

2 ≤ N ≤ 2 × 10^5、1 ≤ C ≤ 10^12、1 ≤ h_1 < h_2 < ... < h_N ≤ 10^6 (問題文の約束)。seed 0 から 2 は問題文の例。seed 3 から
11 は N = 2 × 10^5 で、h の選び方と C は次のとおり。1 から N の連続 (C = 1、C = 10^12)、10^6 以下からの乱択 (C は乱択、C = 1、
C = 10^12)、前半を密に後半を疎に、等間隔 (間隔 5)、10^6 - N + 1 から 10^6 の連続、10^6 以下の乱択で C = 10^6。seed 1000
以上は、愚直解 (brute.hpp) と突き合わせる小さい入力 (N ≤ 8)。
"""
import random
import sys

SAMPLES = [
    "5 6\n1 2 3 4 5",
    "2 1000000000000\n500000 1000000",
    "8 5\n1 3 4 5 10 11 12 13",
]
N = 200000
H = 10**6


def out(c: int, h: list[int]) -> str:
    assert 2 <= len(h) <= N and 1 <= c <= 10**12
    assert all(1 <= a < b <= H for a, b in zip(h, h[1:]))
    return f"{len(h)} {c}\n{' '.join(map(str, h))}"


def main_case(seed: int) -> str:
    if seed < 3:
        return SAMPLES[seed]
    rng = random.Random(seed * 7919 + 101)
    if seed == 3:
        return out(1, list(range(1, N + 1)))
    if seed == 4:
        return out(10**12, list(range(1, N + 1)))
    if seed == 5:
        return out(rng.randint(1, 10**12), sorted(rng.sample(range(1, H + 1), N)))
    if seed == 6:
        return out(1, sorted(rng.sample(range(1, H + 1), N)))
    if seed == 7:
        return out(10**12, sorted(rng.sample(range(1, H + 1), N)))
    if seed == 8:
        dense = list(range(1, N // 2 + 1))
        sparse = sorted(rng.sample(range(N // 2 + 1, H + 1), N - N // 2))
        return out(rng.randint(1, 10**6), dense + sparse)
    if seed == 9:
        return out(rng.randint(1, 10**9), [1 + 5 * i for i in range(N)])
    if seed == 10:
        return out(rng.randint(1, 10**12), list(range(H - N + 1, H + 1)))
    if seed == 11:
        return out(10**6, sorted(rng.sample(range(1, H + 1), N)))
    raise SystemExit(f"seed {seed} は 0 から 11 か、1000 以上にしてください")


def small_case(seed: int) -> str:
    rng = random.Random(seed * 104729 + 3)
    n = rng.randint(2, 8)
    hi = rng.choice((10, 100, H))
    h = sorted(rng.sample(range(1, hi + 1), n))
    c = rng.choice((1, rng.randint(1, 100), rng.randint(1, 10**12)))
    return out(c, h)


def main() -> None:
    seed = int(sys.argv[1])
    print(small_case(seed) if seed >= 1000 else main_case(seed))


if __name__ == "__main__":
    main()
