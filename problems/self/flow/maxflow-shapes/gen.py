#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""最大流 (問題の形の網) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

入力は「族の名前 seed 引数...」の 1 行で、グラフそのものはハーネス (base.cpp) が作る。族と引数の意味は base.cpp の
冒頭に書いてある。seed 0 から 9 は本番のケースで、頂点が 2400 から 100 万、弧の組が 120 万から 300 万ほど。
seed 1000 以上は愚直解 (brute.hpp) と突き合わせる小さい入力で、族を seed で決め、引数を splitmix64 で選ぶ。
"""
import sys

MASK64 = (1 << 64) - 1

CASES = {
    0: "cgrid 1 1000 1000 1000000",
    1: "cgrid 2 700 700 10",
    2: "cgrid 3 300 3000 1000000",
    3: "cgrid_dir 4 500 500 1000000",
    4: "presuf 5 200000 200000",
    5: "presuf 6 200000 100000",
    6: "segtree 7 100000 100000",
    7: "segtree 8 100000 50000",
    8: "closure 9 1200 1200 1000 100000",
    9: "closure 10 2000 500 300 100000",
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
    """頂点が 30 個ほどまでの入力。族を seed で順に回す。"""
    r = SplitMix64(seed * 1000003 + 23)
    fam = seed % 5
    if fam == 0:
        return f"cgrid {seed} {r.range(1, 5)} {r.range(2, 5)} {r.range(1, 5)}"
    if fam == 1:
        return f"cgrid_dir {seed} {r.range(1, 4)} {r.range(1, 4)} {r.range(1, 5)}"
    if fam == 2:
        return f"presuf {seed} {r.range(1, 6)} {r.range(2, 6)}"
    if fam == 3:
        return f"segtree {seed} {r.range(1, 6)} {r.range(1, 8)}"
    q = r.range(1, 5)
    return f"closure {seed} {r.range(1, 5)} {q} {r.range(0, q)} {r.range(1, 10)}"


def main() -> None:
    seed = int(sys.argv[1])
    if seed in CASES:
        print(CASES[seed])
    elif seed >= 1000:
        print(small_case(seed))
    else:
        raise SystemExit(f"seed {seed} は 0 から {len(CASES) - 1} か、1000 以上にしてください")


if __name__ == "__main__":
    main()
