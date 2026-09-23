#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""べき乗 a^b mod m (m は入力、2^40 < m < 2^62 の奇数) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

期待出力は参照実装 (problem.toml の reference) がハーネスと一緒に作るので、ここでは入力だけを書く。
旧 judge の modpow-test/runtime/runtime-62/gen/make_inputs.py の手書きケースと乱数ケースを、その順番のまま seed に割り当てた。
乱数の種も同じなので、中身は旧 judge の testcases/*.in と一致する。

入力: N mod bbits sa sb ((a_i, b_i) は LCG で n 個作る)
"""
import random
import sys

U64_MAX = (1 << 64) - 1
MIN_MOD = (1 << 40) + 1
MAX_MOD = (1 << 62) - 1


def normalize_mod(mod: int) -> int:
    mod = max(mod, MIN_MOD)
    mod = min(mod, MAX_MOD)
    if mod % 2 == 0:
        mod += 1 if mod < MAX_MOD else -1
    return mod


def case(n: int, mod: int, bbits: int, sa: int, sb: int) -> str:
    mod = normalize_mod(mod)
    assert 1 <= bbits <= 64
    return f"{n} {mod} {bbits} {sa & U64_MAX} {sb & U64_MAX}"


def cases() -> list[tuple[str, str]]:
    out = [
        ("small_00", case(0, MIN_MOD, 60, 1, 1)),
        ("small_01", case(1, MIN_MOD, 60, 1, 1)),
        ("small_02", case(5, MIN_MOD, 60, 1, 1)),
        ("edge_b_min", case(100_000, MAX_MOD, 1, 1, 1)),
        ("edge_b_short", case(100_000, MAX_MOD, 32, 1, 1)),
        ("edge_b_max", case(100_000, MAX_MOD, 64, 1, 1)),
        ("edge_mod_min", case(100_000, MIN_MOD, 60, 1, 1)),
        ("edge_mod_max", case(100_000, MAX_MOD, 60, 1, 1)),
    ]
    rng = random.Random(32)
    # b は 60 bit 固定で、内部の mul はおよそ 60 n 回。
    for name, n, bbits in [("rand_00", 10_000, 60), ("rand_01", 100_000, 60), ("mid_00", 1_000_000, 60),
                           ("mid_01", 2_000_000, 60), ("heavy_00", 5_000_000, 60), ("heavy_01", 5_000_000, 60)]:
        mod = normalize_mod(rng.randrange(MIN_MOD, MAX_MOD + 1))
        sa, sb = (rng.randrange(U64_MAX + 1) for _ in range(2))
        out.append((name, case(n, mod, bbits, sa, sb)))
    return out


def main() -> None:
    seed = int(sys.argv[1])
    all_cases = cases()
    if not 0 <= seed < len(all_cases):
        raise SystemExit(f"seed {seed} は {len(all_cases)} 未満にしてください")
    print(all_cases[seed][1])


if __name__ == "__main__":
    main()
