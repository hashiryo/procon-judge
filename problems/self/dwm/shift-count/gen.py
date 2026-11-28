#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""区間の巡回シフトと値の個数の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

1 行目に N Q、2 行目に初期の列、続く Q 行がクエリ。位置は 0-indexed の半開区間。
  0 l r    : [l, r) を右に 1 つ巡回シフト
  1 l r k  : [l, r) の k の個数
値は 1 以上 N 以下。seed 1000 以上は愚直解と突き合わせる小さい入力。
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


# seed -> (N, Q, シフトの重み, 個数の重み, 値の作り方, 区間の作り方)
# 値の作り方は "random" (1..N)、"one" (全部 1)、"max" (全部 N)。
# 区間の作り方は "random"、"full" (全体)、"short" (長さ 2)、"small" (長さ 100 以下)。
CASES = {
    0: (10, 30, 1, 1, "random", "random"),
    1: (1, 1000, 1, 1, "random", "full"),
    2: (100000, 100000, 1, 1, "random", "random"),
    3: (100000, 100000, 9, 1, "random", "random"),
    4: (100000, 100000, 1, 9, "random", "random"),
    5: (100000, 100000, 1, 1, "one", "random"),
    6: (100000, 100000, 7, 3, "random", "full"),
    7: (100000, 100000, 1, 1, "random", "short"),
    8: (100000, 100000, 1, 1, "random", "small"),
    9: (100000, 100000, 5, 5, "max", "random"),
}


def main() -> None:
    seed = int(sys.argv[1])
    r = SplitMix64(seed * 1000003 + 98765)
    if seed >= 1000:
        n, q = 1 + r.below(30), 1 + r.below(50)
        ws, wc = 1 + r.below(3), 1 + r.below(3)
        vmode, where = "random", ("random", "full", "short", "small")[r.below(4)]
    else:
        n, q, ws, wc, vmode, where = CASES[seed]
    wsum = ws + wc

    def value() -> int:
        if vmode == "one":
            return 1
        if vmode == "max":
            return n
        return 1 + r.below(n)

    def interval() -> tuple[int, int]:
        if where == "full":
            return 0, n
        if where == "short":
            if n < 2:
                return 0, n
            l = r.below(n - 1)
            return l, l + 2
        if where == "small":
            l = r.below(n)
            w = 1 + r.below(min(100, n - l))
            return l, l + w
        l = r.below(n)
        w = 1 + r.below(n - l)
        return l, l + w

    a = [value() for _ in range(n)]
    out = [f"{n} {q}", " ".join(map(str, a))]
    for _ in range(q):
        l, rgt = interval()
        if r.below(wsum) < ws and rgt - l >= 2:
            out.append(f"0 {l} {rgt}")
        else:
            k = value() if r.below(2) else 1 + r.below(n)
            out.append(f"1 {l} {rgt} {k}")
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
