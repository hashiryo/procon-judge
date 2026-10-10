#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""n×n の行列の積 (mod = 998244353 固定) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

期待出力は参照実装 (problem.toml の reference) がハーネスと一緒に作るので、ここでは入力だけを書く。
行列そのものは書かず、ハーネスが splitmix64 で作る。前の 4 ケースは正しさを見る小さいケースで、端の処理を通るように
4 や 16 の倍数でない大きさを混ぜた。後ろの 3 ケースは 128、256、512 の正方行列。

入力: n seed
"""
import sys

CASES = [(0, 1), (1, 2), (7, 3), (35, 4), (128, 5), (256, 6), (512, 7)]


def main() -> None:
    seed = int(sys.argv[1])
    all_cases = CASES
    if not 0 <= seed < len(all_cases):
        raise SystemExit(f"seed {seed} は {len(all_cases)} 未満にしてください")
    n, s = all_cases[seed]
    print(f"{n} {s}")


if __name__ == "__main__":
    main()
