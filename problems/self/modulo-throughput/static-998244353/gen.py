#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""配列の積和の throughput (mod = 998244353 固定) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

期待出力は参照実装 (problem.toml の reference) がハーネスと一緒に作るので、ここでは入力だけを書く。
配列そのものは書かず、ハーネスが splitmix64 で作る。前の 4 ケースは正しさを見る小さいケースで、
後ろの 3 ケースはキャッシュに載る大きさで分けた L1、L2、メモリの段 (どれも要素の演算は 2 億回前後)。

入力: N R seed fill (fill = 0 は [0, mod) の一様乱数、fill = 1 はすべて mod - 1)
"""
import sys

CASES = [
    (0, 1, 1, 0),  # 空の配列
    (1, 3, 2, 0),
    (7, 5, 3, 0),  # 8 の倍数でない長さ (ベクトル化したループの端)
    (1024, 10, 4, 1),  # 値がすべて mod - 1
    (2048, 100_000, 5, 0),  # L1 に載る
    (32_768, 6_000, 6, 0),  # L2 に載る
    (1 << 22, 48, 7, 0),  # メモリ
]


def main() -> None:
    seed = int(sys.argv[1])
    if not 0 <= seed < len(CASES):
        raise SystemExit(f"seed {seed} は {len(CASES)} 未満にしてください")
    n, r, s, fill = CASES[seed]
    print(f"{n} {r} {s} {fill}")


if __name__ == "__main__":
    main()
