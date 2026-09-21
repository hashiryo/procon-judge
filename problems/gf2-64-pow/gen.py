#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""GF(2^64) の冪 の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

期待出力は参照実装 (problem.toml の reference) がハーネスと一緒に作るので、
ここでは入力だけを書く。旧 judge の gf2-64-pow/testcases/gen.py の種類を seed に割り当てた。
"""
import random
import sys

MASK64 = (1 << 64) - 1

# seed -> (種類, 件数)
CASES = {0: ("sample", 10), 1: ("small_e", 100), 2: ("edge_e", 200), 3: ("random", 10000), 4: ("random", 100000), 5: ("chunk_top_bit", 100000)}


def make(kind: str, t: int, rng: random.Random) -> list:
    if kind == "sample":
        return [(0, 0), (0, 1), (1, 5), (2, 0), (2, 1), (2, 64), (3, MASK64), (5, 1 << 63),
                (MASK64, MASK64), (0xDEADBEEF, 0x1234567)][:t]
    if kind == "small_e":
        return [(rng.randint(0, MASK64), rng.randint(0, 100)) for _ in range(t)]
    if kind == "random":
        return [(rng.randint(0, MASK64), rng.randint(0, MASK64)) for _ in range(t)]
    if kind == "chunk_top_bit":
        # e を 16 bit ずつに分けたとき、どの塊も最上位 bit が立っている。
        out = []
        for _ in range(t):
            e = 0
            for s in (0, 16, 32, 48):
                e |= (0x8000 | rng.randint(0, 0x7FFF)) << s
            out.append((rng.randint(0, MASK64), e))
        return out
    if kind == "edge_e":
        return [(rng.choice([0, 1, MASK64, rng.randint(0, MASK64)]),
                 rng.choice([0, 1, 2, MASK64, MASK64 - 1, 1 << rng.randint(0, 63)])) for _ in range(t)]
    raise ValueError(kind)


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
