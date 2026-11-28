#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""要素ごとの係数の区間線形加算と区間最大の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

形式は 1 行目に N Q、2 行目に x_0 ... x_{N-1}、3 行目に y_0 ... y_{N-1}、続く Q 行が
「0 l r a b」(i in [l, r) に y_i += a x_i + b。0 <= a) か「1 l r」(max(y_l, ..., y_{r-1})) か
「2 i x y」((x_i, y_i) = (x, y))。制約は N, Q <= 10^5、|x| <= 10^6、|y| <= 10^9、0 <= a <= 10^6、|b| <= 10^9。

交差を集中させた入力を入れる。cross_all は x_i = i - N/2、y_i = -x_i T0 で、全部の直線が時刻 T0 の 1 点で
交わる。a を T0 の前後へ動かすと、どの節でも最大を取る葉が入れ替わる。jump は a を 0 と 10^6 の間で大きく飛ばす。
set_mix は点の更新を混ぜる。seed 1000 以上は愚直解と突き合わせる小さい入力。乱数は Python の random の版に
よる違いを避けるため splitmix64 で作る。
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

    def sym(self, m: int) -> int:  # (-m, m) の一様乱数
        return self.below(2 * m - 1) - (m - 1)


# seed -> (N, Q, 取得の割合 (%), 種類)
CASES = {
    0: (8, 12, 50, "random"),
    1: (100000, 100000, 50, "random"),
    2: (100000, 100000, 90, "random"),
    3: (100000, 100000, 10, "random"),
    4: (100000, 100000, 30, "cross_all"),
    5: (100000, 100000, 50, "same_x"),
    6: (100000, 100000, 50, "zero_x"),
    7: (100000, 100000, 50, "jump"),
    8: (100000, 100000, 30, "set_mix"),
    9: (1, 10, 50, "random"),
}
KINDS = ["random", "cross_all", "same_x", "zero_x", "jump", "set_mix"]


def main() -> None:
    seed = int(sys.argv[1])
    r = SplitMix64(seed * 1000003 + 33333)
    small = seed >= 1000
    if small:
        n, q = 1 + r.below(30), 1 + r.below(60)
        get_pct, kind = 50, KINDS[r.below(len(KINDS))]
        mx, my, ma, mb = 20, 50, 5, 20
    else:
        n, q, get_pct, kind = CASES[seed]
        mx, my, ma, mb = 10**6, 10**9, 10**6, 10**9

    t0 = 1 + r.below(max(1, min(1000, mx)))
    if kind == "cross_all":
        x = [i - n // 2 for i in range(n)]
        y = [-v * t0 for v in x]
    elif kind == "same_x":
        c = r.sym(mx)
        x = [c for _ in range(n)]
        y = [r.sym(my) for _ in range(n)]
    elif kind == "zero_x":
        x = [0 for _ in range(n)]
        y = [r.sym(my) for _ in range(n)]
    else:
        x = [r.sym(mx) for _ in range(n)]
        y = [r.sym(my) for _ in range(n)]

    out = [f"{n} {q}", " ".join(map(str, x)), " ".join(map(str, y))]

    def interval() -> tuple[int, int]:
        lo, hi = r.below(n + 1), r.below(n + 1)
        if lo > hi:
            lo, hi = hi, lo
        if lo == hi:
            hi = lo + 1 if lo < n else hi
            lo = hi - 1
        return lo, hi

    for _ in range(q):
        u = r.below(100)
        if u < get_pct:
            lo, hi = interval()
            out.append(f"1 {lo} {hi}")
            continue
        if kind == "set_mix" and r.below(100) < 40:
            i = r.below(n)
            out.append(f"2 {i} {r.sym(mx)} {r.sym(my)}")
            continue
        lo, hi = interval()
        if kind == "cross_all":
            a = 1 + r.below(t0) if r.below(4) else t0 + r.below(t0)
        elif kind == "jump":
            a = 0 if r.below(2) else ma
        else:
            a = r.below(ma + 1)
        out.append(f"0 {lo} {hi} {a} {r.sym(mb)}")
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
