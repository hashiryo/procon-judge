#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""決まった値を掛ける配列の積和の throughput (mod = 998244353 固定) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

期待出力は参照実装 (problem.toml の reference) がハーネスと一緒に作るので、ここでは入力だけを書く。
配列そのものは書かず、ハーネスが splitmix64 で作る。並びと大きさは throughput-runtime-30 と同じで、法だけを 998244353 に固定した。

入力: N R seed fill w (fill = 0 は [0, mod) の一様乱数、fill = 1 はすべて mod - 1。w は掛ける決まった値)
"""
import random
import sys

MOD = 998244353


def cases() -> list[tuple[int, int, int, int, int]]:
    rng = random.Random(43)
    out = []
    small = [
        (0, 1, 1, 0),  # 空の配列
        (1, 3, 2, 0),
        (7, 5, 3, 0),  # 8 の倍数でない長さ (ベクトル化したループの端)
        (1024, 10, 4, 1),  # 値がすべて mod - 1
    ]
    for n, r, s, fill in small:
        out.append((n, r, s, fill, MOD - 1 if fill == 1 else rng.randrange(MOD)))
    for n, r, s in [(2048, 100_000, 5), (32_768, 6_000, 6), (1 << 22, 48, 7)]:  # L1、L2、メモリ
        out.append((n, r, s, 0, rng.randrange(MOD)))
    return out


def main() -> None:
    seed = int(sys.argv[1])
    all_cases = cases()
    if not 0 <= seed < len(all_cases):
        raise SystemExit(f"seed {seed} は {len(all_cases)} 未満にしてください")
    n, r, s, fill, w = all_cases[seed]
    print(f"{n} {r} {s} {fill} {w}")


if __name__ == "__main__":
    main()
