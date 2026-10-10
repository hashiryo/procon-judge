#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""典型 90 問 077 (Planes on a 2D Plane) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

飛行機 i は時刻 0 に A_i にいて、向き d の飛行機は時刻 T に A_i + T * DIRS[d] にいる。B は時刻 T に飛行機がいた N 点。
A どうし、B どうしはそれぞれ異なり、座標は 0 以上 10^9 以下。B の並びは乱択に混ぜる。
seed 0 から 13 は本番のケース。seed 1000 以上は、向きの組を全部試す愚直解 (brute.cpp) で解ける N ≤ 6 の入力で、
族と大きさを splitmix64 で選ぶ。
"""
import sys

MASK64 = (1 << 64) - 1
CMAX = 10**9
NMAX = 20000
DIRS = [(1, 0), (1, 1), (0, 1), (-1, 1), (-1, 0), (-1, -1), (0, -1), (1, -1)]


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


def inside(p: tuple[int, int]) -> bool:
    return 0 <= p[0] <= CMAX and 0 <= p[1] <= CMAX


def move(p: tuple[int, int], t: int, d: int) -> tuple[int, int]:
    return (p[0] + t * DIRS[d][0], p[1] + t * DIRS[d][1])


def fmt(t: int, a: list, b: list, r: SplitMix64) -> str:
    b = list(b)
    r.shuffle(b)
    lines = [f"{len(a)} {t}"] + [f"{x} {y}" for x, y in a] + [f"{x} {y}" for x, y in b]
    return "\n".join(lines) + "\n"


def random_points(r: SplitMix64, n: int, lo: int, hi: int) -> list:
    s = set()
    while len(s) < n:
        s.add((r.range(lo, hi), r.range(lo, hi)))
    return list(s)


def assign(r: SplitMix64, a: list, t: int, tries: int = 30) -> list:
    """飛行機ごとに乱択の向きを選び、時刻 T の位置が重ならず範囲に収まるようにする。選べなかった飛行機は外す。"""
    used = set()
    keep_a, b = [], []
    for p in a:
        for _ in range(tries):
            q = move(p, t, r.range(0, 7))
            if inside(q) and q not in used:
                used.add(q)
                keep_a.append(p)
                b.append(q)
                break
    return keep_a, b


def lattice(w: int, h: int, t: int, ox: int, oy: int) -> list:
    return [(ox + t * x, oy + t * y) for x in range(w) for y in range(h)]


def unreachable(r: SplitMix64, a: list, b: list, t: int) -> tuple[int, int]:
    """どの飛行機からも届かず、B のどれとも違う点。"""
    reach = {move(p, t, d) for p in a for d in range(8)}
    taken = set(b)
    while True:
        q = (r.range(0, CMAX), r.range(0, CMAX))
        if q not in reach and q not in taken:
            return q


def case(seed: int) -> str:
    r = SplitMix64(seed * 1000003 + 77)
    if seed == 0:
        return "3 2\n3 3\n5 5\n9 2\n11 2\n5 5\n3 3\n"
    if seed == 1:
        return "3 2\n3 3\n5 5\n9 2\n11 1000000000\n5 5\n3 3\n"
    if seed == 2:  # 乱択で Yes。T が小さく、行き先の候補が重ならない
        t = r.range(1, 1000)
        a, b = assign(r, random_points(r, NMAX, t, CMAX - t), t)
        return fmt(t, a, b, r)
    if seed == 3:  # 2 の B を 1 つだけ届かない点に替えて No
        t = r.range(1, 1000)
        a, b = assign(r, random_points(r, NMAX, t, CMAX - t), t)
        b[r.range(0, len(b) - 1)] = unreachable(r, a, b, t)
        return fmt(t, a, b, r)
    if seed == 4:  # 格子の全点から格子の全点へ (王の動きの完全マッチング)。どの飛行機にも候補が 8 つまである
        w, h, t = 141, 141, r.range(1, 1000)
        a = lattice(w, h, t, t * 3, t * 5)
        return fmt(t, a, a, r)
    if seed == 5:  # 4 の B の 1 点を届かない点に替えて No
        w, h, t = 141, 141, r.range(1, 1000)
        a = lattice(w, h, t, t * 3, t * 5)
        b = list(a)
        b[r.range(0, len(b) - 1)] = unreachable(r, a, b, t)
        return fmt(t, a, b, r)
    if seed == 6:  # 格子の穴あきの点から、乱択の向きで動かした先へ。候補が多く、組み方が多い
        t = r.range(1, 1000)
        pts = lattice(200, 200, t, t, t)
        r.shuffle(pts)
        a, b = assign(r, pts[:NMAX], t)
        return fmt(t, a, b, r)
    if seed == 7:  # y = 0 の直線上で、全員が右へ 1 つずれる (答えは 1 通り)
        t = r.range(1, 1000)
        a = [(t * i, 0) for i in range(NMAX)]
        b = [(t * (i + 1), 0) for i in range(NMAX)]
        return fmt(t, a, b, r)
    if seed == 8:  # y = 0 の直線上で、2 つおきの点から隙間をずらして詰める No (Hall の条件が真ん中で崩れる)
        t = r.range(1, 1000)
        a = [(t * i, 0) for i in range(NMAX)]
        b = [(t * (i + 1), 0) for i in range(NMAX // 2)] + [(t * (i + 2), 0) for i in range(NMAX // 2, NMAX)]
        return fmt(t, a, b, r)
    if seed == 9:  # T が大きく、点が 3 x 3 の格子に乗る
        t = CMAX // 2
        pts = lattice(3, 3, t, 0, 0)
        r.shuffle(pts)
        a, b = assign(r, pts, t)
        return fmt(t, a, b, r)
    if seed == 10:  # N = 1
        return "1 1000000000\n0 0\n1000000000 1000000000\n"
    if seed == 11:  # 小さい格子の塊をいくつも置く。塊ごとに王の動きの完全マッチング
        t = r.range(1, 100)
        cells = (CMAX - 10 * t) // (100 * t)
        a, used = [], set()
        while len(a) + 64 <= NMAX:
            o = (r.range(0, cells) * 100 * t, r.range(0, cells) * 100 * t)
            if o in used:
                continue
            used.add(o)
            a += lattice(r.range(2, 8), r.range(2, 8), t, o[0], o[1])
        return fmt(t, a, a, r)
    if seed == 12:  # 斜めの直線上で、全員が斜めに 1 つずれる
        t = r.range(1, 1000)
        a = [(t * i, t * i) for i in range(NMAX)]
        b = [(t * (i + 1), t * (i + 1)) for i in range(NMAX)]
        return fmt(t, a, b, r)
    if seed == 13:  # 格子で、飛行機が全点、B が全点から 1 点を除いて外の 1 点を足したもの。外の点は端の点からだけ届く
        w, h, t = 141, 141, r.range(1, 1000)
        a = lattice(w, h, t, t * 3, t * 5)
        b = list(a)
        b.remove(a[0])
        b.append((a[0][0] - t, a[0][1] - t))
        return fmt(t, a, b, r)
    # seed 1000 以上: 愚直解で解ける小さい入力
    n = r.range(1, 6)
    t = r.range(1, 3)
    kind = r.range(0, 2)
    if kind == 0:  # 小さい格子の上で、行き先が重なりやすい
        pts = lattice(4, 4, t, t, t)
        r.shuffle(pts)
        a = pts[:n]
        cand = list({move(p, t, d) for p in a for d in range(8)} | set(lattice(4, 4, t, t, t)))
        r.shuffle(cand)
        b = cand[:n]
    elif kind == 1:  # 動かした先 (Yes になりやすい)
        pts = lattice(4, 4, t, t, t)
        r.shuffle(pts)
        a, b = assign(r, pts[:n], t)
    else:  # 乱択の点
        a = random_points(r, n, 0, 6)
        b = random_points(r, len(a), 0, 6)
    return fmt(t, a, b, r)


def main() -> None:
    seed = int(sys.argv[1])
    if not (0 <= seed <= 13 or seed >= 1000):
        raise SystemExit(f"seed {seed} は 0 から 13 か、1000 以上にしてください")
    sys.stdout.write(case(seed))


if __name__ == "__main__":
    main()
