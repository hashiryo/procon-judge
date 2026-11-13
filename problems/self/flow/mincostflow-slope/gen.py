#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""s-t の最小費用流の slope (グラフの族) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

入力は「族の名前 seed 引数...」の 1 行で、グラフそのものはハーネス (base.cpp) が作る。族と引数の意味は base.cpp の
冒頭に書いてある。seed 0 から 11 は本番のケースで、参照実装 (教科書どおりの逐次最短路) が手元でどのケースも数秒までで
解ける大きさにしてある (テストデータは CI のジョブごとに作り直すため)。seed 1000 以上は愚直解 (brute.hpp) と突き合わせる
小さい入力で、族を seed で決め、引数を splitmix64 で選ぶ。
"""
import sys

MASK64 = (1 << 64) - 1

CASES = {
    0: "random 1 20000 200000 100 1000 0 1",
    1: "assign 2 3000 3000 10 1000000 0 1",
    2: "assign 3 2000 2000 20 100 1 0",
    3: "grid 4 80 80 1 0",
    4: "dagpaths 5 2000 10 1000000 1",
    5: "intervals 6 100000 100000 100 1000000 1000 0",
    6: "dense 7 300 1000000 100",
    7: "gridflow 8 100 100 100 100",
    8: "expand 9 5000 50000 1000 100 100",
    9: "cooking 10 40 100 800 1000",
    10: "cooking 11 60 150 1500 1000",
    11: "random 12 2000 200000 10 10 0 0",
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
    """頂点が 10 個ほどまでの入力。族を seed で順に回す。"""
    r = SplitMix64(seed * 1000003 + 17)
    fam = seed % 9
    sh = r.range(0, 1)
    if fam == 0:
        n = r.range(2, 7)
        lim = r.range(0, 1) * r.range(1, 6)
        return f"random {seed} {n} {r.range(n, 16)} {r.range(1, 3)} {r.range(0, 5)} {lim} {sh}"
    if fam == 1:
        return f"assign {seed} {r.range(1, 4)} {r.range(1, 4)} {r.range(1, 3)} {r.range(0, 6)} {r.range(0, 1)} {sh}"
    if fam == 2:
        return f"grid {seed} {r.range(1, 3)} {r.range(1, 4)} {r.range(0, 3)} {sh}"
    if fam == 3:
        return f"dagpaths {seed} {r.range(1, 6)} {r.range(1, 4)} {r.range(1, 5)} {sh}"
    if fam == 4:
        return f"intervals {seed} {r.range(1, 6)} {r.range(0, 7)} {r.range(1, 3)} {r.range(1, 5)} {r.range(1, 4)} {sh}"
    if fam == 5:
        return f"dense {seed} {r.range(2, 4)} {r.range(1, 3)} {r.range(0, 5)}"
    if fam == 6:
        return f"gridflow {seed} {r.range(1, 3)} {r.range(1, 3)} {r.range(1, 2)} {r.range(1, 4)}"
    if fam == 7:
        n = r.range(2, 6)
        return f"expand {seed} {n} {r.range(n, 14)} {r.range(1, 3)} {r.range(1, 3)} {r.range(1, 4)}"
    nd = r.range(1, 3)
    return f"cooking {seed} {nd} {r.range(1, 3)} {r.range(nd, 6)} {r.range(1, 5)}"


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
