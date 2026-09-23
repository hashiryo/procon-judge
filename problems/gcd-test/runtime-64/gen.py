#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""64 bit 整数の gcd の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

期待出力は参照実装 (problem.toml の reference) がハーネスと一緒に作るので、ここでは入力だけを書く。
旧 judge の gcd-test/runtime/runtime-64/gen/make_inputs.py の手書きケースと乱数ケースを、その順番のまま seed に割り当てた。
乱数の種も同じなので、中身は旧 judge の testcases/*.in と一致する。

入力: N abits sa sb ((a_i, b_i) は LCG で n 個作る)
"""
import random
import sys

U64_MAX = (1 << 64) - 1


def case(n: int, abits: int, sa: int, sb: int) -> str:
    assert 1 <= abits <= 64
    return f"{n} {abits} {sa & U64_MAX} {sb & U64_MAX}"


def cases() -> list[tuple[str, str]]:
    out = [
        ("small_00", case(0, 64, 1, 1)),
        ("small_01", case(1, 64, 1, 1)),
        ("small_02", case(5, 64, 1, 1)),
        ("edge_small", case(100_000, 16, 1, 1)),
        ("edge_mid", case(100_000, 32, 1, 1)),
        ("edge_large", case(100_000, 64, 1, 1)),
    ]
    rng = random.Random(32)
    for name, n, abits in [("rand_00", 100_000, 64), ("rand_01", 1_000_000, 64), ("mid_00", 5_000_000, 64),
                           ("mid_01", 5_000_000, 32), ("heavy_00", 10_000_000, 64), ("heavy_01", 10_000_000, 64)]:
        sa, sb = (rng.randrange(U64_MAX + 1) for _ in range(2))
        out.append((name, case(n, abits, sa, sb)))
    return out


def main() -> None:
    seed = int(sys.argv[1])
    all_cases = cases()
    if not 0 <= seed < len(all_cases):
        raise SystemExit(f"seed {seed} は {len(all_cases)} 未満にしてください")
    print(all_cases[seed][1])


if __name__ == "__main__":
    main()
