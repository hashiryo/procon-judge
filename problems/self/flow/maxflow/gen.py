#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""最大流 (グラフの族) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

入力は「族の名前 seed 引数...」の 1 行で、グラフそのものはハーネス (base.cpp) が作る。族と引数の意味は base.cpp の
冒頭に書いてある。seed 0 から 9 は本番のケースで、どれも頂点が 6 万から 18 万、辺が 20 万から 54 万ほど。
seed 1000 以上は愚直解 (brute.hpp) と突き合わせる小さい入力で、族を seed で決め、引数を splitmix64 で選ぶ。
"""
import sys

MASK64 = (1 << 64) - 1

CASES = {
    0: "grid 1 300 300 100 100",
    1: "grid 2 300 300 10000 100",
    2: "bipartite 3 100000 100000 3",
    3: "band 4 100000 2 2",
    4: "sparse 5 100000 500000 1000 1000",
    5: "genrmf 6 16 256 1 10000",
    6: "genrmf 7 64 16 1 10000",
    7: "rlg 8 256 256 3 1000",
    8: "closure 9 60000 60000 4 1000",
    9: "rlg 10 64 1024 4 1000",
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
    """頂点が 40 個ほどまでの入力。族を seed で順に回す。"""
    r = SplitMix64(seed * 1000003 + 7)
    fam = seed % 7
    c = r.range(1, 10)
    if fam == 0:
        return f"grid {seed} {r.range(1, 6)} {r.range(1, 6)} {c} {r.range(1, 10)}"
    if fam == 1:
        return f"bipartite {seed} {r.range(1, 8)} {r.range(1, 8)} {r.range(1, 3)}"
    if fam == 2:
        return f"band {seed} {r.range(1, 12)} {r.range(1, 3)} {r.range(0, 2)}"
    if fam == 3:
        return f"sparse {seed} {r.range(1, 12)} {r.range(0, 40)} {r.range(1, 3)} {c}"
    if fam == 4:
        a = r.range(1, 3)
        # A = B = 1 だと頂点が 1 つで s = t になるので、A = 1 なら frame を 2 枚以上にする。
        return f"genrmf {seed} {a} {r.range(2 if a == 1 else 1, 4)} 1 {c}"
    if fam == 5:
        return f"rlg {seed} {r.range(1, 5)} {r.range(1, 5)} {r.range(1, 3)} {c}"
    return f"closure {seed} {r.range(1, 8)} {r.range(1, 8)} {r.range(1, 3)} {c}"


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
