#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""剰余の反復 (mod = 10^9 + 7 固定) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

期待出力は参照実装 (problem.toml の reference) がハーネスと一緒に作るので、ここでは入力だけを書く。
旧 judge の modulo-test/static/static-1e9+7/gen/make_inputs.py の手書きケースと乱数ケースを、その順番のまま seed に割り当てた。
乱数の種も同じなので、中身は旧 judge の testcases/*.in と一致する。

入力: N s a b c d (mod = 1_000_000_007 固定)
"""
import random
import sys

MOD = 1_000_000_007


def case(n: int, s: int, a: int, b: int, c: int, d: int) -> str:
    return f"{n} {s} {a} {b} {c} {d}"


def cases() -> list[tuple[str, str]]:
    out = [
        ("small_00", case(0, 0, 0, 0, 0, 0)),
        ("small_01", case(1, 1, 0, 0, 0, 0)),
        ("small_02", case(5, 1, 1, 1, 1, 1)),
        ("edge_00", case(20, MOD - 1, MOD - 1, 0, MOD - 1, 0)),
        ("edge_01", case(20, MOD - 1, 1, MOD - 1, 1, MOD - 1)),
        ("edge_02", case(100, 123456789, 0, MOD - 1, 0, MOD - 1)),
    ]
    rng = random.Random(MOD)
    for name, n in [("rand_00", 10_000), ("rand_01", 100_000), ("mid_00", 10_000_000), ("mid_01", 30_000_000), ("heavy_00", 100_000_000), ("heavy_01", 200_000_000), ("heavy_02", 200_000_000), ("heavy_03", 200_000_000)]:
        s, a, b, c, d = (rng.randrange(MOD) for _ in range(5))
        out.append((name, case(n, s, a, b, c, d)))
    return out


def main() -> None:
    seed = int(sys.argv[1])
    all_cases = cases()
    if not 0 <= seed < len(all_cases):
        raise SystemExit(f"seed {seed} は {len(all_cases)} 未満にしてください")
    print(all_cases[seed][1])


if __name__ == "__main__":
    main()
