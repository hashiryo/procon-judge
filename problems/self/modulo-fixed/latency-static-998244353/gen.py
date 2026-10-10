#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""決まった値を掛ける剰余の反復 (mod = 998244353 固定) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

期待出力は参照実装 (problem.toml の reference) がハーネスと一緒に作るので、ここでは入力だけを書く。
並びと大きさは latency-runtime-30 と同じで、法だけを 998244353 に固定した。法はハーネスがコンパイル時の定数として持つ。

入力: N s w b (s = s * w + b を N 回くり返す)
"""
import random
import sys

MOD = 998244353


def case(n: int, s: int, w: int, b: int) -> str:
    return f"{n} {s % MOD} {w % MOD} {b % MOD}"


def cases() -> list[tuple[str, str]]:
    out = [
        ("small_00", case(0, 0, 0, 0)),
        ("small_01", case(1, 1, 0, 0)),
        ("small_02", case(5, 1, 1, 1)),
        ("edge_00", case(20, MOD - 1, MOD - 1, 0)),
        ("edge_01", case(20, MOD - 1, 1, MOD - 2)),
        ("edge_02", case(100, 123456789, MOD - 1, MOD - 1)),
    ]
    rng = random.Random(42)
    for name, n in [("rand_00", 10_000), ("rand_01", 100_000), ("mid_00", 10_000_000), ("mid_01", 30_000_000), ("heavy_00", 100_000_000), ("heavy_01", 200_000_000), ("heavy_02", 200_000_000)]:
        s, w, b = (rng.randrange(MOD) for _ in range(3))
        out.append((name, case(n, s, w, b)))
    return out


def main() -> None:
    seed = int(sys.argv[1])
    all_cases = cases()
    if not 0 <= seed < len(all_cases):
        raise SystemExit(f"seed {seed} は {len(all_cases)} 未満にしてください")
    print(all_cases[seed][1])


if __name__ == "__main__":
    main()
