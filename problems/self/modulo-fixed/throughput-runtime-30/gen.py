#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""決まった値を掛ける配列の積和の throughput (mod は入力、2^20 < mod < 2^30 の奇数) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

期待出力は参照実装 (problem.toml の reference) がハーネスと一緒に作るので、ここでは入力だけを書く。
配列そのものは書かず、ハーネスが splitmix64 で作る。大きさの段は modulo-throughput の runtime-30 と同じで、
前の 4 ケースは正しさを見る小さいケース、後ろの 3 ケースはキャッシュに載る大きさで分けた L1、L2、メモリの段。

入力: N R seed fill mod w (fill = 0 は [0, mod) の一様乱数、fill = 1 はすべて mod - 1。w は掛ける決まった値)
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


def cases() -> list[tuple[int, int, int, int, int, int]]:
    rng = random.Random(41)
    out = []
    small = [
        (0, 1, 1, 0, MIN_MOD),  # 空の配列
        (1, 3, 2, 0, MIN_MOD),
        (7, 5, 3, 0, MAX_MOD),  # 8 の倍数でない長さ (ベクトル化したループの端)
        (1024, 10, 4, 1, MAX_MOD),  # 値がすべて mod - 1
    ]
    for n, r, s, fill, mod in small:
        mod = normalize_mod(mod)
        out.append((n, r, s, fill, mod, mod - 1 if fill == 1 else rng.randrange(mod)))
    for n, r, s in [(2048, 100_000, 5), (32_768, 6_000, 6), (1 << 22, 48, 7)]:  # L1、L2、メモリ
        mod = normalize_mod(rng.randrange(MIN_MOD, MAX_MOD + 1))
        out.append((n, r, s, 0, mod, rng.randrange(mod)))
    return out


def main() -> None:
    seed = int(sys.argv[1])
    all_cases = cases()
    if not 0 <= seed < len(all_cases):
        raise SystemExit(f"seed {seed} は {len(all_cases)} 未満にしてください")
    n, r, s, fill, mod, w = all_cases[seed]
    print(f"{n} {r} {s} {fill} {mod} {w}")


if __name__ == "__main__":
    main()
