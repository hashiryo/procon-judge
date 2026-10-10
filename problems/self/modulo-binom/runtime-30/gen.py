#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""二項係数の表と問い合わせ (法 p は入力、30 bit の素数) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

期待出力は参照実装 (problem.toml の reference) がハーネスと一緒に作るので、ここでは入力だけを書く。
問い合わせの (n, k) は書かず、ハーネスが splitmix64 で作る。前の 4 ケースは正しさを見る小さいケースで、後ろの 3 ケースは、
表が L2 に載る大きさで問い合わせが多いもの、表を作るだけのもの、表がメモリにあって問い合わせがメモリを引くものに分けた。
素数は modpow-test の runtime-30 と同じ並びから選ぶ。どれも 10^7 より大きいので、階乗の逆元がある。

入力: N Q seed p (0..N の階乗の表を作り、Q 個の二項係数 C(n, k) (0 <= k <= n <= N) の和を答える)
"""
import sys

PRIMES_30BIT = [998244353, 1000000007, 1000000009, 924844033, 985661441, 754974721, 167772161]

CASES = [
    (0, 0, 1, PRIMES_30BIT[0]),
    (1, 5, 2, PRIMES_30BIT[1]),
    (10, 100, 3, PRIMES_30BIT[2]),
    (1000, 10_000, 4, PRIMES_30BIT[3]),
    (100_000, 20_000_000, 5, PRIMES_30BIT[4]),  # 表は L2 に載り、問い合わせは計算で詰まる
    (10_000_000, 0, 6, PRIMES_30BIT[1]),  # 表を作るだけ (変換と掛け算の鎖)
    (10_000_000, 10_000_000, 7, PRIMES_30BIT[0]),  # 表はメモリにあり、問い合わせはメモリを引く
]


def main() -> None:
    seed = int(sys.argv[1])
    if not 0 <= seed < len(CASES):
        raise SystemExit(f"seed {seed} は {len(CASES)} 未満にしてください")
    n, q, s, p = CASES[seed]
    print(f"{n} {q} {s} {p}")


if __name__ == "__main__":
    main()
