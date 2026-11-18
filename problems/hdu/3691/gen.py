#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""hdu-3691 (Nubulsa Expo) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

どのケースも何組かのテストケースを並べて 0 0 0 で終える (1 < N ≤ 300、0 < M ≤ 50000、0 < K ≤ 10^6、連結、同じ組を結ぶ辺が
何本あってもよい)。入口 S は乱択。seed 0 は問題文の例。seed 1 から 7 は、N = 300 の完全グラフ (K が乱択と、どれも 1)、
辺 5 万本の多重辺の多い乱択、疎な乱択、2 つの密な塊を少ない辺でつないだ形、木と輪、小さい組の詰め合わせ。seed 1000
以上は、愚直解 (brute.hpp) と突き合わせる小さい入力 (頂点 10 個までの組を 1 から 4 組)。
"""
import random
import sys

SAMPLE = """5 5 1
1 2 5
2 4 6
1 3 7
3 4 3
5 1 10
0 0 0"""


def test(rng: random.Random, n: int, es: list[tuple[int, int, int]]) -> str:
    p = list(range(n))
    rng.shuffle(p)
    rng.shuffle(es)
    assert 1 < n <= 300 and 0 < len(es) <= 50000
    lines = [f"{n} {len(es)} {rng.randint(1, n)}"]
    for u, v, k in es:
        assert 0 < k <= 10**6
        if rng.random() < 0.5:
            u, v = v, u
        lines.append(f"{p[u] + 1} {p[v] + 1} {k}")
    return "\n".join(lines)


def tree(rng: random.Random, n: int, w) -> list[tuple[int, int, int]]:
    return [(v, rng.randrange(v), w(rng)) for v in range(1, n)]


def pair(rng: random.Random, lo: int, hi: int) -> tuple[int, int]:
    a = rng.randrange(lo, hi)
    b = rng.randrange(lo, hi - 1)
    return a, b + (b >= a)


def make(rng: random.Random, kind: int, n: int = 300) -> str:
    big = lambda r: r.randint(1, 10**6)
    if kind in (1, 5):
        es = [(u, v, big(rng) if kind == 1 else 1) for u in range(n) for v in range(u + 1, n)]
    elif kind == 2:
        es = tree(rng, n, big)
        es += [(*pair(rng, 0, n), big(rng)) for _ in range(50000 - len(es))]
    elif kind == 3:
        es = tree(rng, n, big) + [(*pair(rng, 0, n), big(rng)) for _ in range(n)]
    elif kind == 4:
        h = n // 2
        es = tree(rng, h, big) + [(h + a, h + b, w) for a, b, w in tree(rng, n - h, big)]
        while len(es) < 20000:
            lo, hi = (0, h) if rng.random() < 0.5 else (h, n)
            es.append((*pair(rng, lo, hi), big(rng)))
        es += [(rng.randrange(h), rng.randrange(h, n), rng.randint(1, 10**4)) for _ in range(5)]
    elif kind == 6:
        es = [(v, rng.randrange(v) if rng.random() < 0.5 else v - 1, big(rng)) for v in range(1, n)]
        if rng.random() < 0.5:
            es.append((0, n - 1, big(rng)))
    else:
        raise ValueError(kind)
    return test(rng, n, es)


def main_case(seed: int) -> str:
    if seed == 0:
        return SAMPLE
    rng = random.Random(seed * 7919 + 43)
    if 1 <= seed <= 5:
        tests = [make(rng, seed) for _ in range(5)]
    elif seed == 6:
        tests = [make(rng, 6) for _ in range(10)]
    elif seed == 7:
        tests = []
        for _ in range(30):
            k = rng.randint(2, 30)
            es = tree(rng, k, lambda r: r.randint(1, 10**6))
            es += [(*pair(rng, 0, k), rng.randint(1, 10**6)) for _ in range(rng.randint(0, 3 * k))]
            tests.append(test(rng, k, es))
    else:
        raise SystemExit(f"seed {seed} は 0 から 7 か、1000 以上にしてください")
    return "\n".join(tests + ["0 0 0"])


def small_case(seed: int) -> str:
    rng = random.Random(seed * 104729 + 7)
    tests = []
    for _ in range(rng.randint(1, 4)):
        k = rng.randint(2, 10)
        hi = rng.choice((1, 10, 10**6))
        es = tree(rng, k, lambda r: r.randint(1, hi))
        es += [(*pair(rng, 0, k), rng.randint(1, hi)) for _ in range(rng.randint(0, 2 * k))]
        tests.append(test(rng, k, es))
    return "\n".join(tests + ["0 0 0"])


def main() -> None:
    seed = int(sys.argv[1])
    print(small_case(seed) if seed >= 1000 else main_case(seed))


if __name__ == "__main__":
    main()
