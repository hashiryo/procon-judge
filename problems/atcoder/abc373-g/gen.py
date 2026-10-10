#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""ABC 373 G (No Cross Matching) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

2N 個の点はどれも異なり、座標は 0 以上 5000 以下で、同じ直線の上に 3 点が並ばない。点を 1 つ足すたびに、既にある点への
向き (gcd で約した差分) が重ならないことを確かめて、並ぶ 3 点を除く。
seed 0 から 12 は本番のケース。seed 1000 以上は、順列を全部試す愚直解 (brute.cpp) で解ける N ≤ 7 の入力。
"""
import math
import sys

MASK64 = (1 << 64) - 1
CMAX = 5000


class SplitMix64:
    def __init__(self, seed: int) -> None:
        self.x = seed & MASK64

    def next(self) -> int:
        self.x = (self.x + 0x9E3779B97F4A7C15) & MASK64
        z = self.x
        z = ((z ^ (z >> 30)) * 0xBF58476D1CE4E5B9) & MASK64
        z = ((z ^ (z >> 27)) * 0x94D049BB133111EB) & MASK64
        return z ^ (z >> 31)

    def range(self, lo: int, hi: int) -> int:
        return lo + (self.next() * (hi - lo + 1) >> 64)

    def shuffle(self, xs: list) -> None:
        for i in range(len(xs) - 1, 0, -1):
            j = self.range(0, i)
            xs[i], xs[j] = xs[j], xs[i]


def direction(p: tuple[int, int], q: tuple[int, int]) -> tuple[int, int]:
    dx, dy = q[0] - p[0], q[1] - p[1]
    g = math.gcd(dx, dy)
    dx, dy = dx // g, dy // g
    return (dx, dy) if dx > 0 or (dx == 0 and dy > 0) else (-dx, -dy)


class Points:
    """同じ直線に 3 点が並ばない点の集まり。"""

    def __init__(self) -> None:
        self.pts: list[tuple[int, int]] = []
        self.seen: set[tuple[int, int]] = set()

    def try_add(self, p: tuple[int, int]) -> bool:
        if p in self.seen or not (0 <= p[0] <= CMAX and 0 <= p[1] <= CMAX):
            return False
        dirs = set()
        for q in self.pts:
            d = direction(p, q)
            if d in dirs:
                return False
            dirs.add(d)
        self.pts.append(p)
        self.seen.add(p)
        return True


def fill(r: SplitMix64, pts: Points, k: int, sample) -> list:
    """sample() で作った点を、足せるものだけ足して k 個集める。"""
    out = []
    while len(out) < k:
        p = sample()
        if pts.try_add(p):
            out.append(p)
    return out


def fmt(ps: list, qs: list, r: SplitMix64) -> str:
    ps, qs = list(ps), list(qs)
    r.shuffle(ps), r.shuffle(qs)
    lines = [str(len(ps))] + [f"{x} {y}" for x, y in ps] + [f"{x} {y}" for x, y in qs]
    return "\n".join(lines) + "\n"


def convex_polygon(k: int) -> list:
    """頂点が k 個 (4 の倍数) の真に凸な多角形。向きの違う原始ベクトルを偏角の順につなぐ。"""
    vs = sorted(((a, b) for a in range(1, 60) for b in range(0, 60) if math.gcd(a, b) == 1), key=lambda v: (v[0] + v[1], v))
    vs = sorted(vs[: k // 4], key=lambda v: math.atan2(v[1], v[0]))
    edges = []
    for rot in range(4):
        for a, b in vs:
            for _ in range(rot):
                a, b = -b, a
            edges.append((a, b))
    edges.sort(key=lambda v: math.atan2(v[1], v[0]))
    x = y = 0
    poly = []
    for a, b in edges:
        poly.append((x, y))
        x, y = x + a, y + b
    mx, my = min(p[0] for p in poly), min(p[1] for p in poly)
    poly = [(p[0] - mx, p[1] - my) for p in poly]
    assert max(max(p) for p in poly) <= CMAX
    return poly


def case(seed: int) -> str:
    r = SplitMix64(seed * 1000003 + 373)
    n = 300
    if seed == 0:
        return "3\n0 0\n2 4\n4 2\n0 2\n2 0\n4 4\n"
    if seed == 1:
        return "8\n59 85\n60 57\n72 12\n3 27\n16 58\n41 94\n77 64\n97 20\n32 37\n7 2\n57 94\n35 70\n38 60\n97 100\n5 76\n38 8\n"
    if seed == 2:  # 一様乱択
        pts = Points()
        u = lambda: (r.range(0, CMAX), r.range(0, CMAX))
        return fmt(fill(r, pts, n, u), fill(r, pts, n, u), r)
    if seed == 3:  # P が左半分、Q が右半分
        pts = Points()
        ps = fill(r, pts, n, lambda: (r.range(0, CMAX // 2), r.range(0, CMAX)))
        qs = fill(r, pts, n, lambda: (r.range(CMAX // 2 + 1, CMAX), r.range(0, CMAX)))
        return fmt(ps, qs, r)
    if seed == 4:  # 凸多角形の頂点に P と Q を交互に置く
        poly = convex_polygon(2 * n)
        return fmt(poly[0::2], poly[1::2], r)
    if seed == 5:  # 凸多角形の頂点に、P と Q を長さの揃わない塊で置く (数は釣り合う)
        poly = convex_polygon(2 * n)
        colors = [0] * n + [1] * n
        r.shuffle(colors)
        return fmt([p for p, c in zip(poly, colors) if c == 0], [p for p, c in zip(poly, colors) if c == 1], r)
    if seed == 6:  # P は中央の小さな塊、Q は全体に散らす
        pts = Points()
        ps = fill(r, pts, n, lambda: (r.range(2300, 2700), r.range(2300, 2700)))
        qs = fill(r, pts, n, lambda: (r.range(0, CMAX), r.range(0, CMAX)))
        return fmt(ps, qs, r)
    if seed == 7:  # 横長の帯 (高さ 300) に全部の点を置く
        pts = Points()
        both = fill(r, pts, 2 * n, lambda: (r.range(0, CMAX), r.range(2350, 2650)))
        r.shuffle(both)
        return fmt(both[:n], both[n:], r)
    if seed == 8:  # N = 1
        return "1\n0 0\n5000 5000\n"
    if seed == 9:  # N = 2
        pts = Points()
        u = lambda: (r.range(0, CMAX), r.range(0, CMAX))
        return fmt(fill(r, pts, 2, u), fill(r, pts, 2, u), r)
    if seed == 10:  # 小さな箱 (0 から 1000) の一様乱択
        pts = Points()
        u = lambda: (r.range(0, 1000), r.range(0, 1000))
        return fmt(fill(r, pts, n, u), fill(r, pts, n, u), r)
    if seed == 11:  # P は凸多角形の頂点、Q はその内側
        poly = convex_polygon(2 * n)
        r.shuffle(poly)
        pts = Points()
        ps = []
        for p in poly:
            if len(ps) < n and pts.try_add(p):
                ps.append(p)
        w = max(p[0] for p in poly)
        qs = fill(r, pts, n, lambda: (r.range(w // 4, 3 * w // 4), r.range(w // 4, 3 * w // 4)))
        return fmt(ps, qs, r)
    if seed == 12:  # P は下の帯、Q は上の帯 (どちらも高さ 300)。長い線分がほぼ平行に並ぶ
        pts = Points()
        ps = fill(r, pts, n, lambda: (r.range(0, CMAX), r.range(0, 300)))
        qs = fill(r, pts, n, lambda: (r.range(0, CMAX), r.range(CMAX - 300, CMAX)))
        return fmt(ps, qs, r)
    # seed 1000 以上: 愚直解で解ける小さい入力
    k = r.range(1, 7)
    box = [10, 100, CMAX][r.range(0, 2)]
    pts = Points()
    u = lambda: (r.range(0, box), r.range(0, box))
    return fmt(fill(r, pts, k, u), fill(r, pts, k, u), r)


def main() -> None:
    seed = int(sys.argv[1])
    if not (0 <= seed <= 12 or seed >= 1000):
        raise SystemExit(f"seed {seed} は 0 から 12 か、1000 以上にしてください")
    sys.stdout.write(case(seed))


if __name__ == "__main__":
    main()
