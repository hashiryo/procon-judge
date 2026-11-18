#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""cf-1728-f (Fishermen) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

seed 0 から 11 は本番のケースで、問題文の例 2 つと、n = 1000 の 10 個。a_i の選び方は、10^6 以下の乱択、1000 以下の乱択、
1 から 1000 の並べ替え、全部 1、全部 10^6、30 以下の乱択、2 の冪、小さい素数、5 × 10^5 以上の乱択 (値がほとんど重ならない)、
720720 の約数 (値が多く重なる)。seed 1000 以上は、愚直解 (brute.cpp) と突き合わせる小さい入力 (n ≤ 7)。
"""
import random
import sys


def out(a: list[int]) -> str:
    return f"{len(a)}\n{' '.join(map(str, a))}"


def primes(k: int) -> list[int]:
    ps: list[int] = []
    x = 2
    while len(ps) < k:
        if all(x % p for p in ps if p * p <= x):
            ps.append(x)
        x += 1
    return ps


def main_case(seed: int) -> str:
    rng = random.Random(seed * 7919 + 17)
    n = 1000
    if seed == 0:
        return out([1, 8, 2, 3, 2, 2, 3])
    if seed == 1:
        return out([5, 6] * 5)
    if seed == 2:
        return out([rng.randint(1, 10**6) for _ in range(n)])
    if seed == 3:
        return out([rng.randint(1, 1000) for _ in range(n)])
    if seed == 4:
        a = list(range(1, n + 1))
        rng.shuffle(a)
        return out(a)
    if seed == 5:
        return out([1] * n)
    if seed == 6:
        return out([10**6] * n)
    if seed == 7:
        return out([rng.randint(1, 30) for _ in range(n)])
    if seed == 8:
        return out([1 << rng.randint(0, 19) for _ in range(n)])
    if seed == 9:
        ps = primes(100)
        return out([rng.choice(ps) for _ in range(n)])
    if seed == 10:
        return out([rng.randint(5 * 10**5, 10**6) for _ in range(n)])
    if seed == 11:
        ds = [d for d in range(1, 720721) if 720720 % d == 0]
        return out([rng.choice(ds) for _ in range(n)])
    raise SystemExit(f"seed {seed} は 0 から 11 か、1000 以上にしてください")


def small_case(seed: int) -> str:
    rng = random.Random(seed * 104729 + 7)
    n = rng.randint(1, 7)
    hi = rng.choice((3, 6, 12, 40))
    return out([rng.randint(1, hi) for _ in range(n)])


def main() -> None:
    seed = int(sys.argv[1])
    print(small_case(seed) if seed >= 1000 else main_case(seed))


if __name__ == "__main__":
    main()
