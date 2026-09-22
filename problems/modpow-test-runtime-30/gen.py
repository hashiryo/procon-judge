#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""べき乗 a^b mod p (p は入力、30 bit の素数) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

期待出力は参照実装 (problem.toml の reference) がハーネスと一緒に作るので、ここでは入力だけを書く。
旧 judge の modpow-test/runtime/runtime-30/gen/make_inputs.py の手書きケースと乱数ケースを、その順番のまま seed に割り当てた。
乱数の種も同じなので、中身は旧 judge の testcases/*.in と一致する。

入力: N mod bbits sa sb ((a_i, b_i) は LCG で n 個作る)
"""
import random
import sys

U64_MAX = (1 << 64) - 1
# 30 bit の素数。998244353 (= 119 * 2^23 + 1) を中心に使う。
PRIMES_30BIT = [998244353, 1000000007, 1000000009, 924844033, 985661441, 754974721, 167772161]
DEFAULT_PRIME = 998244353


def case(n: int, mod: int, bbits: int, sa: int, sb: int) -> str:
    assert 1 <= bbits <= 32
    return f"{n} {mod} {bbits} {sa & U64_MAX} {sb & U64_MAX}"


def cases() -> list[tuple[str, str]]:
    out = [
        ("small_00", case(0, DEFAULT_PRIME, 30, 1, 1)),
        ("small_01", case(1, DEFAULT_PRIME, 30, 1, 1)),
        ("small_02", case(5, DEFAULT_PRIME, 30, 1, 1)),
        ("edge_b_min", case(100_000, DEFAULT_PRIME, 1, 1, 1)),
        ("edge_b_short", case(100_000, DEFAULT_PRIME, 16, 1, 1)),
        ("edge_b_med", case(100_000, DEFAULT_PRIME, 30, 1, 1)),
        ("edge_b_max", case(100_000, DEFAULT_PRIME, 32, 1, 1)),
    ]
    rng = random.Random(32)
    # b は 30 bit (p - 1 と同程度) と 32 bit (フェルマーの小定理で削れる) の両方。
    for name, n, bbits in [("rand_00", 10_000, 30), ("rand_01", 100_000, 30), ("mid_00", 1_000_000, 30),
                           ("mid_01", 1_000_000, 32), ("heavy_00", 5_000_000, 30), ("heavy_01", 5_000_000, 32)]:
        mod = rng.choice(PRIMES_30BIT)
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
