#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""64 bit の整数から剰余の表現への変換 (mod は入力、2^20 < mod < 2^30 の奇数) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

期待出力は参照実装 (problem.toml の reference) がハーネスと一緒に作るので、ここでは入力だけを書く。
配列そのものは書かず、ハーネスが splitmix64 で作る。大きさの段は modulo-throughput の runtime-30 と同じで、
前の 4 ケースは正しさを見る小さいケース、後ろの 3 ケースはキャッシュに載る大きさで分けた L1、L2、メモリの段。

入力: N R seed mod
"""
import random
import sys

MIN_MOD = (1 << 20) + 1
MAX_MOD = (1 << 30) - 1
EVEN = False


def normalize_mod(mod: int) -> int:
    mod = max(mod, MIN_MOD)
    mod = min(mod, MAX_MOD)
    if (mod % 2 == 0) != EVEN:
        mod += 1 if mod < MAX_MOD else -1
    return mod


def cases() -> list[tuple[int, int, int, int]]:
    rng = random.Random(44)
    out = [(0, 1, 1, MIN_MOD), (1, 3, 2, MIN_MOD), (10, 5, 3, MAX_MOD), (1024, 10, 4, MAX_MOD)]
    for n, r, s in [(2048, 100_000, 5), (32_768, 6_000, 6), (1 << 22, 48, 7)]:  # L1、L2、メモリ
        out.append((n, r, s, rng.randrange(MIN_MOD, MAX_MOD + 1)))
    return [(n, r, s, normalize_mod(mod)) for n, r, s, mod in out]


def main() -> None:
    seed = int(sys.argv[1])
    all_cases = cases()
    if not 0 <= seed < len(all_cases):
        raise SystemExit(f"seed {seed} は {len(all_cases)} 未満にしてください")
    n, r, s, mod = all_cases[seed]
    print(f"{n} {r} {s} {mod}")


if __name__ == "__main__":
    main()
