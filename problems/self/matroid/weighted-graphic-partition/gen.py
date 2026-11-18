#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""グラフ的マトロイドと分割マトロイドの重み付きの交差の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

入力は「族の名前 seed 引数...」の 1 行で、グラフと色と重みはハーネス (base.cpp) が作る。族と引数の意味は base.cpp の
冒頭に書いてある。seed 0 から 9 は本番のケース。seed 1000 以上は愚直解 (brute.hpp) と突き合わせる小さい入力
(辺 14 本まで) で、族を seed で決め、引数を splitmix64 で選ぶ。重みの上限は、同じ重みが多くなるように小さくもする。
"""
import sys

MASK64 = (1 << 64) - 1

CASES = {
    0: "random 1 300 1000 30 10 1000000000",
    1: "random 2 500 2000 500 1 1000000000",
    2: "random 3 500 2000 50 10 3",
    3: "grid 4 20 20 40 10 1000000000",
    4: "vertex 5 400 2000 1 1000000000",
    5: "few 6 400 2000 4 1000000000",
    6: "few 7 400 2000 4 2",
    7: "complete 8 60 1000000000",
    8: "complete 9 60 5",
    9: "random 10 1000 3000 100 10 1000000000",
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
    """辺が 14 本までの入力。族を seed で順に回す。"""
    r = SplitMix64(seed * 1000003 + 7)
    fam = seed % 5
    wt = [1, 2, 3, 10, 1000000000][r.range(0, 4)]
    if fam == 0:
        return f"random {seed} {r.range(2, 8)} {r.range(0, 14)} {r.range(1, 5)} {r.range(0, 3)} {wt}"
    if fam == 1:
        return f"grid {seed} {r.range(1, 3)} {r.range(1, 3)} {r.range(1, 4)} {r.range(0, 2)} {wt}"
    if fam == 2:
        return f"vertex {seed} {r.range(2, 8)} {r.range(0, 14)} {r.range(0, 2)} {wt}"
    if fam == 3:
        return f"few {seed} {r.range(2, 8)} {r.range(0, 14)} {r.range(1, 4)} {wt}"
    return f"complete {seed} {r.range(1, 5)} {wt}"


def main() -> None:
    seed = int(sys.argv[1])
    print(CASES[seed] if seed in CASES else small_case(seed))


if __name__ == "__main__":
    main()
