#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""一般グラフの最大マッチング (グラフの族) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

入力は「族の名前 seed 引数...」の 1 行で、グラフそのものはハーネス (base.cpp) が作る。族と引数の意味は base.cpp の
冒頭に書いてある。seed 0 から 9 は本番のケースで、頂点が 4000 から 20 万、辺が 24 万から 100 万ほど。
seed 1000 以上は愚直解 (brute.hpp) と突き合わせる小さい入力 (頂点 20 個まで) で、族を seed で決め、引数を splitmix64
で選ぶ。
"""
import sys

MASK64 = (1 << 64) - 1

CASES = {
    0: "random 1 200000 300000",
    1: "random 2 100000 1000000",
    2: "regular 3 200000 3",
    3: "tri 4 400 400 10 1",
    4: "bipartite 5 100000 100000 3",
    5: "augcycle 6 100000 250 100",
    6: "band 7 100000 2 2 100",
    7: "star 8 200000 10000 3",
    8: "cliques 9 20000 5 2",
    9: "random 10 4000 1000000",
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
    """頂点が 20 個までの入力。族を seed で順に回す。"""
    r = SplitMix64(seed * 1000003 + 7)
    fam = seed % 8
    if fam == 0:
        return f"random {seed} {r.range(2, 20)} {r.range(0, 40)}"
    if fam == 1:
        return f"regular {seed} {r.range(1, 20)} {r.range(1, 4)}"
    if fam == 2:
        h = r.range(1, 5)
        return f"tri {seed} {h} {r.range(1, 20 // h)} {r.range(0, 30)} {r.range(0, 1)}"
    if fam == 3:
        return f"bipartite {seed} {r.range(1, 10)} {r.range(1, 10)} {r.range(1, 3)}"
    if fam == 4:
        return f"augcycle {seed} {r.range(2, 10)} {r.range(1, 3)} {r.range(0, 4)}"
    if fam == 5:
        return f"band {seed} {r.range(1, 10)} {r.range(1, 3)} {r.range(0, 2)} {r.range(0, 4)}"
    if fam == 6:
        n = r.range(2, 20)
        return f"star {seed} {n} {r.range(1, min(n, 6))} {r.range(1, 3)}"
    k = r.range(1, 4)
    return f"cliques {seed} {k} {r.range(1, 20 // k if k > 1 else 5)} {r.range(0, 3)}"


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
