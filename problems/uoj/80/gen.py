#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""uoj-80 (二分图最大权匹配) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

1 ≤ n_l, n_r ≤ 400、1 ≤ m ≤ 160000、1 ≤ w ≤ 10^9、同じ組は 1 本 (問題文の約束)。seed 0 は問題文の例。seed 1 から 9 は、
400 × 400 の全部の組 (重みが 10^9 以下の乱択、10 以下の乱択、どれも 10^9)、疎な乱択 (2000 本)、女が 1 人、男が 1 人、
400 × 300 で 12 万本の乱択、対角だけ重い形、重みが (i + 1)(j + 1) の形。組の順は乱択に並べ替える。seed 1000 以上は、愚直解
(brute.hpp) と突き合わせる小さい入力 (どちらも 6 人まで)。
"""
import random
import sys

SAMPLE = """2 2 3
1 1 100
1 2 1
2 1 1"""


def out(rng: random.Random, nl: int, nr: int, es: dict[tuple[int, int], int]) -> str:
    items = list(es.items())
    rng.shuffle(items)
    assert 1 <= nl <= 400 and 1 <= nr <= 400 and 1 <= len(items) <= 160000
    lines = [f"{nl} {nr} {len(items)}"]
    for (v, u), w in items:
        assert 1 <= w <= 10**9
        lines.append(f"{v + 1} {u + 1} {w}")
    return "\n".join(lines)


def full(nl: int, nr: int, w) -> dict[tuple[int, int], int]:
    return {(v, u): w(v, u) for v in range(nl) for u in range(nr)}


def main_case(seed: int) -> str:
    if seed == 0:
        return SAMPLE
    rng = random.Random(seed * 7919 + 47)
    if seed == 1:
        return out(rng, 400, 400, full(400, 400, lambda v, u: rng.randint(1, 10**9)))
    if seed == 2:
        return out(rng, 400, 400, full(400, 400, lambda v, u: rng.randint(1, 10)))
    if seed == 3:
        es: dict[tuple[int, int], int] = {}
        while len(es) < 2000:
            es[(rng.randrange(400), rng.randrange(400))] = rng.randint(1, 10**9)
        return out(rng, 400, 400, es)
    if seed == 4:
        return out(rng, 400, 1, full(400, 1, lambda v, u: rng.randint(1, 10**9)))
    if seed == 5:
        return out(rng, 1, 400, full(1, 400, lambda v, u: rng.randint(1, 10**9)))
    if seed == 6:
        es = {}
        while len(es) < 120000:
            es[(rng.randrange(400), rng.randrange(300))] = rng.randint(1, 10**9)
        return out(rng, 400, 300, es)
    if seed == 7:
        return out(rng, 400, 400, full(400, 400, lambda v, u: 10**9))
    if seed == 8:
        return out(rng, 400, 400, full(400, 400, lambda v, u: 10**9 if v == u else rng.randint(1, 1000)))
    if seed == 9:
        return out(rng, 400, 400, full(400, 400, lambda v, u: (v + 1) * (u + 1)))
    raise SystemExit(f"seed {seed} は 0 から 9 か、1000 以上にしてください")


def small_case(seed: int) -> str:
    rng = random.Random(seed * 104729 + 7)
    nl, nr = rng.randint(1, 6), rng.randint(1, 6)
    hi = rng.choice((1, 10, 10**9))
    es: dict[tuple[int, int], int] = {}
    for _ in range(rng.randint(1, nl * nr)):
        es[(rng.randrange(nl), rng.randrange(nr))] = rng.randint(1, hi)
    return out(rng, nl, nr, es)


def main() -> None:
    seed = int(sys.argv[1])
    print(small_case(seed) if seed >= 1000 else main_case(seed))


if __name__ == "__main__":
    main()
