#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""2×2 行列の列への挿入、削除、反転と区間の積の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

1 行目に N Q、続く N 行に初めの列の行列 (a b c d で [[a, b], [c, d]])、続く Q 行が操作。成分は 998244353 未満。
  0 i a b c d   位置 i (0 <= i <= 長さ) に行列を挿む
  1 i           位置 i (0 <= i < 長さ) を取り除く
  2 l r         [l, r) を反転 (0 <= l < r <= 長さ)
  3 l r         a[l] a[l+1] ... a[r-1] を出す (0 <= l < r <= 長さ)
列が空のときは挿入にする。seed 1000 以上は愚直解と突き合わせる小さい入力。
乱数は Python の random の版による違いを避けるため splitmix64 で作る。
"""
import sys

MASK64 = (1 << 64) - 1
MOD = 998244353


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


# seed -> (N, Q, 操作の重み (挿入, 削除, 反転, 積), 区間の長さの上限 (0 なら制限なし), 挿入の位置 ("random" か "hot"))
CASES = {
    0: (4, 10, (1, 1, 1, 2), 0, "random"),
    1: (200000, 200000, (15, 15, 35, 35), 0, "random"),
    2: (200000, 200000, (0, 0, 80, 20), 0, "random"),
    3: (200000, 200000, (0, 0, 10, 90), 0, "random"),
    4: (200000, 200000, (40, 40, 10, 10), 0, "random"),
    5: (200000, 200000, (15, 15, 35, 35), 16, "random"),
    6: (1, 200000, (60, 5, 15, 20), 0, "random"),
    7: (1, 200000, (60, 5, 15, 20), 0, "hot"),
    8: (200000, 200000, (0, 0, 50, 50), 0, "random"),
}


def main() -> None:
    seed = int(sys.argv[1])
    r = SplitMix64(seed * 1000003 + 98765)
    if seed >= 1000:
        n, q = r.below(30), 1 + r.below(80)
        weights = tuple(1 + r.below(4) for _ in range(4))
        short, where = (0, 4)[r.below(2)], ("random", "hot")[r.below(2)]
    else:
        n, q, weights, short, where = CASES[seed]

    def mat() -> str:
        return " ".join(str(r.below(MOD)) for _ in range(4))

    def interval(length: int, nonempty: bool) -> tuple[int, int]:
        if short:
            k = r.below(min(short, length) + 1)
            if nonempty and k == 0:
                k = 1
            lo = r.below(length - k + 1)
            return lo, lo + k
        lo, hi = r.below(length + 1), r.below(length + 1)
        if lo > hi:
            lo, hi = hi, lo
        if nonempty and lo == hi:
            if hi < length:
                hi += 1
            else:
                lo -= 1
        return lo, hi

    out = [f"{n} {q}"]
    out.extend(mat() for _ in range(n))
    length = n
    hot = 0
    total = sum(weights)
    for _ in range(q):
        x = r.below(total)
        ty = 0
        while x >= weights[ty]:
            x -= weights[ty]
            ty += 1
        if length == 0:
            ty = 0
        if ty == 0:
            if where == "hot":
                i = min(hot, length)
                if r.below(8) == 0:
                    hot = r.below(length + 1)
            else:
                i = r.below(length + 1)
            out.append(f"0 {i} {mat()}")
            length += 1
        elif ty == 1:
            out.append(f"1 {r.below(length)}")
            length -= 1
        elif ty == 2:
            lo, hi = interval(length, True)
            out.append(f"2 {lo} {hi}")
        else:
            lo, hi = interval(length, True)
            out.append(f"3 {lo} {hi}")
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
