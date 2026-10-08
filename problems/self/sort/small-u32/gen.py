#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""短い u32 の配列の並べ替え の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

入力は配列の束の作り方だけで、配列そのものはハーネスが作る (base.cpp と _shared/sort/_common.hpp)。
1 行目が束の個数 G、続く G 行が「本数 分布 N の下限 N の上限 seed 引数」で、引数を使わない分布でも 0 を書く。
束の中の配列は、束の seed から作る splitmix64 の列で N を [下限, 上限] の一様分布から選ぶ。
期待出力は参照実装 (problem.toml の reference) がハーネスと一緒に作るので、ここでは入力だけを書く。
"""
import sys

U32_MAX = (1 << 32) - 1

# seed 0 で、N を 0 から 100 の乱択にして 300 本ずつ作る分布と引数。
SMALL_DISTS = [
    ("range", 255),
    ("range", 1),
    ("few", 1),
    ("few", 2),
    ("few", 16),
    ("equal", 0),
    ("sorted", 0),
    ("reversed", 0),
    ("nearly", 1),
    ("nearly", 3),
    ("mask", 0xFFFF0000),
    ("mask", 0x80000001),
]


def small_case() -> list[tuple[int, str, int, int, int]]:
    """N = 0 から 100 を全域の range で 2 本ずつ並べ、ほかの分布は N を 0 から 100 の乱択にして 300 本ずつ。"""
    rows = [(2, "range", n, n, U32_MAX) for n in range(101)]
    rows += [(300, dist, 0, 100, arg) for dist, arg in SMALL_DISTS]
    return rows


# seed -> 束の並び (本数, 分布, N の下限, N の上限, 引数)。seed 2 から 11 は、どれも要素が 50 万個ほどになる本数にする。
CASES = {
    0: small_case(),
    1: [(65536, "range", 1, 64, U32_MAX)],
    2: [(262144, "range", 2, 2, U32_MAX)],
    3: [(65536, "range", 8, 8, U32_MAX)],
    4: [(32768, "range", 16, 16, U32_MAX)],
    5: [(16384, "range", 32, 32, U32_MAX)],
    6: [(8192, "range", 64, 64, U32_MAX)],
    7: [(65536, "range", 1, 16, U32_MAX)],
    8: [(4096, "range", 65, 256, U32_MAX)],
    9: [(16384, "few", 1, 64, 4)],
    10: [(16384, "sorted", 1, 64, 0)],
    11: [(16384, "reversed", 1, 64, 0)],
}


def main() -> None:
    seed = int(sys.argv[1])
    if seed not in CASES:
        raise SystemExit(f"seed {seed} は {len(CASES)} 未満にしてください")
    rows = CASES[seed]
    out = [str(len(rows))]
    # 束ごとの乱数の種は、ケースの seed と行の番号から決める。
    out += [f"{count} {dist} {lo} {hi} {seed * 1_000_003 + i} {arg}" for i, (count, dist, lo, hi, arg) in enumerate(rows)]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
