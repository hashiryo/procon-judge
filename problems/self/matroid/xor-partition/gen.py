#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""GF(2) の線形マトロイドと分割マトロイドの交差の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

入力は「族の名前 seed 引数...」の 1 行で、ベクトルと色はハーネス (base.cpp) が作る。族と引数の意味は base.cpp の
冒頭に書いてある。seed 0 から 9 は本番のケース。seed 1000 以上は愚直解 (brute.hpp) と突き合わせる小さい入力
(ベクトル 16 本まで) で、族を seed で決め、引数を splitmix64 で選ぶ。
"""
import sys

MASK64 = (1 << 64) - 1

CASES = {
    0: "random 1 200000 64 64 1",
    1: "random 2 200000 64 16 4",
    2: "low 3 200000 64 48 48 1",
    3: "low 4 200000 64 63 63 1",
    4: "nim 5 200000 64",
    5: "sparse 6 200000 64 2 64 1",
    6: "sparse 7 200000 64 3 8 8",
    7: "chain 8 200000 64 63 1",
    8: "match 9 200000 64",
    9: "match 10 200000 63",
}


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


def small_case(seed: int) -> str:
    """ベクトルが 16 本までの入力。族を seed で順に回す。"""
    r = SplitMix64(seed * 1000003 + 7)
    fam = seed % 6
    m = r.range(0, 16)
    if fam == 0:
        return f"random {seed} {m} {r.range(1, 6)} {r.range(1, 5)} {r.range(0, 3)}"
    if fam == 1:
        d = r.range(1, 8)
        return f"low {seed} {m} {d} {r.range(1, d)} {r.range(1, 5)} {r.range(0, 3)}"
    if fam == 2:
        return f"nim {seed} {m} {r.range(1, 6)}"
    if fam == 3:
        return f"sparse {seed} {m} {r.range(1, 8)} {r.range(1, 3)} {r.range(1, 5)} {r.range(0, 3)}"
    if fam == 4:
        return f"chain {seed} {m} {r.range(2, 8)} {r.range(1, 5)} {r.range(0, 3)}"
    return f"match {seed} {m} {r.range(1, 6)}"


def main() -> None:
    seed = int(sys.argv[1])
    print(CASES[seed] if seed in CASES else small_case(seed))


if __name__ == "__main__":
    main()
