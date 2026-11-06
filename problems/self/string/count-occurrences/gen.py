#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""パターンの出現回数 の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

入力はテキストとパターンの作り方だけで、文字列はハーネスが作る (base.cpp と _shared/string/_common.hpp)。
1 行目がブロックの数 T、続くブロックごとに「族 n seed x y z」と「mix q seed L σ」の 2 行。
seed 0 は小さいテキストを 200 個並べたもの、seed 1 から 9 は本番の大きさのケース。
seed 1000 以上は愚直解 (brute.hpp) と突き合わせる小さい入力で、族とパターンを splitmix64 で選ぶ。
"""
import sys

MASK64 = (1 << 64) - 1
N = 5 * 10**5


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


def small_blocks(rng: SplitMix64, count: int, max_n: int, q: int) -> list[tuple[str, str]]:
    rows = []
    for _ in range(count):
        n = rng.range(1, max_n)
        sigma = rng.range(1, 3)
        fam = rng.below(7)
        p = rng.range(1, 8)
        if fam == 0:
            s = f"random {n} {{}} {sigma} 0 0"
        elif fam == 1:
            s = f"period {n} {{}} {p} {sigma} 0"
        elif fam == 2:
            s = f"noisy {n} {{}} {p} {max(sigma, 2)} {rng.range(1, 3)}"
        elif fam == 3:
            s = f"fib {n} {{}} 0 0 0"
        elif fam == 4:
            s = f"thue {n} {{}} 0 0 0"
        elif fam == 5:
            s = f"carry {n} {{}} 0 0 0"
        else:
            s = f"blocks {n} {{}} {p} 0 0"
        rows.append((s, f"mix {q} {{}} {rng.range(1, 10)} {max(sigma, 2)}"))
    return rows


# seed -> ブロックの並び (テキストの作り方, パターンの作り方)。seed の欄 {} は後で埋める。
CASES = {
    1: [(f"random {N} {{}} 26 0 0", "mix 200000 {} 20 26")],
    2: [(f"random {N} {{}} 2 0 0", "mix 100000 {} 40 2")],
    3: [(f"period {N} {{}} 1 1 0", "mix 20000 {} 1000 1")],
    4: [(f"fib {N} {{}} 0 0 0", "mix 20000 {} 1000 2")],
    5: [(f"noisy {N} {{}} 1000 26 300", "mix 20000 {} 3000 26")],
    6: [(f"carry {N} {{}} 0 0 0", "mix 50000 {} 200 19")],
    7: [(f"random {N} {{}} 26 0 0", "mix 100 {} 20 26")],
    8: [("random 5000 {} 4 0 0", "mix 1000000 {} 12 4")],
    9: [("random 500 {} 26 0 0", "mix 200 {} 8 26")] * 1000,
}


def rows_for(seed: int) -> list[tuple[str, str]]:
    if seed == 0:
        return small_blocks(SplitMix64(20261107), 200, 60, 60)
    if seed in CASES:
        return CASES[seed]
    if seed >= 1000:
        rng = SplitMix64(seed * 1000003 + 23)
        return small_blocks(rng, rng.range(1, 3), 40, 30)
    raise SystemExit(f"seed {seed} は 0 から {max(CASES)} か、1000 以上にしてください")


def main() -> None:
    seed = int(sys.argv[1])
    rows = rows_for(seed)
    out = [str(len(rows))]
    for i, (s, q) in enumerate(rows):
        out.append(s.format(seed * 1_000_003 + 2 * i))
        out.append(q.format(seed * 1_000_003 + 2 * i + 1))
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
