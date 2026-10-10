#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""二項係数の表と問い合わせ (mod = 998244353 固定) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

期待出力は参照実装 (problem.toml の reference) がハーネスと一緒に作るので、ここでは入力だけを書く。
並びと大きさは runtime-30 と同じで、法だけを 998244353 に固定した。法はハーネスがコンパイル時の定数として持つ。

入力: N Q seed (0..N の階乗の表を作り、Q 個の二項係数 C(n, k) (0 <= k <= n <= N) の和を答える)
"""
import sys

CASES = [
    (0, 0, 1),
    (1, 5, 2),
    (10, 100, 3),
    (1000, 10_000, 4),
    (100_000, 20_000_000, 5),  # 表は L2 に載り、問い合わせは計算で詰まる
    (10_000_000, 0, 6),  # 表を作るだけ (変換と掛け算の鎖)
    (10_000_000, 10_000_000, 7),  # 表はメモリにあり、問い合わせはメモリを引く
]


def main() -> None:
    seed = int(sys.argv[1])
    if not 0 <= seed < len(CASES):
        raise SystemExit(f"seed {seed} は {len(CASES)} 未満にしてください")
    n, q, s = CASES[seed]
    print(f"{n} {q} {s}")


if __name__ == "__main__":
    main()
