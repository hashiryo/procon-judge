#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""64 bit の整数から剰余の表現への変換 (mod = 998244353 固定) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

期待出力は参照実装 (problem.toml の reference) がハーネスと一緒に作るので、ここでは入力だけを書く。
配列そのものは書かず、ハーネスが splitmix64 で作る。大きさの段は modulo-throughput の static-998244353 と同じで、
前の 4 ケースは正しさを見る小さいケース、後ろの 3 ケースはキャッシュに載る大きさで分けた L1、L2、メモリの段。

入力: N R seed
"""
import sys

def cases() -> list[tuple[int, int, int]]:
    # L1、L2、メモリの段は後ろの 3 つ
    return [(0, 1, 1), (1, 3, 2), (10, 5, 3), (1024, 10, 4), (2048, 100_000, 5), (32_768, 6_000, 6), (1 << 22, 48, 7)]


def main() -> None:
    seed = int(sys.argv[1])
    all_cases = cases()
    if not 0 <= seed < len(all_cases):
        raise SystemExit(f"seed {seed} は {len(all_cases)} 未満にしてください")
    n, r, s = all_cases[seed]
    print(f"{n} {r} {s}")


if __name__ == "__main__":
    main()
