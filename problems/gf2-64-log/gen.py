#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""GF(2^64) の離散対数 の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

期待出力は参照実装 (problem.toml の reference) がハーネスと一緒に作るので、
ここでは入力だけを書く。旧 judge の gf2-64-log/testcases/gen.py の種類を seed に割り当てた。
"""
import random
import sys

MASK64 = (1 << 64) - 1

# seed -> (種類, 件数)
CASES = {0: ("sample", 8), 1: ("small", 50), 2: ("edge", 50), 3: ("random", 1000), 4: ("random", 10000)}


ORDER = MASK64  # 乗法群の位数 2^64 - 1
IRRED_LOW = 0x1B  # x^64 ≡ x^4 + x^3 + x + 1


def gf_mul(a: int, b: int) -> int:
    """GF(2)[x] / (x^64 + x^4 + x^3 + x + 1) の積。素朴でよい (入力を作るだけ)。"""
    r = 0
    while b:
        if b & 1:
            r ^= a
        b >>= 1
        a <<= 1
        if a >> 64:
            a = (a & MASK64) ^ IRRED_LOW
    return r


def gf_pow(a: int, e: int) -> int:
    r = 1
    while e:
        if e & 1:
            r = gf_mul(r, a)
        a = gf_mul(a, a)
        e >>= 1
    return r


def make(kind: str, t: int, rng: random.Random) -> list:
    """答え k を決めてから x = 2^k を作る。答えは参照実装が出す。"""
    if kind == "sample":
        ks = [0, 1, 2, 100, 1 << 16, 1 << 32, 1 << 60, ORDER - 1][:t]
    elif kind == "small":
        ks = [rng.randint(0, 1000) for _ in range(t)]
    elif kind == "edge":
        ks = [rng.choice([0, 1, 2, ORDER - 2, ORDER - 1, rng.randint(0, ORDER - 1)]) for _ in range(t)]
    elif kind == "random":
        ks = [rng.randint(0, ORDER - 1) for _ in range(t)]
    else:
        raise ValueError(kind)
    return [gf_pow(2, k) for k in ks]


def main() -> None:
    seed = int(sys.argv[1])
    if seed not in CASES:
        raise SystemExit(f"seed {seed} は {len(CASES)} 未満にしてください")
    kind, t = CASES[seed]
    rng = random.Random(seed * 1_000_003 + 12345)
    rows = make(kind, t, rng)
    out = [str(len(rows))]
    out += [" ".join(str(v) for v in row) if isinstance(row, tuple) else str(row) for row in rows]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
