#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""決まった値を掛ける剰余の反復 (mod は入力、2^20 < mod < 2^30 の偶数) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

期待出力は参照実装 (problem.toml の reference) がハーネスと一緒に作るので、ここでは入力だけを書く。
手書きの小さいケースと角のケースのあとに、乱数のケースを大きさの順に並べる。法の範囲と偶奇の揃え方は modulo-test の runtime-30-even と同じ。

入力: N mod s w b (s = s * w + b を N 回くり返す)
"""
import random
import sys

MIN_MOD = (1 << 20) + 1
MAX_MOD = (1 << 30) - 1
EVEN = True


def normalize_mod(mod: int) -> int:
    mod = max(mod, MIN_MOD)
    mod = min(mod, MAX_MOD)
    if (mod % 2 == 0) != EVEN:
        mod += 1 if mod < MAX_MOD else -1
    return mod


def case(n: int, mod: int, s: int, w: int, b: int) -> str:
    mod = normalize_mod(mod)
    return f"{n} {mod} {s % mod} {w % mod} {b % mod}"


def cases() -> list[tuple[str, str]]:
    out = [
        ("small_00", case(0, MIN_MOD, 0, 0, 0)),
        ("small_01", case(1, MIN_MOD, 1, 0, 0)),
        ("small_02", case(5, MIN_MOD, 1, 1, 1)),
        ("edge_00", case(20, MIN_MOD, MIN_MOD - 1, MIN_MOD - 1, 0)),
        ("edge_01", case(20, MAX_MOD, MAX_MOD - 1, 1, MAX_MOD - 2)),
        ("edge_02", case(100, MAX_MOD, 123456789, MAX_MOD - 1, MAX_MOD - 1)),
    ]
    rng = random.Random(40)
    for name, n in [("rand_00", 10_000), ("rand_01", 100_000), ("mid_00", 10_000_000), ("mid_01", 30_000_000), ("heavy_00", 100_000_000), ("heavy_01", 200_000_000), ("heavy_02", 200_000_000)]:
        mod = normalize_mod(rng.randrange(MIN_MOD, MAX_MOD + 1))
        s, w, b = (rng.randrange(mod) for _ in range(3))
        out.append((name, case(n, mod, s, w, b)))
    return out


def main() -> None:
    seed = int(sys.argv[1])
    all_cases = cases()
    if not 0 <= seed < len(all_cases):
        raise SystemExit(f"seed {seed} は {len(all_cases)} 未満にしてください")
    print(all_cases[seed][1])


if __name__ == "__main__":
    main()
