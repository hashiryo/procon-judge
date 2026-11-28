#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""列への追加・削除・変更と区間 k 番目の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

1 行目に Q、続く Q 行がクエリ。位置は 0-indexed。
  0 val    : 末尾に val を追加
  1 x      : 位置 x を削除
  2 x val  : 位置 x の値を val に変更
  3 x y k  : [x, y) の k 番目
値は 1 以上 10^9 以下。seed 1000 以上は愚直解と突き合わせる小さい入力。
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


# seed -> (Q, 追加の重み, 削除の重み, 変更の重み, k 番目の重み, 値の上限, 位置の作り方)
CASES = {
    0: (40, 3, 2, 2, 3, 100, "random"),
    1: (1000, 2, 2, 1, 1, 10, "random"),
    2: (100000, 30, 20, 20, 30, 10**9, "random"),
    3: (100000, 70, 10, 10, 10, 10**9, "random"),
    4: (100000, 20, 40, 20, 20, 10**9, "random"),
    5: (100000, 5, 5, 10, 80, 10**9, "random"),
    6: (100000, 30, 20, 20, 30, 1, "random"),  # 全部 1
    7: (100000, 30, 20, 20, 30, 10**9, "max"),  # 全部 10^9
    8: (100000, 20, 20, 20, 40, 10**9, "ends"),
    9: (100000, 50, 50, 0, 0, 10**9, "random"),  # 末尾の追加と削除だけ
}


def main() -> None:
    seed = int(sys.argv[1])
    r = SplitMix64(seed * 1000003 + 98765)
    if seed >= 1000:
        q = 1 + r.below(60)
        wa, we, ws, wk = 1 + r.below(3), r.below(3), r.below(3), 1 + r.below(3)
        vmax, where = 1 + r.below(20), ("random", "ends")[r.below(2)]
    else:
        q, wa, we, ws, wk, vmax, where = CASES[seed]
    wsum = wa + we + ws + wk

    def value() -> int:
        if where == "max":
            return 10**9
        return 1 + r.below(vmax)

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
        return l, rgt, r.below(rgt - l)

    out = [str(q)]
    length = 0
    for _ in range(q):
        t = r.below(wsum)
        if length == 0 or t < wa:
            out.append(f"0 {value()}")
            length += 1
        elif t < wa + we:
            x = r.below(length)
            out.append(f"1 {x}")
            length -= 1
        elif t < wa + we + ws:
            x = r.below(length)
            out.append(f"2 {x} {value()}")
        else:
            l, rgt, k = interval(length)
            out.append(f"3 {l} {rgt} {k}")
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
