#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""GF(2^64) の定数倍 の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

期待出力は参照実装 (problem.toml の reference) がハーネスと一緒に作るので、
ここでは入力だけを書く。掛ける相手は common.hpp の定数なので、入力は片側だけ。
ケースの種類は gf2-64-frob2 と同じ割り当てにしてある。
"""

import random
import sys

MASK64 = (1 << 64) - 1

# seed -> (種類, 件数)
CASES = {
    0: ("sample", 8),
    1: ("small", 100),
    2: ("edge_zero_one", 200),
    3: ("structured", 1000),
    4: ("random", 10000),
    5: ("random", 100000),
    6: ("random", 1000000),
}


def make(kind: str, t: int, rng: random.Random) -> list:
    if kind == "sample":
        return [0, 1, 2, 3, 0xFF, MASK64, 0x12345678ABCDEF00, 0xFEDCBA9876543210][:t]
    if kind == "small":
        return [rng.randint(0, 255) for _ in range(t)]
    if kind == "random":
        return [rng.randint(0, MASK64) for _ in range(t)]
    if kind == "edge_zero_one":
        return [rng.choice([0, 1, MASK64, rng.randint(0, MASK64)]) for _ in range(t)]
    if kind == "structured":
        return [(1 << rng.randint(0, 63)) | rng.randint(0, 7) for _ in range(t)]
    raise ValueError(kind)


def main() -> None:
    seed = int(sys.argv[1])
    if seed not in CASES:
        raise SystemExit(f"seed {seed} は {len(CASES)} 未満にしてください")
    kind, t = CASES[seed]
    rng = random.Random(seed * 1_000_003 + 12345)
    rows = make(kind, t, rng)
    out = [str(len(rows))]
    out += [
        " ".join(str(v) for v in row) if isinstance(row, tuple) else str(row)
        for row in rows
    ]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
