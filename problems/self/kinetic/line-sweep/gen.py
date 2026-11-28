#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""直線の集合の最大を時刻を進めながら問う入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

形式は 1 行目に N Q、続く N 行が「a_i b_i」(直線 y = a_i t + b_i)、続く Q 行が「0 i a b」(直線 i を置き換える) か
「1 t」(時刻 t の最大。t は前に出した t 以上)。制約は N, Q <= 10^5、|a|, |b| <= 10^9、0 <= t <= 10^9。

cross_all は a_i = i - N/2、b_i = -a_i T0 で、全部の直線が時刻 T0 の 1 点で交わる。jump は時刻を 10^9 まで
大きく飛ばす。seed 1000 以上は愚直解と突き合わせる小さい入力。乱数は splitmix64 で作る。
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


# seed -> (N, Q, 置き換えの割合 (%), 種類)
CASES = {
    0: (8, 12, 30, "random"),
    1: (100000, 100000, 50, "random"),
    2: (100000, 100000, 5, "random"),
    3: (100000, 100000, 80, "random"),
    4: (100000, 100000, 0, "cross_all"),
    5: (100000, 100000, 0, "same_slope"),
    6: (100000, 100000, 0, "zero_slope"),
    7: (100000, 100000, 50, "jump"),
    8: (100000, 100000, 0, "static"),
    9: (1, 10, 50, "random"),
}
KINDS = ["random", "cross_all", "same_slope", "zero_slope", "jump", "static"]
LIMIT = 10**9


def main() -> None:
    seed = int(sys.argv[1])
    r = SplitMix64(seed * 1000003 + 77777)
    small = seed >= 1000
    if small:
        n, q = 1 + r.below(30), 1 + r.below(60)
        set_pct, kind = 40, KINDS[r.below(len(KINDS))]
        ma, mb = 20, 20
    else:
        n, q, set_pct, kind = CASES[seed]
        ma, mb = 10**9, 10**9

    t0 = 1 + r.below(max(1, min(1000, ma)))
    if kind == "cross_all":
        a = [i - n // 2 for i in range(n)]
        b = [-v * t0 for v in a]
    elif kind == "same_slope":
        c = r.sym(ma)
        a = [c for _ in range(n)]
        b = [r.sym(mb) for _ in range(n)]
    elif kind == "zero_slope":
        a = [0 for _ in range(n)]
        b = [r.sym(mb) for _ in range(n)]
    else:
        a = [r.sym(ma) for _ in range(n)]
        b = [r.sym(mb) for _ in range(n)]

    out = [f"{n} {q}"]
    out += [f"{a[i]} {b[i]}" for i in range(n)]

    now = 0
    top = LIMIT if not small else 10**6
    for _ in range(q):
        if r.below(100) >= set_pct:
            if kind == "jump":
                step = r.below(top)
            elif kind == "cross_all":
                step = 1 + r.below(50)
            elif small:
                step = r.below(3)
            else:
                step = r.below(1000)
            now = min(top if not small else LIMIT, now + step)
            out.append(f"1 {now}")
        else:
            i = r.below(n)
            out.append(f"0 {i} {r.sym(ma)} {r.sym(mb)}")
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
