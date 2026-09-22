#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""1 語の剰余 a % b (b = 998244353 固定) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

期待出力は参照実装 (problem.toml の reference) がハーネスと一緒に作るので、ここでは入力だけを書く。
旧 judge の 1word-mod/static/1word-998244353/gen/make_inputs.py の手書きケースと乱数ケースを、その順番のまま seed に割り当てた。
乱数の種も同じなので、中身は旧 judge の testcases/*.in と一致する。

入力: N sa ma mb (a_i は LCG で n 個作る)
"""
import random
import sys

U64_MAX = (1 << 64) - 1
LCG_MA, LCG_MB = 6364136223846793005, 1442695040888963407


def case(n: int, sa: int, ma: int, mb: int) -> str:
    return f"{n} {sa & U64_MAX} {ma & U64_MAX} {mb & U64_MAX}"


def cases() -> list[tuple[str, str]]:
    out = [
        ("small_00", case(0, 0, 0, 0)),
        ("small_01", case(1, 1, 1, 0)),
        ("small_02", case(5, 1, LCG_MA, LCG_MB)),
    ]
    rng = random.Random(998244353)
    for name, n in [("rand_00", 10_000), ("rand_01", 100_000), ("mid_00", 10_000_000), ("mid_01", 30_000_000), ("heavy_00", 100_000_000), ("heavy_01", 200_000_000), ("heavy_02", 200_000_000), ("heavy_03", 200_000_000)]:
        sa, ma, mb = (rng.randrange(U64_MAX + 1) for _ in range(3))
        out.append((name, case(n, sa, ma, mb)))
    return out


def main() -> None:
    seed = int(sys.argv[1])
    all_cases = cases()
    if not 0 <= seed < len(all_cases):
        raise SystemExit(f"seed {seed} は {len(all_cases)} 未満にしてください")
    print(all_cases[seed][1])


if __name__ == "__main__":
    main()
