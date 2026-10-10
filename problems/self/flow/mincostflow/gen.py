#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""最小費用流 (グラフの族) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

入力は「族の名前 seed 引数...」の 1 行で、グラフそのものはハーネス (base.cpp) が作る。族と引数の意味は base.cpp の
冒頭に書いてある。seed 0 から 9 は本番のケースで、頂点が 1 万から 10 万、辺が 6 万から 40 万ほど。参照実装 (ネットワーク
単体法) が、手元でどのケースも 5 秒ほどまでで解ける大きさにしてある (テストデータは CI のジョブごとに作り直すため)。
seed 1000 以上は愚直解 (brute.hpp) と突き合わせる小さい入力で、族を seed で決め、引数を splitmix64 で選ぶ。
"""
import sys

MASK64 = (1 << 64) - 1

CASES = {
    0: "netgen 1 30000 240000 100 100 100000 10000 1000",
    1: "netgen 2 20000 400000 1000 1000 1000000 100 10000",
    2: "assign 3 50000 5 1000000",
    3: "assign 4 50000 20 100",
    4: "transport 5 150 150 100 100",
    5: "transport 6 50 500 1000 10000",
    6: "stflow 7 20000 160000 500 1000000 10000 1000",
    7: "mixed 8 10000 60000 10000 1000",
    8: "mixed 9 20000 60000 100 1000000",
    9: "netgen 10 50000 200000 200 200 1000000 100 1000",
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
    r = SplitMix64(seed * 1000003 + 11)
    fam = seed % 5
    if fam == 0:
        s, t = r.range(1, 2), r.range(1, 2)
        n = r.range(s + t, 8)
        return f"netgen {seed} {n} {r.range(0, 16)} {s} {t} {r.range(0, 5)} {r.range(1, 10)} {r.range(1, 4)}"
    if fam == 1:
        return f"assign {seed} {r.range(1, 4)} {r.range(0, 2)} {r.range(0, 10)}"
    if fam == 2:
        return f"transport {seed} {r.range(1, 3)} {r.range(1, 3)} {r.range(0, 3)} {r.range(1, 5)}"
    if fam == 3:
        return f"stflow {seed} {r.range(1, 6)} {r.range(0, 12)} {r.range(1, 3)} {r.range(0, 6)} {r.range(1, 5)} {r.range(1, 3)}"
    return f"mixed {seed} {r.range(1, 6)} {r.range(0, 12)} {r.range(1, 10)} {r.range(0, 4)}"


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
