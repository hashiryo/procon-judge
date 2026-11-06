#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""パターンの出現位置 の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

入力はテキストとパターンの作り方だけで、文字列はハーネスが作る (base.cpp と _shared/string/_common.hpp)。
1 行目がブロックの数 T、続くブロックごとに「族 n seed x y z」と、パターンの「族 m seed x y z」か「sub pos len」か
「subx pos len」の 2 行。seed 0 は小さいテキストを 200 個並べたもの、seed 1 から 11 は本番の大きさのケース。
seed 1000 以上は愚直解 (brute.hpp) と突き合わせる小さい入力で、族とパターンを splitmix64 で選ぶ。
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


def small_text(rng: SplitMix64, n: int, sigma: int) -> str:
    fam = rng.below(7)
    p = rng.range(1, 8)
    if fam == 0:
        return f"random {n} {{}} {sigma} 0 0"
    if fam == 1:
        return f"period {n} {{}} {p} {sigma} 0"
    if fam == 2:
        return f"noisy {n} {{}} {p} {max(sigma, 2)} {rng.range(1, 3)}"
    if fam == 3:
        return f"fib {n} {{}} 0 0 0"
    if fam == 4:
        return f"thue {n} {{}} 0 0 0"
    if fam == 5:
        return f"carry {n} {{}} 0 0 0"
    return f"blocks {n} {{}} {p} 0 0"


def small_blocks(rng: SplitMix64, count: int, max_n: int, max_m: int) -> list[tuple[str, str]]:
    """長さ 1 から max_n のテキストと、長さ 1 から max_m のパターンを count 組。seed の欄は後で埋める。"""
    rows = []
    for _ in range(count):
        n = rng.range(1, max_n)
        sigma = rng.range(1, 3)
        m = rng.range(1, min(n, max_m))
        kind = rng.below(4)
        if kind < 2:
            pat = f"{'sub' if kind == 0 else 'subx'} {rng.range(0, n - m)} {m}"
        elif kind == 2:
            pat = f"random {m} {{}} {sigma} 0 0"
        else:
            pat = small_text(rng, m, sigma)
        rows.append((small_text(rng, n, sigma), pat))
    return rows


# seed -> ブロックの並び (テキストの作り方, パターンの作り方)。seed の欄 {} は後で埋める。
CASES = {
    1: [(f"random {M} {{}} 26 0 0", "sub 123456 8")],
    2: [(f"random {M} {{}} 2 0 0", "sub 500000 20")],
    3: [(f"random {M} {{}} 2 0 0", "sub 777 4")],
    4: [(f"random {M} {{}} 26 0 0", "sub 200000 100000")],
    5: [(f"period {M} {{}} 1 1 0", "period 1000 {} 1 1 0")],
    6: [(f"period {M} {{}} 1 1 0", "blocks 1000 {} 1000 0 0")],
    7: [(f"fib {M} {{}} 0 0 0", "sub 0 10000")],
    8: [(f"noisy {M} {{}} 1000 26 300", "sub 5000 3000")],
    9: [(f"thue {M} {{}} 0 0 0", "sub 0 4096")],
    10: [(f"random {M} {{}} 4 0 0", "subx 314159 100")],
}


def rows_for(seed: int) -> list[tuple[str, str]]:
    if seed == 0:
        return small_blocks(SplitMix64(20261107), 200, 100, 12)
    if seed == 11:
        rng = SplitMix64(20261108)
        return [("random 1000 {} 2 0 0", f"sub {rng.range(0, 997)} 3") for _ in range(1000)]
    if seed in CASES:
        return CASES[seed]
    if seed >= 1000:
        rng = SplitMix64(seed * 1000003 + 17)
        return small_blocks(rng, rng.range(1, 3), 40, 8)
    raise SystemExit(f"seed {seed} は 0 から 11 か、1000 以上にしてください")


def main() -> None:
    seed = int(sys.argv[1])
    rows = rows_for(seed)
    out = [str(len(rows))]
    for i, (t, p) in enumerate(rows):
        out.append(t.format(seed * 1_000_003 + 2 * i))
        out.append(p.format(seed * 1_000_003 + 2 * i + 1))
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
