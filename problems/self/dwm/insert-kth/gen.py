#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""列への挿入・変更と区間 k 番目の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

1 行目に N Q、2 行目に初期の列、続く Q 行がクエリ。位置は 0-indexed。
  0 x val  : 位置 x の前に val を挿入 (0 <= x <= 現在の長さ)
  1 x val  : 位置 x の値を val に変更 (0 <= x < 現在の長さ)
  2 x y k  : [x, y) の k 番目 (0 <= x < y <= 現在の長さ, 0 <= k < y-x)
値は 0 以上 70000 以下。seed 1000 以上は愚直解と突き合わせる小さい入力。
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


# seed -> (N, Q, 挿入の重み, 変更の重み, k 番目の重み, 値の上限, 位置の作り方)
# 位置の作り方は "random"、"front" (先頭への挿入が多い)、"hot" (先頭付近)、"ends" (k 番目の区間を端に寄せる)。
CASES = {
    0: (10, 30, 3, 3, 4, 20, "random"),
    1: (1, 2000, 1, 1, 1, 20, "random"),
    2: (35000, 100000, 20, 40, 40, 70001, "random"),
    3: (35000, 100000, 60, 20, 20, 70001, "random"),
    4: (35000, 100000, 5, 15, 80, 70001, "random"),
    5: (35000, 100000, 20, 40, 40, 1, "random"),  # 全部 0
    6: (35000, 100000, 10, 10, 80, 70001, "ends"),
    7: (35000, 175000, 25, 35, 40, 70001, "random"),
    8: (100, 175000, 30, 30, 40, 70001, "hot"),
    9: (35000, 100000, 50, 10, 40, 70001, "front"),
}


def main() -> None:
    seed = int(sys.argv[1])
    r = SplitMix64(seed * 1000003 + 98765)
    if seed >= 1000:
        n, q = 1 + r.below(30), 1 + r.below(50)
        wi, ws, wq = 1 + r.below(4), r.below(4), 1 + r.below(4)
        vmax, where = 1 + r.below(20), ("random", "front", "hot", "ends")[r.below(4)]
    else:
        n, q, wi, ws, wq, vmax, where = CASES[seed]
    wsum = wi + ws + wq

    def value() -> int:
        return r.below(vmax)

    def insert_pos(length: int) -> int:
        if where == "front":
            return 0 if r.below(4) else r.below(length + 1)
        if where == "hot":
            return r.below(min(length + 1, 8))
        return r.below(length + 1)

    def interval(length: int) -> tuple[int, int, int]:
        if where == "ends" and r.below(2):
            if r.below(2):
                l, rgt = 0, 1 + r.below(length)
            else:
                l = r.below(length)
                rgt = length
        else:
            l = r.below(length)
            rgt = l + 1 + r.below(length - l)
        k = r.below(rgt - l)
        return l, rgt, k

    a = [value() for _ in range(n)]
    out = [f"{n} {q}", " ".join(map(str, a))]
    length = n
    for _ in range(q):
        t = r.below(wsum)
        if length == 0 or t < wi:
            x = insert_pos(length)
            out.append(f"0 {x} {value()}")
            length += 1
        elif t < wi + ws:
            x = r.below(length)
            out.append(f"1 {x} {value()}")
        else:
            l, rgt, k = interval(length)
            out.append(f"2 {l} {rgt} {k}")
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
