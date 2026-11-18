#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""poj-2914 (Minimum Cut) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

どのケースも何組かのテストケースを並べる (2 ≤ N ≤ 500、同じ組は 1 行、頂点は 0 始まり、C ≥ 1)。seed 0 は問題文の例。
seed 1 から 7 は、N = 500 の完全グラフ (C が 100 以下の乱択と、どれも 1)、疎な乱択 (連結)、つながっていない形と辺の無い形
(答えは 0)、2 つの密な塊を少ない辺でつないだ形、木と輪、小さい組の詰め合わせ。seed 1000 以上は、愚直解 (brute.hpp) と
突き合わせる小さい入力 (頂点 10 個までの組を 1 から 4 組)。
"""
import random
import sys

SAMPLE = """3 3
0 1 1
1 2 1
2 0 1
4 3
0 1 1
1 2 1
2 3 1
8 14
0 1 1
0 2 1
0 3 1
1 2 1
1 3 1
2 3 1
4 5 1
4 6 1
4 7 1
5 6 1
5 7 1
6 7 1
4 0 1
7 3 1"""


def test(rng: random.Random, n: int, es: dict[tuple[int, int], int]) -> str:
    p = list(range(n))
    rng.shuffle(p)
    items = list(es.items())
    rng.shuffle(items)
    assert 2 <= n <= 500 and len(items) <= n * (n - 1) // 2
    lines = [f"{n} {len(items)}"]
    for (u, v), c in items:
        if rng.random() < 0.5:
            u, v = v, u
        lines.append(f"{p[u]} {p[v]} {c}")
    return "\n".join(lines)


def add(es: dict[tuple[int, int], int], u: int, v: int, c: int) -> None:
    if u != v and (min(u, v), max(u, v)) not in es:
        es[(min(u, v), max(u, v))] = c


def make(rng: random.Random, kind: int, n: int = 500) -> str:
    es: dict[tuple[int, int], int] = {}
    if kind in (1, 5):
        for u in range(n):
            for v in range(u + 1, n):
                add(es, u, v, rng.randint(1, 100) if kind == 1 else 1)
    elif kind == 2:
        for v in range(1, n):
            add(es, v, rng.randrange(v), rng.randint(1, 10))
        while len(es) < 2000:
            add(es, rng.randrange(n), rng.randrange(n), rng.randint(1, 10))
    elif kind == 3:
        if rng.random() < 0.3:
            return test(rng, n, {})
        h = rng.randint(1, n - 1)
        while len(es) < 3000:
            lo, hi = (0, h) if rng.random() < h / n else (h, n)
            if hi - lo >= 2:
                add(es, rng.randrange(lo, hi), rng.randrange(lo, hi), rng.randint(1, 10))
    elif kind == 4:
        h = n // 2
        for lo, hi in ((0, h), (h, n)):
            for u in range(lo, hi):
                for v in range(u + 1, hi):
                    if rng.random() < 0.3:
                        add(es, u, v, rng.randint(1, 20))
        for _ in range(8):
            add(es, rng.randrange(h), rng.randrange(h, n), rng.randint(1, 20))
    elif kind == 6:
        for v in range(1, n):
            add(es, v, rng.randrange(v) if rng.random() < 0.5 else v - 1, rng.randint(1, 100))
        if rng.random() < 0.5:
            add(es, 0, n - 1, rng.randint(1, 100))
    else:
        raise ValueError(kind)
    return test(rng, n, es)


def main_case(seed: int) -> str:
    if seed == 0:
        return SAMPLE
    rng = random.Random(seed * 7919 + 41)
    if seed in (1, 5):
        return "\n".join(make(rng, seed) for _ in range(4))
    if seed in (2, 3, 4):
        return "\n".join(make(rng, seed) for _ in range(5))
    if seed == 6:
        return "\n".join(make(rng, 6) for _ in range(10))
    if seed == 7:
        tests = []
        for _ in range(30):
            k = rng.randint(2, 30)
            es: dict[tuple[int, int], int] = {}
            for _ in range(rng.randint(0, k * (k - 1) // 2)):
                add(es, rng.randrange(k), rng.randrange(k), rng.randint(1, 100))
            tests.append(test(rng, k, es))
        return "\n".join(tests)
    raise SystemExit(f"seed {seed} は 0 から 7 か、1000 以上にしてください")


def small_case(seed: int) -> str:
    rng = random.Random(seed * 104729 + 7)
    tests = []
    for _ in range(rng.randint(1, 4)):
        k = rng.randint(2, 10)
        es: dict[tuple[int, int], int] = {}
        for _ in range(rng.randint(0, k * (k - 1) // 2)):
            add(es, rng.randrange(k), rng.randrange(k), rng.randint(1, rng.choice((1, 10, 100))))
        tests.append(test(rng, k, es))
    return "\n".join(tests)


def main() -> None:
    seed = int(sys.argv[1])
    print(small_case(seed) if seed >= 1000 else main_case(seed))


if __name__ == "__main__":
    main()
