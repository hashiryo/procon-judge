#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""接頭辞の個数 (追加と削除つき) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

入力は操作の列の作り方だけで、操作の列はハーネスが作る (base.cpp)。
1 行目がブロックの数 T、続く T 行が「作り方 ops seed σ lo hi」。
seed 0 は小さいブロックを 200 個並べたもの、seed 1 から 8 は本番の大きさのケース (文字の合計が 300 万前後)。
seed 1000 以上は愚直解 (brute.hpp) と突き合わせる小さい入力で、作り方を splitmix64 で選ぶ。
"""
import sys

MASK64 = (1 << 64) - 1


class SplitMix64:
    def __init__(self, seed: int) -> None:
        self.x = seed & MASK64

    def next(self) -> int:
        self.x = (self.x + 0x9E3779B97F4A7C15) & MASK64
        z = self.x
        z = ((z ^ (z >> 30)) * 0xBF58476D1CE4E5B9) & MASK64
        z = ((z ^ (z >> 27)) * 0x94D049BB133111EB) & MASK64
        return z ^ (z >> 31)

    def below(self, n: int) -> int:
        return (self.next() * n) >> 64

    def range(self, lo: int, hi: int) -> int:
        return lo + self.below(hi - lo + 1)


def small_blocks(rng: SplitMix64, count: int, max_ops: int) -> list[str]:
    rows = []
    for _ in range(count):
        lo = rng.range(1, 4)
        hi = rng.range(lo, 6)
        mode = "tree" if rng.below(2) else "words"
        rows.append(f"{mode} {rng.range(0, max_ops)} {{}} {rng.range(1, 3)} {lo} {hi}")
    return rows


# seed -> ブロックの並び。seed の欄 {} は後で埋める。
CASES = {
    1: ["words 500000 {} 26 1 10"],
    2: ["words 200000 {} 2 1 30"],
    3: ["tree 100000 {} 26 5 50"],
    4: ["tree 100000 {} 2 20 60"],
    5: ["words 30 {} 26 100000 100000"],
    6: ["words 1000000 {} 26 1 4"],
    7: ["tree 2000 {} 4 1000 2000"],
    8: ["words 300 {} 26 1 10"] * 1000,
}


def rows_for(seed: int) -> list[str]:
    if seed == 0:
        return small_blocks(SplitMix64(20261107), 200, 60)
    if seed in CASES:
        return CASES[seed]
    if seed >= 1000:
        rng = SplitMix64(seed * 1000003 + 19)
        return small_blocks(rng, rng.range(1, 3), 40)
    raise SystemExit(f"seed {seed} は 0 から {max(CASES)} か、1000 以上にしてください")


def main() -> None:
    seed = int(sys.argv[1])
    rows = rows_for(seed)
    out = [str(len(rows))]
    out += [s.format(seed * 1_000_003 + i) for i, s in enumerate(rows)]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
