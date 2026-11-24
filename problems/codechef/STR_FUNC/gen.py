#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""CodeChef STR_FUNC (Strange Functions) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

1 行目に N、2 行目に A (1 以上 1e9 以下)。seed 0 は問題文の例。seed 1 から 7 は、N = 6e5 の乱数、N = 2^19 (524288)、N = 2^19 + 1、
N = 1、N = 2、値が全部 1e9、値が全部 1。seed 1000 以上は N <= 64 の小さい入力 (愚直解 brute.hpp 用)。
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
    rng = SplitMix64(seed * 1000003 + 29)
    if seed == 0:
        print("5\n1 1 1 2 3")
        return
    mode = "rand"
    if seed >= 1000:
        n = 1 + rng.next() % 64
    else:
        n = {1: 600000, 2: 1 << 19, 3: (1 << 19) + 1, 4: 1, 5: 2, 6: 600000, 7: 600000}[seed]
        mode = {6: "max", 7: "one"}.get(seed, "rand")
    if mode == "max":
        a = [10**9] * n
    elif mode == "one":
        a = [1] * n
    else:
        a = [1 + rng.next() % 10**9 for _ in range(n)]
    print(n)
    print(" ".join(map(str, a)))


if __name__ == "__main__":
    main()
