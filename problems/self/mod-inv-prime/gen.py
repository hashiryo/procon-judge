#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""素数 mod の逆元の入力を作る。p = 998244353 固定で、n 個の a を出す。5% は 0 (逆元なし)。

期待出力は参照実装 (naive_fermat) がハーネスと一緒に作る。旧 judge の gen.py の大きさを seed に割り当てた。
"""
import random
import sys

P = 998244353
CASES = {0: 8, 1: 100, 2: 10000, 3: 100000, 4: 1000000}


def main() -> None:
    seed = int(sys.argv[1])
    n = CASES[seed]
    rng = random.Random(seed * 1_000_003 + 42)
    out = [f"{P} {n}"]
    for _ in range(n):
        out.append("0" if rng.random() < 0.05 else str(rng.randint(1, P - 1)))
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
