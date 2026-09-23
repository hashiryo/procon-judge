#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""GF(2^64) の積 (2 並列) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

期待出力は参照実装 (problem.toml の reference) がハーネスと一緒に作るので、ここでは入力だけを書く。

件数の決め方が gf2-64-mul と違う。一番大きいケースでも T = 10^5 (a, b, 答えで 2.4 MB) に
抑えてあり、作業集合が cache に収まる。ここを 10^6 にすると 1 積あたり 24 byte の読み書きに
対してベクタ命令が数個しか無く、帯域律速になって mul の中身の差が消えるため。

小さい方は端数の確認に使う。1 反復 2 積の実装は T が奇数のとき、1 反復 4 積の実装は
T mod 4 が 0 でないときに端数の処理が要る。T = 0, 1, 2, 3 は本体のループが 1 回も回らない側、
T = 5, 6, 7 は回ったうえで端数が出る側で、どちらも踏ませる。
"""
import random
import sys

MASK64 = (1 << 64) - 1
# 0 と 1 と最大値は積の性質 (吸収元・単位元・最上位 bit の折り返し) を踏む値。
EDGE = [0, 1, 2, MASK64, MASK64 - 1]

# seed -> (種類, 件数)
CASES = {0: ("sample", 8), 1: ("small", 0), 2: ("small", 1), 3: ("small", 2), 4: ("small", 3), 5: ("small", 5),
         6: ("small", 6), 7: ("small", 7), 8: ("edge", 201), 9: ("structured", 1001), 10: ("random", 10007),
         11: ("random", 99999)}


def make(kind: str, t: int, rng: random.Random) -> list:
    if kind == "sample":
        return [(0, 0), (0, 1), (1, 1), (2, 2), (3, 5), (MASK64, 1), (MASK64, MASK64),
                (0x12345678ABCDEF00, 0xFEDCBA9876543210)][:t]
    if kind == "small":
        return [(rng.choice(EDGE + [rng.randint(0, MASK64)]), rng.choice(EDGE + [rng.randint(0, MASK64)]))
                for _ in range(t)]
    if kind == "edge":
        return [(rng.choice(EDGE + [rng.randint(0, MASK64)]), rng.choice(EDGE + [rng.randint(0, MASK64)]))
                for _ in range(t)]
    if kind == "structured":
        # 1 bit だけ立った値に近いもの。桁上がりの折り返しが端に寄る。
        return [((1 << rng.randint(0, 63)) | rng.randint(0, 7), (1 << rng.randint(0, 63)) | rng.randint(0, 7))
                for _ in range(t)]
    if kind == "random":
        return [(rng.randint(0, MASK64), rng.randint(0, MASK64)) for _ in range(t)]
    raise ValueError(kind)


def main() -> None:
    seed = int(sys.argv[1])
    if seed not in CASES:
        raise SystemExit(f"seed {seed} は {len(CASES)} 未満にしてください")
    kind, t = CASES[seed]
    rng = random.Random(seed * 1_000_003 + 12345)
    rows = make(kind, t, rng)
    out = [str(len(rows))]
    out += [" ".join(str(v) for v in row) for row in rows]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
