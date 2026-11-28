#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""点更新と区間の大きい方から和が k 以上になる最小個数の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

1 行目に N Q、2 行目に初期の列、続く Q 行がクエリ。
  c x l r k  (1-indexed)
A_c を x に更新してから、[l, r] の要素を大きい方から選んで和が k 以上になる最小個数を出す。
値は 1 以上 10^9 以下、k は 1 以上 10^15 以下。seed 1000 以上は愚直解と突き合わせる小さい入力。
"""
import sys

MASK64 = (1 << 64) - 1
VMAX = 10**9
KMAX = 10**15


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


# seed -> (N, Q, 値の作り方, 問いの作り方)
# 値: "random" (1..10^9)、"one" (全部 1)、"max" (全部 10^9)。
# 問い: "random"、"k1" (k=1)、"neg" (和より大きい k が多い)、"ends" (区間を端に寄せる)。
CASES = {
    0: (7, 5, "sample", "sample"),
    1: (1, 1000, "random", "random"),
    2: (30000, 30000, "random", "random"),
    3: (30000, 30000, "random", "random"),
    4: (30000, 30000, "random", "random"),
    5: (30000, 30000, "one", "random"),
    6: (30000, 30000, "max", "random"),
    7: (30000, 30000, "random", "k1"),
    8: (30000, 30000, "random", "neg"),
    9: (30000, 30000, "random", "ends"),
}

SAMPLE_A = [8, 2, 4, 1, 7, 3, 6]
SAMPLE_Q = [
    (1, 1, 4, 7, 9),
    (5, 2, 1, 3, 8),
    (6, 4, 1, 5, 9),
    (6, 5, 3, 5, 1),
    (7, 9, 4, 6, 4),
]


def main() -> None:
    seed = int(sys.argv[1])
    if seed == 0:
        out = ["7 5", " ".join(map(str, SAMPLE_A))]
        out += [" ".join(map(str, q)) for q in SAMPLE_Q]
        sys.stdout.write("\n".join(out) + "\n")
        return
    r = SplitMix64(seed * 1000003 + 98765)
    if seed >= 1000:
        n, q = 1 + r.below(30), 1 + r.below(50)
        vmode, qmode = "random", ("random", "neg", "ends")[r.below(3)]
    else:
        n, q, vmode, qmode = CASES[seed]

    def value() -> int:
        if vmode == "one":
            return 1
        if vmode == "max":
            return VMAX
        return 1 + r.below(VMAX)

    a = [value() for _ in range(n)]
    bit = [0] * (n + 1)

    def add(i: int, delta: int) -> None:
        i += 1
        while i <= n:
            bit[i] += delta
            i += i & -i

    def pref(i: int) -> int:
        s = 0
        while i > 0:
            s += bit[i]
            i -= i & -i
        return s

    for i, x in enumerate(a):
        add(i, x)

    out = [f"{n} {q}", " ".join(map(str, a))]
    for _ in range(q):
        c = 1 + r.below(n)
        x = value()
        add(c - 1, x - a[c - 1])
        a[c - 1] = x
        if qmode == "ends" and r.below(2):
            if r.below(2):
                l, rgt = 1, 1 + r.below(n)
            else:
                l = 1 + r.below(n)
                rgt = n
        else:
            l = 1 + r.below(n)
            rgt = l + r.below(n - l + 1)
        total = pref(rgt) - pref(l - 1)
        if qmode == "k1":
            k = 1
        elif qmode == "neg" and r.below(2):
            k = total + 1 + r.below(10**12)
        else:
            k = 1 + r.below(total)
        k = min(k, KMAX)
        out.append(f"{c} {x} {l} {rgt} {k}")
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
