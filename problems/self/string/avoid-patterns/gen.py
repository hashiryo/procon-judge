#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""禁止パターンを含まない文字列の数 の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

入力はパターンの作り方だけで、パターンはハーネスが作る (base.cpp)。1 行目がブロックの数 T、ブロックごとに「n σ G」と
G 行の作り方 (words、tree、sub、subx)。
seed 0 は小さいブロックを 200 個並べたもの、seed 1 から 12 は本番の大きさのケース。
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


def small_text(rng: SplitMix64, sigma: int, max_n: int) -> str:
    """σ 種類に収まるテキストの作り方の 1 行 (seed の欄は {})。"""
    n = rng.range(1, max_n)
    p = rng.range(1, 6)
    fams = ["random", "period", "noisy"]
    if sigma >= 2:
        fams += ["fib", "thue", "blocks", "carry"]
    fam = fams[rng.below(len(fams))]
    if fam == "random":
        return f"random {n} {{}} {sigma} 0 0"
    if fam == "period":
        return f"period {n} {{}} {p} {sigma} 0"
    if fam == "noisy":
        return f"noisy {n} {{}} {p} {sigma} {rng.range(1, 3)}"
    if fam == "blocks":
        return f"blocks {n} {{}} {p} 0 0"
    if fam == "carry":
        # 位置 i の文字は a + ctz(i + 1) なので、長さ 2^σ - 1 までなら σ 種類に収まる。
        return f"carry {min(n, (1 << sigma) - 1)} {{}} 0 0 0"
    return f"{fam} {n} {{}} 0 0 0"


def small_blocks(rng: SplitMix64, count: int, max_n: int, max_k: int, max_len: int) -> list[list[str]]:
    blocks = []
    for _ in range(count):
        n = rng.range(1, max_n)
        # 1 種類の文字や 1 文字のパターンが多いと答えがほとんど 0 になるので、どちらも 8 回に 1 回ほどにする。
        sigma = 1 if rng.below(8) == 0 else rng.range(2, 3)
        rows = [f"{n} {sigma} {{G}}"]
        for _ in range(rng.range(1, 3)):
            mode = ["words", "tree", "sub", "subx"][rng.below(4)]
            k = rng.range(1, max_k)
            lo = 1 if rng.below(8) == 0 else rng.range(2, max_len)
            hi = rng.range(lo, max_len)
            row = f"{mode} {k} {{}} {lo} {hi}"
            if mode in ("sub", "subx"):
                row += " " + small_text(rng, sigma, 30)
            rows.append(row)
        blocks.append(rows)
    return blocks


N = 10**5

# seed -> ブロックの並び。ブロックは「n σ G」と G 行の作り方。seed の欄 {} は後で埋め、{G} は行の数で埋める。
CASES = {
    # 一様な単語の大きい辞書 (根に近い頂点が詰まり、遷移の先が文字ごとに違う)
    1: [["100 26 {G}", "words 20000 {} 4 8"]],
    # 2 種類の文字の長めの単語
    2: [["2000 2 {G}", "words 2000 {} 16 24"]],
    # 数字の列で、yuki-1269 (I hate Fibonacci Number) くらいの大きさ
    3: [["5000 10 {G}", "words 100 {} 10 19"]],
    # 接頭辞を共有するパターン (トライが深い)
    4: [["300 26 {G}", "tree 3000 {} 20 60"]],
    # フィボナッチ文字列の部分文字列
    5: [["2000 2 {G}", f"sub 50 {{}} 100 1000 fib {N} {{}} 0 0 0"]],
    # Thue–Morse 列の長い部分文字列 1 本 (KMP のオートマトン)
    6: [["10000 2 {G}", f"sub 1 {{}} 5000 5000 thue {N} {{}} 0 0 0"]],
    # carry の列の部分文字列 (17 種類の文字が出る。文字ごとの遷移の先が重なりやすい)
    7: [["600 20 {G}", f"sub 200 {{}} 30 300 carry {N} {{}} 0 0 0"]],
    # 周期 1000 の列の 300 か所を替えたものの部分文字列の、1 文字を替えたもの
    8: [["200 26 {G}", f"subx 5000 {{}} 10 40 noisy {N} {{}} 1000 26 300"]],
    # 1 文字のパターンで文字の半分ほどを禁じ、残りに短い単語
    9: [["1000 26 {G}", "words 10 {} 1 1", "words 5000 {} 3 6"]],
    # 小さい辞書を 1000 個 (構築の手間)
    10: [["50 26 {G}", "words 30 {} 1 8"]] * 1000,
    # a だけのパターン (suffix link が 1 本の鎖)
    11: [["20000 2 {G}", "sub 200 {} 500 2000 period 2000 {} 1 1 0"]],
    # 文字の合計が 10^6 ほどの辞書で短い n (構築が大半)
    12: [["20 26 {G}", "words 50000 {} 10 30"]],
}


def blocks_for(seed: int) -> list[list[str]]:
    if seed == 0:
        return small_blocks(SplitMix64(20261107), 200, 20, 8, 8)
    if seed in CASES:
        return CASES[seed]
    if seed >= 1000:
        rng = SplitMix64(seed * 1000003 + 31)
        return small_blocks(rng, rng.range(1, 3), 40, 6, 6)
    raise SystemExit(f"seed {seed} は 0 から {max(CASES)} か、1000 以上にしてください")


def main() -> None:
    seed = int(sys.argv[1])
    blocks = blocks_for(seed)
    out = [str(len(blocks))]
    ctr = 0
    for rows in blocks:
        out.append(rows[0].replace("{G}", str(len(rows) - 1)))
        for row in rows[1:]:
            parts = row.split("{}")
            line = parts[0]
            for part in parts[1:]:
                line += str(seed * 1_000_003 + ctr) + part
                ctr += 1
            out.append(line)
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
