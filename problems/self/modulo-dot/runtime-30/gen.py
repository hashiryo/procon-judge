#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""n×n の行列の積 (mod は入力、2^20 < mod < 2^30 の奇数) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

期待出力は参照実装 (problem.toml の reference) がハーネスと一緒に作るので、ここでは入力だけを書く。
行列そのものは書かず、ハーネスが splitmix64 で作る。前の 4 ケースは正しさを見る小さいケースで、端の処理を通るように
4 や 16 の倍数でない大きさを混ぜた。後ろの 3 ケースは 128、256、512 の正方行列。

入力: n seed mod
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


def cases() -> list[tuple[int, int, int]]:
    rng = random.Random(45)
    out = [(0, 1, MIN_MOD), (1, 2, MAX_MOD), (7, 3, MAX_MOD), (35, 4, MIN_MOD)]
    for n, s in [(128, 5), (256, 6), (512, 7)]:
        out.append((n, s, rng.randrange(MIN_MOD, MAX_MOD + 1)))
    return [(n, s, normalize_mod(mod)) for n, s, mod in out]


def main() -> None:
    seed = int(sys.argv[1])
    all_cases = cases()
    if not 0 <= seed < len(all_cases):
        raise SystemExit(f"seed {seed} は {len(all_cases)} 未満にしてください")
    n, s, mod = all_cases[seed]
    print(f"{n} {s} {mod}")


if __name__ == "__main__":
    main()
