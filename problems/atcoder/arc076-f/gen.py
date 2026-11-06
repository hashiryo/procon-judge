#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""ARC 076 F (Exhausted?) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

1 ≤ N, M ≤ 2 × 10^5、0 ≤ L_i < R_i ≤ M + 1。seed 0 から 11 は本番のケースで、族と大きさは CASES の表のとおり。
人が椅子より多いケース (seed 1、3、4、7) は、最大流を push-relabel で解くと t に届かない余りが累積の鎖を行き来する形。
seed 1000 以上は愚直解 (brute.hpp) と突き合わせる N, M ≤ 8 の入力で、族を seed で回し、大きさを splitmix64 で選ぶ。

族:
  random    l と r を [0, M + 1] から一様に選び、小さい方を L、大きい方を R にする (同じなら選び直す)
  narrow    L を [0, W)、R を (M + 1 - W, M + 1] から選ぶ (両端の近くにしか座れない)
  right     L = 0 で、R を [1, M + 1] から選ぶ
  left      R = M + 1 で、L を [0, M] から選ぶ
  all       R = L + 1 (どの椅子にも座れる)
  half      L を [0, M / 2]、R を [M / 2 + 1, M + 1] から選ぶ
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

    def range(self, lo: int, hi: int) -> int:
        return lo + (self.next() * (hi - lo + 1) >> 64)


# seed: (族, N, M, W)。W は narrow のときだけ使う。
CASES = {
    0: ("random", 200000, 200000, 0),
    1: ("random", 200000, 100000, 0),
    2: ("random", 100000, 200000, 0),
    3: ("random", 200000, 199000, 0),
    4: ("random", 200000, 1000, 0),
    5: ("random", 1000, 200000, 0),
    6: ("narrow", 200000, 200000, 10000),
    7: ("narrow", 200000, 100000, 5000),
    8: ("right", 200000, 200000, 0),
    9: ("left", 200000, 150000, 0),
    10: ("all", 200000, 100000, 0),
    11: ("half", 200000, 200000, 0),
}


def person(r: SplitMix64, fam: str, m: int, w: int) -> tuple[int, int]:
    if fam == "random":
        while True:
            a, b = r.range(0, m + 1), r.range(0, m + 1)
            if a != b:
                return min(a, b), max(a, b)
    if fam == "narrow":
        w = min(w, m + 1)
        while True:
            lo, hi = r.range(0, w - 1), r.range(m + 2 - w, m + 1)
            if lo < hi:
                return lo, hi
    if fam == "right":
        return 0, r.range(1, m + 1)
    if fam == "left":
        return r.range(0, m), m + 1
    if fam == "all":
        x = r.range(0, m)
        return x, x + 1
    if fam == "half":
        return r.range(0, m // 2), r.range(m // 2 + 1, m + 1)
    raise SystemExit(f"unknown family {fam}")


def emit(r: SplitMix64, fam: str, n: int, m: int, w: int) -> None:
    out = [f"{n} {m}"]
    for _ in range(n):
        lo, hi = person(r, fam, m, w)
        out.append(f"{lo} {hi}")
    sys.stdout.write("\n".join(out) + "\n")


def main() -> None:
    seed = int(sys.argv[1])
    r = SplitMix64(seed * 1000003 + 7)
    if seed in CASES:
        fam, n, m, w = CASES[seed]
        emit(r, fam, n, m, w)
    elif seed >= 1000:
        fams = ["random", "narrow", "right", "left", "all", "half"]
        fam = fams[seed % len(fams)]
        n, m = r.range(1, 8), r.range(1, 8)
        emit(r, fam, n, m, r.range(1, 3))
    else:
        raise SystemExit(f"seed {seed} は 0 から {len(CASES) - 1} か、1000 以上にしてください")


if __name__ == "__main__":
    main()
