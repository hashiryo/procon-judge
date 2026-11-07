#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""1 点代入と区間最小の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

1 行目に N Q、2 行目に a_0 ... a_{N-1}、続く Q 行が「0 p x」(a_p = x) か「1 l r」(min(a_l, ..., a_{r-1}) を出す、l < r)。
値は絶対値 V 以下の整数で、V は 10^18 か 10^9 か 10。seed 1000 以上は愚直解と突き合わせる小さい入力。
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


# seed -> (N, Q, 値の絶対値の上限 V, 取得の割合 (%), 区間の長さの上限 (0 なら制限なし), 初めの並び)
CASES = {
    0: (8, 12, 10, 50, 0, "random"),
    1: (500000, 500000, 10**18, 50, 0, "random"),
    2: (500000, 500000, 10**18, 50, 32, "random"),
    3: (500000, 500000, 10**18, 90, 0, "random"),
    4: (500000, 500000, 10**18, 10, 0, "random"),
    5: (500000, 500000, 10**9, 50, 0, "random"),
    6: (500000, 500000, 10**18, 50, 0, "descending"),
    7: (1 << 19, 500000, 10**18, 50, 0, "random"),
    8: (499999, 500000, 10**18, 50, 0, "random"),
    9: (1, 10, 10, 50, 0, "random"),
}


def main() -> None:
    seed = int(sys.argv[1])
    r = SplitMix64(seed * 1000003 + 12345)
    if seed >= 1000:
        n, q = 1 + r.below(40), 1 + r.below(60)
        v = [10, 10**9, 10**18][r.below(3)]
        get_pct, short, init = 50, 0, "random"
    else:
        n, q, v, get_pct, short, init = CASES[seed]

    def val() -> int:
        return r.below(2 * v + 1) - v

    if init == "descending":
        a = [v - i * (v // n) for i in range(n)]
    else:
        a = [val() for _ in range(n)]
    out = [f"{n} {q}", " ".join(map(str, a))]
    t = v
    for _ in range(q):
        if r.below(100) < get_pct:
            if short:
                k = 1 + r.below(min(short, n))
                lo = r.below(n - k + 1)
                out.append(f"1 {lo} {lo + k}")
            else:
                lo, hi = r.below(n + 1), r.below(n + 1)
                if lo > hi:
                    lo, hi = hi, lo
                if lo == hi:
                    hi = lo + 1 if lo < n else hi
                    lo = hi - 1
                out.append(f"1 {lo} {hi}")
        else:
            p = r.below(n)
            if init == "descending":
                t -= r.below(1000)
                x = max(t, -v)
            else:
                x = val()
            out.append(f"0 {p} {x}")
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
