#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""区間に 1 次関数を足して区間最小を取る入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

形式は Library Checker の range_linear_add_range_min と同じで、1 行目に N Q、2 行目に a_0 ... a_{N-1}、続く Q 行が
「0 l r b c」(i in [l, r) に a_i += b i + c) か「1 l r」(min(a_l, ..., a_{r-1}) を出す)。どちらも l < r で、制約も同じ
(N, Q <= 10^5、|a_i| < 10^9、|b| < 100、|c| < 10^7)。

Library Checker のケースに無い、kinetic segment tree の時刻を行き来させる入力を入れる。a_i = -i に +2 i と -2 i を
交互に足すと、全部の直線が 1 点で交わる時刻をまたいで行き来するので、どの節でも最小を取る葉が毎回入れ替わる。
seed 1000 以上は愚直解と突き合わせる小さい入力。乱数は Python の random の版による違いを避けるため splitmix64 で作る。
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
#   random: Library Checker の max_random と同じ乱数
#   swing_all: a_i = -i に、全体へ +2 i と -2 i を交互に足す
#   swing_edge: a_i = -i に、両端を 50 以内で欠いた区間へ +2 (i - l) と -2 (i - l) を交互に足す
#   swing_rand: a_i = -i に、乱数の区間へ +2 (i - l) と -2 (i - l) を交互に足す
#   forward: b が正だけ (時刻が前にだけ進む)
#   small: 値も b も c も絶対値 3 以下 (同じ値と一直線に並ぶ点が多い)
CASES = {
    0: (8, 12, 50, "random"),
    1: (100000, 100000, 50, "random"),
    2: (100000, 100000, 10, "random"),
    3: (100000, 100000, 90, "random"),
    4: (100000, 100000, 25, "swing_all"),
    5: (100000, 100000, 25, "swing_edge"),
    6: (100000, 100000, 25, "swing_rand"),
    7: (100000, 100000, 50, "forward"),
    8: (100000, 100000, 50, "small"),
    9: (1, 10, 50, "random"),
}
KINDS = ["random", "swing_all", "swing_edge", "swing_rand", "forward", "small"]


def main() -> None:
    seed = int(sys.argv[1])
    r = SplitMix64(seed * 1000003 + 22222)
    if seed >= 1000:
        n, q = 1 + r.below(40), 1 + r.below(60)
        get_pct, kind = 50, KINDS[r.below(len(KINDS))]
    else:
        n, q, get_pct, kind = CASES[seed]
    swing = kind.startswith("swing")
    if swing:
        a = [-i for i in range(n)]
    elif kind == "small":
        a = [r.sym(4) for _ in range(n)]
    else:
        a = [r.sym(10**9) for _ in range(n)]
    out = [f"{n} {q}", " ".join(map(str, a))]

    def interval() -> tuple[int, int]:
        lo, hi = r.below(n + 1), r.below(n + 1)
        if lo > hi:
            lo, hi = hi, lo
        if lo == hi:
            hi = lo + 1 if lo < n else hi
            lo = hi - 1
        return lo, hi

    sign = 1
    for _ in range(q):
        if r.below(100) < get_pct:
            lo, hi = interval()
            out.append(f"1 {lo} {hi}")
            continue
        if kind == "swing_all":
            out.append(f"0 0 {n} {2 * sign} 0")
            sign = -sign
        elif kind in ("swing_edge", "swing_rand"):
            if kind == "swing_edge":
                lo, hi = r.below(min(50, n)), n - r.below(min(50, n))
            else:
                lo, hi = interval()
            out.append(f"0 {lo} {hi} {2 * sign} {-2 * sign * lo}")
            sign = -sign
        else:
            lo, hi = interval()
            if kind == "forward":
                b, c = 1 + r.below(99), r.sym(10**7)
            elif kind == "small":
                b, c = r.sym(4), r.sym(4)
            else:
                b, c = r.sym(100), r.sym(10**7)
            out.append(f"0 {lo} {hi} {b} {c}")
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
