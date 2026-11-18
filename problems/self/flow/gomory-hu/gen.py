#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""Gomory-Hu 木 (グラフの族) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

入力は「族の名前 seed 引数...」の 1 行で、グラフそのものはハーネス (base.cpp) が作る。族と引数の意味は base.cpp の
冒頭に書いてある。seed 0 から 9 は本番のケースで、頂点が 300 から 3000、辺が 3000 から 4.5 万ほど。参照実装
(組ごとに網を作り直す素朴な Dinic) が手元でどのケースも数秒で解ける大きさにしてある (テストデータは CI のジョブごとに
作り直すため)。seed 1000 以上は愚直解 (brute.hpp) と突き合わせる小さい入力で、族を seed で決め、引数を splitmix64 で
選ぶ。
"""
import sys

MASK64 = (1 << 64) - 1

CASES = {
    0: "random 1 2000 20000 1 100000",
    1: "dense 2 300 100",
    2: "grid 3 40 50 1000",
    3: "cluster 4 20 100 5 300 1000",
    4: "unit 5 2000 8000",
    5: "cycle 6 3000 500 1000",
    6: "tree 7 3000 300 1000",
    7: "hub 8 2000 4 6000 1000",
    8: "comps 9 2000 8 16000 1000",
    9: "random 10 1500 6000 0 10",
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
    """頂点が 12 個までの入力。族を seed で順に回す。"""
    r = SplitMix64(seed * 1000003 + 11)
    fam = seed % 10
    c = r.range(1, 6)
    if fam == 0:
        n, c0 = r.range(1, 12), r.range(0, 2)
        return f"random {seed} {n} {r.range(0, 30)} {c0} {c0 + c}"
    if fam == 1:
        return f"dense {seed} {r.range(1, 8)} {c}"
    if fam == 2:
        h = r.range(1, 3)
        return f"grid {seed} {h} {r.range(1, 12 // h)} {c}"
    if fam == 3:
        k = r.range(1, 4)
        return f"cluster {seed} {k} {r.range(1, 12 // k)} {r.range(0, 3)} {r.range(0, 4)} {c}"
    if fam == 4:
        return f"unit {seed} {r.range(1, 12)} {r.range(0, 30)}"
    if fam == 5:
        return f"cycle {seed} {r.range(1, 12)} {r.range(0, 5)} {c}"
    if fam == 6:
        return f"tree {seed} {r.range(1, 12)} {r.range(0, 5)} {c}"
    if fam == 7:
        n = r.range(1, 12)
        return f"hub {seed} {n} {r.range(1, 3)} {r.range(0, 10)} {c}"
    if fam == 8:
        n = r.range(1, 12)
        return f"comps {seed} {n} {r.range(1, 4)} {r.range(0, 25)} {c}"
    return f"path {seed} {r.range(1, 12)} {c}"


def main() -> None:
    seed = int(sys.argv[1])
    print(CASES[seed] if seed in CASES else small_case(seed))


if __name__ == "__main__":
    main()
