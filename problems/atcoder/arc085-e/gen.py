#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""ARC 085 E (MUL) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

seed 0 から 3 は問題文の例、4 と 5 は N = 1 の角のケース、6 から 13 は N = 100 (乱択、全部が正、全部が負、絶対値が最大で
符号が交互、素数の番号だけ正、合成数の番号だけ大きく正で約数が負、1 だけ大きく負、小さい値ばかりで同点が多いもの)。
seed 1000 以上は、割る宝石の集合を全部試す愚直解 (brute.cpp) で解ける N <= 16 の入力。
"""
import sys

MASK64 = (1 << 64) - 1
AMAX = 10**9


class SplitMix64:
    def __init__(self, seed: int) -> None:
        self.x = seed & MASK64

    def next(self) -> int:
        self.x = (self.x + 0x9E3779B97F4A7C15) & MASK64
        z = self.x
        z = ((z ^ (z >> 30)) * 0xBF58476D1CE4E5B9) & MASK64
        z = ((z ^ (z >> 27)) * 0x94D049BB133111EB) & MASK64
        return z ^ (z >> 31)

    def range(self, lo: int, hi: int) -> int:
        return lo + (self.next() * (hi - lo + 1) >> 64)


SAMPLES = {
    0: "6\n1 2 -6 4 5 3\n",
    1: "6\n100 -100 -100 -100 100 -100\n",
    2: "5\n-1 -2 -3 -4 -5\n",
    3: "2\n-1000 100000\n",
}


def fmt(a: list[int]) -> str:
    return f"{len(a)}\n" + " ".join(map(str, a)) + "\n"


def is_prime(x: int) -> bool:
    return x >= 2 and all(x % d for d in range(2, int(x**0.5) + 1))


def case(seed: int) -> str:
    if seed in SAMPLES:
        return SAMPLES[seed]
    r = SplitMix64(seed * 998244353 + 13)
    n = 100
    if seed == 4:
        return fmt([AMAX])
    if seed == 5:
        return fmt([-AMAX])
    if seed == 6:
        return fmt([r.range(-AMAX, AMAX) for _ in range(n)])
    if seed == 7:
        return fmt([r.range(1, AMAX) for _ in range(n)])
    if seed == 8:
        return fmt([-r.range(1, AMAX) for _ in range(n)])
    if seed == 9:
        return fmt([AMAX if i % 2 else -AMAX for i in range(1, n + 1)])
    if seed == 10:
        return fmt([r.range(1, AMAX) if is_prime(i) else -r.range(1, AMAX) for i in range(1, n + 1)])
    if seed == 11:
        return fmt([AMAX - r.range(0, 1000) if i > 1 and not is_prime(i) else -r.range(1, AMAX // 50) for i in range(1, n + 1)])
    if seed == 12:
        return fmt([-AMAX] + [r.range(-AMAX // 100, AMAX // 10) for _ in range(n - 1)])
    if seed == 13:
        return fmt([r.range(-3, 3) for _ in range(n)])
    raise SystemExit(f"seed {seed} のケースはありません")


def small_case(seed: int) -> str:
    r = SplitMix64(seed * 1000003 + 7)
    n = r.range(1, 16)
    m = [10, 1000, AMAX][r.range(0, 2)]
    return fmt([r.range(-m, m) for _ in range(n)])


def main() -> None:
    seed = int(sys.argv[1])
    print(small_case(seed) if seed >= 1000 else case(seed), end="")


if __name__ == "__main__":
    main()
