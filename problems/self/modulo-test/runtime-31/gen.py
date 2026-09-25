#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""剰余の反復 (mod は入力、2^30 < mod < 2^31 の奇数) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

期待出力は参照実装 (problem.toml の reference) がハーネスと一緒に作るので、ここでは入力だけを書く。
旧 judge の modulo-test/runtime/runtime-31/gen/make_inputs.py の手書きケースと乱数ケースを、その順番のまま seed に割り当てた。
乱数の種も同じなので、中身は旧 judge の testcases/*.in と一致する。

入力: N mod s a b c d ((1 << 30) + 1 < mod < (1 << 31) - 1 の奇数)
"""
import random
import sys

MIN_MOD = (1 << 30) + 1
MAX_MOD = (1 << 31) - 1
EVEN = False


def normalize_mod(mod: int) -> int:
    mod = max(mod, MIN_MOD)
    mod = min(mod, MAX_MOD)
    if (mod % 2 == 0) != EVEN:
        mod += 1 if mod < MAX_MOD else -1
    return mod


def case(n: int, mod: int, s: int, a: int, b: int, c: int, d: int) -> str:
    mod = normalize_mod(mod)
    return f"{n} {mod} {s % mod} {a % mod} {b % mod} {c % mod} {d % mod}"


def cases() -> list[tuple[str, str]]:
    out = [
        ("small_00", case(0, MIN_MOD, 0, 0, 0, 0, 0)),
        ("small_01", case(1, MIN_MOD, 1, 0, 0, 0, 0)),
        ("small_02", case(5, MIN_MOD, 1, 1, 1, 1, 1)),
        ("edge_00", case(20, MIN_MOD, MIN_MOD - 1, MIN_MOD - 1, 0, MIN_MOD - 1, 0)),
        ("edge_01", case(20, (1 << 31) - 1, (1 << 31) - 2, 1, (1 << 31) - 2, 1, (1 << 31) - 2)),
        ("edge_02", case(100, MAX_MOD, 123456789, 0, MAX_MOD - 1, 0, MAX_MOD - 1)),
    ]
    rng = random.Random(32)
    for name, n in [("rand_00", 10_000), ("rand_01", 100_000), ("mid_00", 10_000_000), ("mid_01", 30_000_000), ("heavy_00", 100_000_000), ("heavy_01", 200_000_000), ("heavy_02", 200_000_000), ("heavy_03", 200_000_000)]:
        mod = normalize_mod(rng.randrange(MIN_MOD, MAX_MOD + 1))
        s, a, b, c, d = (rng.randrange(mod) for _ in range(5))
        out.append((name, case(n, mod, s, a, b, c, d)))
    return out


def main() -> None:
    seed = int(sys.argv[1])
    all_cases = cases()
    if not 0 <= seed < len(all_cases):
        raise SystemExit(f"seed {seed} は {len(all_cases)} 未満にしてください")
    print(all_cases[seed][1])


if __name__ == "__main__":
    main()
