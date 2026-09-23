#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""GF(2^64) の冪 の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

期待出力は参照実装 (problem.toml の reference) がハーネスと一緒に作るので、
ここでは入力だけを書く。seed 0..5 は旧 judge の gf2-64-pow/testcases/gen.py の種類を割り当てた
もので、seed 6 はこちらで足した。
"""
import random
import sys

MASK64 = (1 << 64) - 1
# (2^64-1)/(2^16-1) = 2^48 + 2^32 + 2^16 + 1。a^M は GF(2^16) に落ちる。
M = MASK64 // 0xFFFF

# seed -> (種類, 件数)
CASES = {0: ("sample", 10), 1: ("small_e", 100), 2: ("edge_e", 200), 3: ("random", 10000), 4: ("random", 100000), 5: ("chunk_top_bit", 100000), 6: ("rem_high", 1000)}


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
    if kind == "rem_high":
        # e を M で割ったあまり r が 2^48 以上。e = q M + r と分けて a^e = (a^M)^q · a^r と
        # する解法は、r の側を 48 bit の窓で回すことがあるが、r は M-1 まで取れるので
        # 48 bit に収まらない。ランダムな e がこの側に落ちる割合は 2^32/M ≒ 1.5e-5 しかない。
        # 先頭は境目の並び (r は 2^48 の 1 つ手前も入れて、枝の両側を踏む)、残りは r を一様に選ぶ。
        # 65535 M = 2^64-1 ちょうどなので、q = 65535 では r = 0 しか取れない。q は 65534 まで。
        out = [(a, q * M + r)
               for r in ((1 << 48) - 1, 1 << 48, (1 << 48) + 1, M - 2, M - 1)
               for q in (0, 1, 2, 65533, 65534)
               for a in (0, 1, 2, MASK64)]
        out += [(rng.randint(0, MASK64), rng.randint(0, 65534) * M + rng.randint(1 << 48, M - 1))
                for _ in range(t - len(out))]
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
