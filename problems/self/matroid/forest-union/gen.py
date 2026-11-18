#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""グラフ的マトロイドの和 (k 本の森への分解) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

入力は「族の名前 seed 引数...」の 1 行で、グラフはハーネス (base.cpp) が作る。族と引数の意味は base.cpp の冒頭に
書いてある。seed 0 から 9 は本番のケース。seed 1000 以上は愚直解 (brute.hpp) と突き合わせる小さい入力 (辺 12 本
まで、森 3 本まで) で、族を seed で決め、引数を splitmix64 で選ぶ。
"""
import sys

MASK64 = (1 << 64) - 1

CASES = {
    0: "random 1 3000 9000 2",
    1: "random 2 5000 12000 3",
    2: "grid 3 100 100 2",
    3: "cubic 4 10000 2",
    4: "complete 5 200 100",
    5: "complete 6 150 10",
    6: "core 7 5000 10000 120 3",
    7: "planted 8 5000 2 5000",
    8: "planted 9 3000 4 3000",
    9: "multi 10 60 30000 20",
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
    """辺が 12 本まで、森が 3 本までの入力。族を seed で順に回す。"""
    r = SplitMix64(seed * 1000003 + 7)
    fam = seed % 7
    k = r.range(1, 3)
    if fam == 0:
        return f"random {seed} {r.range(2, 7)} {r.range(0, 12)} {k}"
    if fam == 1:
        return f"grid {seed} {r.range(1, 3)} {r.range(1, 3)} {k}"
    if fam == 2:
        return f"cubic {seed} {r.range(2, 6)} {k}"
    if fam == 3:
        return f"complete {seed} {r.range(1, 5)} {k}"
    if fam == 4:
        return f"core {seed} {r.range(3, 7)} {r.range(0, 6)} {r.range(2, 4)} {k}"
    if fam == 5:
        return f"planted {seed} {r.range(2, 5)} {r.range(1, 2)} {r.range(0, 4)}"
    return f"multi {seed} {r.range(2, 3)} {r.range(0, 12)} {k}"


def main() -> None:
    seed = int(sys.argv[1])
    print(CASES[seed] if seed in CASES else small_case(seed))


if __name__ == "__main__":
    main()
