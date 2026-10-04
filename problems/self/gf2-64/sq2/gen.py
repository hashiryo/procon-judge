#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""GF(2^64) の二乗 (2 並列) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

1 行目が T と K、続く T 行が (a, b)。期待出力は参照実装 (problem.toml の reference) がハーネスと
一緒に作るので、ここでは入力だけを書く。

時間の差を見るのは最後のケース (T = 1000、K = 1000 で、2 並列の二乗が 10^6 回) で、どの回も
前の答えを待つので sq2 の待ち時間で差が付く。入出力は 1000 組ぶんしか無いので帯域の差は出ない。

ほかは正しさの確かめに使う。二乗の reduce は値の上 2 bit (bit 62 と 63) で表を引くので、edge は
その 4 通りを a と b で組み合わせた 16 通りを全部踏ませる。T = 0 と K = 0 (二乗しない) も入れる。
"""

import random
import sys

MASK64 = (1 << 64) - 1
# 0 と 1 は二乗で動かない値。上 2 bit の 4 通り (1 << 62、1 << 63、3 << 62 と、立っていないもの) は
# reduce の表引きの全部の欄を踏む。
EDGE = [0, 1, 2, 3, 1 << 62, 1 << 63, 3 << 62, MASK64, MASK64 - 1]

# seed -> (種類, T, K)
CASES = {
    0: ("sample", 8, 1),
    1: ("random", 0, 5),
    2: ("random", 1, 0),
    3: ("random", 3, 2),
    4: ("edge", 0, 1),
    5: ("structured", 1000, 3),
    6: ("random", 10000, 7),
    7: ("random", 1000, 1000),
}


def make(kind: str, t: int, rng: random.Random) -> list:
    if kind == "sample":
        return [
            (0, 1),
            (2, 3),
            (1 << 32, 1 << 63),
            (3 << 62, 1 << 62),
            (MASK64, MASK64 - 1),
            (0xFF, 0xFF00),
            (0x12345678ABCDEF00, 0xFEDCBA9876543210),
            (0x8000000000000001, 0x4000000000000002),
        ][:t]
    if kind == "edge":
        rows = [(a, b) for a in EDGE for b in EDGE]
        # 上 2 bit を固定し、下 62 bit を乱数にした組。a と b の上 2 bit の 16 通りを 8 組ずつ。
        for ta in range(4):
            for tb in range(4):
                rows += [(ta << 62 | rng.getrandbits(62), tb << 62 | rng.getrandbits(62)) for _ in range(8)]
        return rows
    if kind == "structured":
        # 1 bit だけ立った値に近いもの。二乗で bit が端に寄る。
        return [((1 << rng.randint(0, 63)) | rng.randint(0, 7), (1 << rng.randint(0, 63)) | rng.randint(0, 7)) for _ in range(t)]
    if kind == "random":
        return [(rng.randint(0, MASK64), rng.randint(0, MASK64)) for _ in range(t)]
    raise ValueError(kind)


def main() -> None:
    seed = int(sys.argv[1])
    if seed not in CASES:
        raise SystemExit(f"seed {seed} は {len(CASES)} 未満にしてください")
    kind, t, k = CASES[seed]
    rng = random.Random(seed * 1_000_003 + 12345)
    rows = make(kind, t, rng)
    out = [f"{len(rows)} {k}"]
    out += [f"{a} {b}" for a, b in rows]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
