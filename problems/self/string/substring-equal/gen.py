#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""部分文字列の一致判定 の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

入力は文字列とクエリの作り方だけで、文字列とクエリはハーネスが作る (base.cpp と _shared/string/_common.hpp)。
1 行目がブロックの数 T、続くブロックごとに「族 n seed x y z」と「mix q seed p」か「aligned q seed k」の 2 行。
seed 0 は小さい文字列を 200 個並べたもの、seed 1 から 10 は本番の大きさのケース。
seed 1000 以上は愚直解 (brute.hpp) と突き合わせる小さい入力で、族とクエリを splitmix64 で選ぶ。
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

    def choice(self, xs: list):
        return xs[self.below(len(xs))]


def small_blocks(rng: SplitMix64, count: int, max_n: int, q: int) -> list[tuple[str, str]]:
    """長さ 0 から max_n の文字列を count 個。族と周期とクエリの作り方を乱択で選ぶ。seed の欄は後で埋める。"""
    rows = []
    for _ in range(count):
        n = rng.range(0, max_n)
        fam = rng.below(7)
        p = rng.range(1, 8)
        if fam == 0:
            s = f"random {n} {{}} {rng.range(1, 3)} 0 0"
        elif fam == 1:
            s = f"period {n} {{}} {p} {rng.range(1, 3)} 0"
        elif fam == 2:
            s = f"noisy {n} {{}} {p} {rng.range(2, 3)} {rng.range(1, 3)}"
        elif fam == 3:
            s, p = f"fib {n} {{}} 0 0 0", rng.choice([1, 2, 3, 5, 8, 13, 21])
        elif fam == 4:
            s, p = f"thue {n} {{}} 0 0 0", 1 << rng.range(0, 4)
        elif fam == 5:
            s, p = f"carry {n} {{}} 0 0 0", 1 << rng.range(0, 4)
        else:
            s = f"blocks {n} {{}} {p} 0 0"
        if fam == 4 and rng.below(2):
            qs = f"aligned {q} {{}} {rng.range(0, 4)}"
        else:
            qs = f"mix {q} {{}} {p if rng.below(4) else 0}"
        rows.append((s, qs))
    return rows


# seed -> ブロックの並び (文字列の作り方, クエリの作り方)。seed の欄 {} は後で埋める。
CASES = {
    1: [(f"random {M} {{}} 26 0 0", f"mix {M} {{}} 0")],
    2: [(f"random {M} {{}} 2 0 0", f"mix {M} {{}} 0")],
    3: [(f"noisy {M} {{}} 1000 26 300", f"mix {M} {{}} 1000")],
    4: [(f"period {M} {{}} 1 1 0", f"mix {M} {{}} 1")],
    5: [(f"fib {M} {{}} 0 0 0", f"mix {M} {{}} 4181")],
    6: [(f"thue {M} {{}} 0 0 0", f"aligned {M} {{}} 12")],
    7: [(f"carry {M} {{}} 0 0 0", f"mix {M} {{}} 4096")],
    8: [(f"random 4096 {{}} 2 0 0", f"mix {M} {{}} 0")],
    9: [(f"random {M} {{}} 26 0 0", f"mix 1000 {{}} 0")],
    10: [(f"random 1000 {{}} 26 0 0", f"mix 1000 {{}} 0")] * 1000,
}


def blocks_for(seed: int) -> list[tuple[str, str]]:
    if seed == 0:
        return small_blocks(SplitMix64(20261107), 200, 100, 100)
    if seed in CASES:
        return CASES[seed]
    if seed >= 1000:
        rng = SplitMix64(seed * 1000003 + 11)
        return small_blocks(rng, rng.range(1, 3), 40, 60)
    raise SystemExit(f"seed {seed} は 0 から {max(CASES)} か、1000 以上にしてください")


def main() -> None:
    seed = int(sys.argv[1])
    rows = blocks_for(seed)
    out = [str(len(rows))]
    # ブロックごとの乱数の種は、ケースの seed と行の番号から決める。
    for i, (s, q) in enumerate(rows):
        out.append(s.format(seed * 1_000_003 + 2 * i))
        out.append(q.format(seed * 1_000_003 + 2 * i + 1))
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
