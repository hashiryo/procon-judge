#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""CodeChef DANYANUM (Danya and Numbers) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

1 行目に N、M、k、2 行目に初めの列 A、続く M 行に操作 (1 v は v を足す、2 v は v を 1 つ消す、3 x は f(x, A) を問う)。
seed 0 は問題文の例。seed 1 から 11 は N = M = 75000 で、k = 18 の乱数 (3 種類が同じくらい、問いが 8 割で x が小さい、
問いが 8 割で x が |A| に近い)、bit の多い値 (乱数の OR)、bit の少ない値 (乱数の AND)、足すのを先に並べて問いを後に並べたもの、
値の種類が少ないもの、消すのが多いもの、k = 1、k = 17、x = 1 だけのもの。消す数は今 A にあるものから選び、x は 1 から |A|。
seed 1000 以上は k = 1 から 6、N <= 10、M <= 20 の小さい入力 (愚直解 brute.hpp 用)。
乱数は Python の random の版による違いを避けるため splitmix64 で作る。
"""
import sys

MASK64 = (1 << 64) - 1


class SplitMix64:
    def __init__(self, seed: int) -> None:
        self.x = seed & MASK64

    def next(self) -> int:
        self.x = (self.x + 0x9E3779B97F4A7C15) & MASK64
        z = self.x
        z = ((z ^ (z >> 30)) * 0xBF58476D1CE4E5B9) & MASK64
        z = ((z ^ (z >> 27)) * 0x94D049BB133111EB) & MASK64
        return z ^ (z >> 31)


def main() -> None:
    seed = int(sys.argv[1])
    rng = SplitMix64(seed * 1000003 + 13)
    if seed == 0:
        print("4 7 3")
        print("6 7 5 2")
        print("\n".join(["3 2", "3 3", "1 7", "3 3", "2 7", "2 6", "3 2"]))
        return
    # (k, 足す, 消す, 問うの重み, 値の作り方, x の作り方, 足すのを先に並べるか)
    kinds = {
        1: (18, 1, 1, 1, "rand", "rand", False), 2: (18, 1, 1, 8, "rand", "small", False),
        3: (18, 1, 1, 8, "rand", "large", False), 4: (18, 1, 1, 1, "or", "rand", False),
        5: (18, 1, 1, 1, "and", "rand", False), 6: (18, 1, 0, 1, "rand", "rand", True),
        7: (18, 1, 1, 1, "few", "rand", False), 8: (18, 1, 3, 1, "rand", "rand", False),
        9: (1, 1, 1, 1, "rand", "rand", False), 10: (17, 1, 1, 1, "rand", "rand", False),
        11: (18, 1, 1, 2, "rand", "one", False),
    }
    if seed >= 1000:
        k = 1 + rng.next() % 6
        n0, m = 1 + rng.next() % 10, 1 + rng.next() % 20
        wa, wd, wq, vm, xm, first = 1, 1, 1, "rand", "rand", False
    else:
        k, wa, wd, wq, vm, xm, first = kinds[seed]
        n0, m = 75000, 75000
    full = (1 << k) - 1
    pool = [rng.next() & full for _ in range(64)]

    def val() -> int:
        if vm == "or":
            return (rng.next() | rng.next()) & full
        if vm == "and":
            return rng.next() & rng.next() & rng.next() & full
        if vm == "few":
            return pool[rng.next() % len(pool)]
        return rng.next() & full

    A = [val() for _ in range(n0)]
    cur = list(A)
    ops = []
    if first:
        kinds_seq = [1] * (m // 2) + [3] * (m - m // 2)
    else:
        tot = wa + wd + wq
        kinds_seq = []
        for _ in range(m):
            r = rng.next() % tot
            kinds_seq.append(1 if r < wa else 2 if r < wa + wd else 3)
    for t in kinds_seq:
        if t != 1 and not cur:
            t = 1
        if t == 1:
            v = val()
            cur.append(v)
            ops.append(f"1 {v}")
        elif t == 2:
            i = rng.next() % len(cur)
            cur[i], cur[-1] = cur[-1], cur[i]
            ops.append(f"2 {cur.pop()}")
        else:
            sz = len(cur)
            if xm == "small":
                x = 1 + rng.next() % min(sz, 3)
            elif xm == "large":
                x = sz - rng.next() % min(sz, 3)
            elif xm == "one":
                x = 1
            else:
                x = 1 + rng.next() % sz
            ops.append(f"3 {x}")
    print(n0, m, k)
    print(" ".join(map(str, A)))
    print("\n".join(ops))


if __name__ == "__main__":
    main()
