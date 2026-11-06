#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""接頭辞関数 の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

入力は文字列の作り方だけで、文字列はハーネスが作る (base.cpp と _shared/string/_common.hpp)。
1 行目がブロックの数 T、続く T 行が「族 n seed x y z」。
seed 0 は小さい文字列を 200 個並べたもの、seed 1 から 10 は本番の大きさのケース。
seed 1000 以上は愚直解 (brute.hpp) と突き合わせる小さい入力で、族を splitmix64 で選ぶ。
"""
import sys

MASK64 = (1 << 64) - 1
M = 10**6


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


def small_blocks(rng: SplitMix64, count: int, max_n: int) -> list[str]:
    """長さ 1 から max_n の文字列を count 個。族と周期を乱択で選ぶ。seed の欄は後で埋める。"""
    rows = []
    for _ in range(count):
        n = rng.range(1, max_n)
        fam = rng.below(7)
        p = rng.range(1, 8)
        if fam == 0:
            rows.append(f"random {n} {{}} {rng.range(1, 3)} 0 0")
        elif fam == 1:
            rows.append(f"period {n} {{}} {p} {rng.range(1, 3)} 0")
        elif fam == 2:
            rows.append(f"noisy {n} {{}} {p} {rng.range(2, 3)} {rng.range(1, 3)}")
        elif fam == 3:
            rows.append(f"fib {n} {{}} 0 0 0")
        elif fam == 4:
            rows.append(f"thue {n} {{}} 0 0 0")
        elif fam == 5:
            rows.append(f"carry {n} {{}} 0 0 0")
        else:
            rows.append(f"blocks {n} {{}} {p} 0 0")
    return rows


# seed -> 文字列の作り方の並び。seed の欄 {} は後で埋める。
CASES = {
    1: [f"random {M} {{}} 26 0 0"],
    2: [f"random {M} {{}} 2 0 0"],
    3: [f"period {M} {{}} 1 1 0"],
    4: [f"fib {M} {{}} 0 0 0"],
    5: [f"thue {M} {{}} 0 0 0"],
    6: [f"carry {M} {{}} 0 0 0"],
    7: [f"noisy {M} {{}} 1000 26 300"],
    8: [f"period {M} {{}} 1000 26 0"],
    9: [f"noisy {M} {{}} 7 2 10000"],
    10: [f"random 1000 {{}} 2 0 0"] * 1000,
}


def rows_for(seed: int) -> list[str]:
    if seed == 0:
        return small_blocks(SplitMix64(20261107), 200, 100)
    if seed in CASES:
        return CASES[seed]
    if seed >= 1000:
        rng = SplitMix64(seed * 1000003 + 13)
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
