#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""洛谷 P4177 ([CEOI 2008] order) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

1 ≤ N, M ≤ 1200、1 ≤ x_i ≤ 5000、借りる費用 b と買う費用 y は 1 以上 20000 以下。seed 0 から 9 は本番のケースで、大きさと値の
範囲は CASES の表のとおり。seed 1000 以上は愚直解 (brute.hpp) と突き合わせる N ≤ 8、M ≤ 6 の入力。
仕事 i の工程の機械は、乱択の始点 c と M と互いに素な乱択の歩幅 a で、c, c + a, c + 2a, ... (mod M) の先頭 t_i 個にする
(同じ機械は 2 度使わない)。乱数は Mersenne Twister (random.Random) で、整数の seed からの列は Python の版によらない。

値の範囲の決め方:
  x の範囲と、t_i 個の b の和が同じくらいなら、仕事ごとに得か損かが分かれる。買う費用 y が、その機械を使う仕事の b の和と
  同じくらいなら、買うか借りるかが分かれる。どちらかに偏ると、答えが 0 か、全部を選ぶ自明な形になる。
"""
import math
import random
import sys

# seed: (N, M, t の範囲, x の範囲, b の範囲, y の範囲)
CASES = {
    0: (1200, 1200, (1200, 1200), (1, 5000), (1, 20000), (1, 20000)),  # 制約の全幅 (答えは 0 になりやすい)
    1: (1200, 1200, (1200, 1200), (1, 5000), (1, 6), (1, 20000)),
    2: (1200, 1200, (1200, 1200), (2000, 5000), (1, 4), (1, 3000)),
    3: (1200, 1200, (1200, 1200), (5000, 5000), (1, 1), (20000, 20000)),  # 全部を借りる
    4: (1200, 1200, (1200, 1200), (5000, 5000), (5, 5), (1000, 1000)),  # 全部を買う (1 つずつでは損)
    5: (1200, 1200, (1, 60), (1, 5000), (1, 400), (1, 4000)),
    6: (1200, 1200, (400, 800), (1, 5000), (1, 12), (1, 8000)),
    7: (1200, 1200, (1200, 1200), (1, 5000), (1, 3), (1, 20000)),
    8: (1200, 1, (1, 1), (1, 5000), (1, 20000), (1, 20000)),
    9: (1, 1200, (1200, 1200), (5000, 5000), (1, 20000), (1, 20000)),
}


def emit(rnd: random.Random, n: int, m: int, tr, xr, br, yr) -> None:
    out = [f"{n} {m}"]
    for _ in range(n):
        t = rnd.randint(*tr)
        out.append(f"{rnd.randint(*xr)} {t}")
        c = rnd.randrange(m)
        while True:
            a = rnd.randrange(1, m) if m > 1 else 1
            if math.gcd(a, m) == 1:
                break
        lo, hi = br
        bs = [lo + rnd.getrandbits(32) * (hi - lo + 1) // (1 << 32) for _ in range(t)] if lo < hi else [lo] * t
        out.extend(f"{(c + a * k) % m + 1} {b}" for k, b in enumerate(bs))
    out.extend(str(rnd.randint(*yr)) for _ in range(m))
    sys.stdout.write("\n".join(out) + "\n")


def main() -> None:
    seed = int(sys.argv[1])
    rnd = random.Random(seed * 1000003 + 7)
    if seed in CASES:
        emit(rnd, *CASES[seed])
    elif seed >= 1000:
        n, m = rnd.randint(1, 8), rnd.randint(1, 6)
        emit(rnd, n, m, (1, m), (1, 20), (1, rnd.randint(1, 8)), (1, rnd.randint(1, 20)))
    else:
        raise SystemExit(f"seed {seed} は 0 から {len(CASES) - 1} か、1000 以上にしてください")


if __name__ == "__main__":
    main()
