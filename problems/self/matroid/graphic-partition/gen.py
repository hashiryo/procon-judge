#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""グラフ的マトロイドと分割マトロイドの交差の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

入力は「族の名前 seed 引数...」の 1 行で、グラフと色はハーネス (base.cpp) が作る。族と引数の意味は base.cpp の
冒頭に書いてある。seed 0 から 9 は本番のケース。seed 1000 以上は愚直解 (brute.hpp) と突き合わせる小さい入力
(辺 16 本まで) で、族を seed で決め、引数を splitmix64 で選ぶ。
"""
import sys

MASK64 = (1 << 64) - 1

CASES = {
    0: "random 1 3000 9000 300 10",
    1: "random 2 3000 6000 3000 1",
    2: "grid 3 70 70 1225 4",
    3: "vertex 4 3000 15000 1",
    4: "vertex 5 2000 8000 2",
    5: "few 6 3000 20000 20",
    6: "complete 7 300",
    7: "cycles 8 6000 3 3000",
    8: "random 9 5000 20000 5000 1",
    9: "few 10 5000 30000 2",
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
    """辺が 16 本までの入力。族を seed で順に回す。"""
    r = SplitMix64(seed * 1000003 + 7)
    fam = seed % 6
    if fam == 0:
        return f"random {seed} {r.range(2, 8)} {r.range(0, 16)} {r.range(1, 5)} {r.range(0, 3)}"
    if fam == 1:
        h = r.range(1, 3)
        return f"grid {seed} {h} {r.range(1, 3)} {r.range(1, 4)} {r.range(0, 2)}"
    if fam == 2:
        return f"vertex {seed} {r.range(2, 8)} {r.range(0, 16)} {r.range(0, 2)}"
    if fam == 3:
        return f"few {seed} {r.range(2, 8)} {r.range(0, 16)} {r.range(1, 4)}"
    if fam == 4:
        return f"complete {seed} {r.range(1, 6)}"
    return f"cycles {seed} {r.range(3, 12)} {r.range(3, 5)} {r.range(1, 6)}"


def main() -> None:
    seed = int(sys.argv[1])
    print(CASES[seed] if seed in CASES else small_case(seed))


if __name__ == "__main__":
    main()
