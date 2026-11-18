#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""洛谷 P6562 ([SBCOI2020] 归家之路) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

1 行目に n と q、2 行目に 2^n 個の初期値 ([0, 2^32))、続く q 行に操作 (1 a b k か 2 a b)。seed 0 は問題文の例。
seed 1 から 13 は n = 16、q = 2e5 で、操作の作り方を変える: a ⊆ b の乱数 (加算と問いが半々、加算が 9 割、問いが 9 割)、
自由な bit (b \\ a) が 8 個前後のもの (素朴にたどる手と包除の手が同じくらい重い)、自由な bit が多いもの (a = 0 に近い)、
1 点だけのもの (a = b)、全体 (a = 0、b = 2^n - 1)、a ⊆ b でないものを 2 割混ぜたもの、初めに加算だけを並べて後に問いだけを並べたもの。
seed 1000 以上は n = 1 から 6、q <= 30 の小さい入力 (愚直解 brute.hpp 用)。
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


def main() -> None:
    seed = int(sys.argv[1])
    rng = SplitMix64(seed * 1000003 + 91)
    if seed == 0:
        print("3 3")
        print("1 2 3 4 5 6 7 8")
        print("2 0 7")
        print("1 1 5 1")
        print("2 1 7")
        return
    if seed >= 1000:
        n = 1 + rng.next() % 6
        q = 1 + rng.next() % 30
    else:
        n, q = 16, 200000
    full = (1 << n) - 1
    # 操作の作り方: (加算の割合 (百分率), 自由な bit の作り方, a ⊆ b でないものの割合, 加算を先に並べるか)
    kinds = {
        1: (50, "rand", 0, False), 2: (90, "rand", 0, False), 3: (10, "rand", 0, False), 4: (50, "half", 0, False),
        5: (90, "half", 0, False), 6: (10, "half", 0, False), 7: (50, "many", 0, False), 8: (50, "point", 0, False),
        9: (50, "all", 0, False), 10: (50, "rand", 20, False), 11: (50, "rand", 0, True), 12: (50, "few1", 0, False),
        13: (30, "half", 0, False),
    }
    pu, mode, bad, sorted_ops = kinds.get(seed, (50, "rand", 10, False))
    lines = []
    for _ in range(q):
        if mode == "rand":
            b = rng.next() & full
            a = b & rng.next()
        elif mode == "half":  # 1 の bit と自由な bit と 0 の bit がそれぞれ n / 3 前後か、自由な bit が n / 2 前後
            b, a = 0, 0
            for i in range(n):
                r = rng.next() % 6
                if r < 3:  # 自由
                    b |= 1 << i
                elif r < 5:  # 1
                    b |= 1 << i
                    a |= 1 << i
        elif mode == "many":
            b = full ^ (1 << (rng.next() % n)) if rng.next() % 2 else full
            a = (1 << (rng.next() % n)) & b if rng.next() % 2 else 0
        elif mode == "point":
            a = b = rng.next() & full
        elif mode == "all":
            a, b = 0, full
        else:  # few1: 1 の bit が少なく、自由な bit が多い
            b = full & ~(rng.next() & rng.next() & rng.next())
            a = b & rng.next() & rng.next() & rng.next()
        if bad and rng.next() % 100 < bad:
            a |= 1 << (rng.next() % n)
            b &= ~a | (rng.next() & full)
        lines.append((rng.next() % 100 < pu, a, b, rng.next() & 0xFFFFFFFF))
    if sorted_ops:
        lines.sort(key=lambda t: not t[0])
    print(n, q)
    print(" ".join(str(rng.next() & 0xFFFFFFFF) for _ in range(1 << n)))
    out = []
    for is_add, a, b, k in lines:
        out.append(f"1 {a} {b} {k}" if is_add else f"2 {a} {b}")
    print("\n".join(out))


if __name__ == "__main__":
    main()
