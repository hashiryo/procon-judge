#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""区間代入と区間和の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

1 行目に N Q、2 行目に a_0 ... a_{N-1}、続く Q 行が「0 l r x」(a_l, ..., a_{r-1} を x にする) か「1 l r」(a_l + ... + a_{r-1} を出す)。
どちらも l < r。値は初めも代入も絶対値 10^9 以下なので、和は i64 に収まる。seed 1000 以上は愚直解と
突き合わせる小さい入力。
乱数は Python の random の版による違いを避けるため splitmix64 で作る。
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


# seed -> (N, Q, 取得の割合 (%), 区間の長さの上限 (0 なら制限なし), 全体を取るか)
CASES = {
    0: (8, 12, 50, 0, False),
    1: (500000, 500000, 50, 0, False),
    2: (500000, 500000, 90, 0, False),
    3: (500000, 500000, 10, 0, False),
    4: (500000, 500000, 50, 32, False),
    5: (1 << 19, 500000, 50, 0, False),
    6: (499999, 500000, 50, 0, False),
    7: (500000, 500000, 50, 0, True),
    8: (200000, 500000, 50, 0, False),
    9: (1, 10, 50, 0, False),
}


def main() -> None:
    seed = int(sys.argv[1])
    r = SplitMix64(seed * 1000003 + 22222)
    if seed >= 1000:
        n, q = 1 + r.below(40), 1 + r.below(60)
        get_pct, short, whole = 50, 0, False
    else:
        n, q, get_pct, short, whole = CASES[seed]
    a = [r.below(2 * 10**9 + 1) - 10**9 for _ in range(n)]
    out = [f"{n} {q}", " ".join(map(str, a))]

    def interval() -> tuple[int, int]:
        if whole:
            return 0, n
        if short:
            k = 1 + r.below(min(short, n))
            lo = r.below(n - k + 1)
            return lo, lo + k
        lo, hi = r.below(n + 1), r.below(n + 1)
        if lo > hi:
            lo, hi = hi, lo
        if lo == hi:
            hi = lo + 1 if lo < n else hi
            lo = hi - 1
        return lo, hi

    for _ in range(q):
        lo, hi = interval()
        if r.below(100) < get_pct:
            out.append(f"1 {lo} {hi}")
        else:
            out.append(f"0 {lo} {hi} {r.below(2 * 10**9 + 1) - 10**9}")
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
