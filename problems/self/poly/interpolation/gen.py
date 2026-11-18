#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""多項式補間の係数の入力を作る (mod 998244353)。x は相異なる。

seed ごとに (点の数の範囲, ケース数) を決める。7 は x = 0..n-1、8 は x がある値の近くに固まる形。
1000 以上の seed は愚直解 (brute.hpp) と突き合わせるための小さい入力。
"""
import random
import sys

P = 998244353
SIZES = {
    1: ((1, 8), 20000),
    2: ((60, 70), 500),
    3: ((300, 300), 20),
    4: ((1000, 1000), 3),
    5: ((3000, 3000), 1),
    6: ((5000, 5000), 1),
}


def distinct(rng: random.Random, n: int) -> list[int]:
    s = set()
    while len(s) < n:
        s.add(rng.randrange(P))
    xs = list(s)
    rng.shuffle(xs)
    return xs


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed * 1_000_003 + 13)
    cs = []
    if seed == 0:
        cs.append(([0, 1, 2], [1, 2, 5]))  # x^2 + 1
        cs.append(([5], [7]))
        cs.append(([P - 1, 1], [0, 2]))
    elif seed in SIZES:
        (lo, hi), t = SIZES[seed]
        for _ in range(t):
            n = rng.randint(lo, hi)
            cs.append((distinct(rng, n), [rng.randrange(P) for _ in range(n)]))
    elif seed == 7:
        n = 3000
        cs.append((list(range(n)), [rng.randrange(P) for _ in range(n)]))
    elif seed == 8:
        n = 2000
        base = rng.randrange(P)
        xs = [(base + i * i) % P for i in range(n)]
        cs.append((xs, [rng.randrange(P) for _ in range(n)]))
    else:  # 1000 以上: 小さい入力
        for _ in range(100):
            n = rng.randint(1, 7)
            cs.append((distinct(rng, n), [rng.randrange(P) for _ in range(n)]))
    out = [str(len(cs))]
    for xs, ys in cs:
        out.append(str(len(xs)))
        out.append(" ".join(map(str, xs)))
        out.append(" ".join(map(str, ys)))
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
