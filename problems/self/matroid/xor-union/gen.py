#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""GF(2) の線形マトロイドの和の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

入力は「族の名前 seed 引数...」の 1 行で、ベクトルはハーネス (base.cpp) が作る。族と引数の意味は base.cpp の冒頭に
書いてある。seed 0 から 9 は本番のケース。seed 1000 以上は愚直解 (brute.hpp) と突き合わせる小さい入力 (ベクトル
10 本まで、組 3 つまで) で、族を seed で決め、引数を splitmix64 で選ぶ。
"""
import sys

MASK64 = (1 << 64) - 1

CASES = {
    0: "random 1 200000 64 2",
    1: "random 2 200000 64 8",
    2: "random 3 200000 40 5",
    3: "low 4 200000 64 30 3",
    4: "graph 5 200000 64 4",
    5: "graph 6 3000 64 30",
    6: "dup 7 200000 64 100 4",
    7: "sparse 8 200000 64 3 6",
    8: "low 9 5000 64 10 20",
    9: "random 10 1000 60 20",
}


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


def small_case(seed: int) -> str:
    """ベクトルが 10 本まで、組が 3 つまでの入力。族を seed で順に回す。"""
    r = SplitMix64(seed * 1000003 + 7)
    fam = seed % 5
    m, d, k = r.range(0, 10), r.range(2, 5), r.range(1, 3)
    if fam == 0:
        return f"random {seed} {m} {d} {k}"
    if fam == 1:
        return f"low {seed} {m} {d} {r.range(1, d)} {k}"
    if fam == 2:
        return f"graph {seed} {m} {d} {k}"
    if fam == 3:
        return f"dup {seed} {m} {d} {r.range(1, 4)} {k}"
    return f"sparse {seed} {m} {d} {r.range(1, 2)} {k}"


def main() -> None:
    seed = int(sys.argv[1])
    print(CASES[seed] if seed in CASES else small_case(seed))


if __name__ == "__main__":
    main()
