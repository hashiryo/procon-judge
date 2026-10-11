#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""最小費用流 (鎖と木の形の網) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

入力は「族の名前 seed 引数...」の 1 行で、グラフそのものはハーネス (base.cpp) が作る。族と引数の意味は base.cpp の
冒頭に書いてある。seed 0 から 9 は本番のケースで、頂点が 1 万から 30 万、辺が 3 万から 90 万ほど。参照実装 (ネットワーク
単体法) が手元でどのケースも数秒で解ける大きさにしてある (テストデータは CI のジョブごとに作り直すため)。cost scaling は
長い鎖を余りが 1 歩ずつ進むので鎖の族で頂点の数の 2 乗ほどかかり、ネットワーク単体法も ring と buysell と deadline では
2 乗ほどで伸びる (手元で、ring の 30 万頂点は参照実装で 214 秒、buysell の 3 万頂点は 15 秒かかった)。
seed 1000 以上は愚直解 (brute.hpp) と突き合わせる小さい入力で、族を seed で決め、引数を splitmix64 で選ぶ。
"""
import sys

MASK64 = (1 << 64) - 1

CASES = {
    0: "iso_chain 1 200000 1000000000 300000000",
    1: "iso_chain 2 300000 1000000000 1000000000",
    2: "iso_tree 3 200000 1000000000 300000000 200000",
    3: "iso_tree 4 200000 1000000000 300000000 3",
    4: "ring 5 30000 1000000",
    5: "buysell 6 10000 1000000 10000",
    6: "deadline 7 20000 20000 10000",
    7: "fewsink 8 100000 3 1000000000",
    8: "snow 9 40000 40000 1000000000 1000000000 3",
    9: "iso_chain 10 200000 100 50",
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
    fam = seed % 7
    if fam == 0:
        return f"iso_chain {seed} {r.range(1, 6)} {r.range(0, 10)} {r.range(0, 10)}"
    if fam == 1:
        return f"iso_tree {seed} {r.range(1, 6)} {r.range(0, 10)} {r.range(0, 10)} {r.range(1, 5)}"
    if fam == 2:
        return f"ring {seed} {r.range(1, 6)} {r.range(0, 5)}"
    if fam == 3:
        return f"buysell {seed} {r.range(1, 6)} {r.range(1, 10)} {r.range(0, 5)}"
    if fam == 4:
        return f"deadline {seed} {r.range(1, 5)} {r.range(1, 5)} {r.range(1, 10)}"
    if fam == 5:
        return f"fewsink {seed} {r.range(1, 5)} {r.range(1, 3)} {r.range(0, 10)}"
    return f"snow {seed} {r.range(1, 4)} {r.range(1, 3)} {r.range(0, 10)} {r.range(0, 5)} {r.range(1, 3)}"


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
