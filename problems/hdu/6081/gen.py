#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""hdu-6081 (度度熊的王国战略) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

どのケースも何組かのテストデータを並べる (2 ≤ n ≤ 3000、1 ≤ m ≤ 10^5、1 ≤ w ≤ 1000、両端の同じ辺と、同じ組を結ぶ辺が
何本あってもよい)。seed 0 は問題文の例。seed 1 から 11 は、n = 3000 で m = 10^5 の乱択 (連結)、2 つの塊を少ない辺で
つないだ形、ほぼ完全なグラフ (n = 447、重みがどれも 1)、木に辺を足した疎な形、つながっていない形 (答えは 0)、輪に弦を
足した形、50 × 60 の格子、自己ループと多重辺の多い乱択、100 頂点の塊 30 個を輪につないだ形、重みがどれも 1 の乱択、
n = 2 と小さい組の詰め合わせ。頂点の番号と辺の順は乱択に並べ替える。seed 1000 以上は、愚直解 (brute.hpp) と突き合わせる
小さい入力 (頂点 10 個までの組を 1 から 4 組)。
"""
import random
import sys

SAMPLE = """2 1
1 2 1
3 3
1 2 5
1 2 4
2 3 3"""


def test(rng: random.Random, n: int, es: list[tuple[int, int, int]]) -> str:
    p = list(range(n))
    rng.shuffle(p)
    rng.shuffle(es)
    assert 2 <= n <= 3000 and 1 <= len(es) <= 10**5
    lines = [f"{n} {len(es)}"]
    for u, v, w in es:
        assert 1 <= w <= 1000
        if rng.random() < 0.5:
            u, v = v, u
        lines.append(f"{p[u] + 1} {p[v] + 1} {w}")
    return "\n".join(lines)


def tree(rng: random.Random, n: int, w) -> list[tuple[int, int, int]]:
    return [(v, rng.randrange(v), w(rng)) for v in range(1, n)]


def pair(rng: random.Random, lo: int, hi: int) -> tuple[int, int]:
    a = rng.randrange(lo, hi)
    b = rng.randrange(lo, hi - 1)
    return a, b + (b >= a)


def make(rng: random.Random, kind: int) -> str:
    n, m = 3000, 10**5
    w1k = lambda r: r.randint(1, 1000)
    if kind == 1:
        es = tree(rng, n, w1k)
        es += [(*pair(rng, 0, n), w1k(rng)) for _ in range(m - len(es))]
    elif kind == 2:
        h = n // 2
        es = tree(rng, h, w1k) + [(h + a, h + b, w) for a, b, w in tree(rng, n - h, w1k)]
        while len(es) < m - 5:
            lo, hi = (0, h) if rng.random() < 0.5 else (h, n)
            es.append((*pair(rng, lo, hi), w1k(rng)))
        es += [(rng.randrange(h), rng.randrange(h, n), rng.randint(1, 50)) for _ in range(5)]
    elif kind == 3:
        n = 447
        es = [(u, v, 1) for u in range(n) for v in range(u + 1, n)][:m]
    elif kind == 4:
        es = tree(rng, n, w1k)
        es += [(*pair(rng, 0, n), w1k(rng)) for _ in range(n // 2)]
    elif kind == 5:
        h = rng.randint(1, n - 1)
        es = []
        while len(es) < m:
            lo, hi = (0, h) if rng.random() < h / n else (h, n)
            if hi - lo >= 2:
                es.append((*pair(rng, lo, hi), w1k(rng)))
    elif kind == 6:
        es = [(v, (v + 1) % n, w1k(rng)) for v in range(n)]
        es += [(v, (v + rng.randint(2, 30)) % n, rng.randint(1, 10)) for v in (rng.randrange(n) for _ in range(m - n))]
    elif kind == 7:
        R, C = 50, 60
        es = []
        for i in range(R):
            for j in range(C):
                if j + 1 < C:
                    es.append((i * C + j, i * C + j + 1, w1k(rng)))
                if i + 1 < R:
                    es.append((i * C + j, (i + 1) * C + j, w1k(rng)))
    elif kind == 8:
        es = tree(rng, n, w1k)
        base = [(*pair(rng, 0, n), w1k(rng)) for _ in range(m // 4)]
        es += base
        while len(es) < m:
            if rng.random() < 0.2:
                v = rng.randrange(n)
                es.append((v, v, w1k(rng)))
            else:
                u, v, _ = rng.choice(base)
                es.append((u, v, w1k(rng)))
    elif kind == 9:
        es = []
        for c in range(30):
            lo = 100 * c
            es += [(lo + a, lo + b, w) for a, b, w in tree(rng, 100, w1k)]
            es += [(*pair(rng, lo, lo + 100), w1k(rng)) for _ in range(3000)]
            nxt = 100 * ((c + 1) % 30)
            es += [(rng.randrange(lo, lo + 100), rng.randrange(nxt, nxt + 100), w1k(rng)) for _ in range(30)]
    elif kind == 10:
        es = tree(rng, n, lambda r: 1)
        es += [(*pair(rng, 0, n), 1) for _ in range(m - len(es))]
    else:
        raise ValueError(kind)
    return test(rng, n, es)


def main_case(seed: int) -> str:
    if seed == 0:
        return SAMPLE
    rng = random.Random(seed * 7919 + 37)
    if 1 <= seed <= 10:
        reps = 2 if seed == 3 else 4
        return "\n".join(make(rng, seed) for _ in range(reps))
    if seed == 11:
        tests = [test(rng, 2, [(0, 1, rng.randint(1, 1000))]), test(rng, 2, [(0, 0, 5), (1, 1, 7)])]
        for _ in range(20):
            k = rng.randint(2, 30)
            es = [(*pair(rng, 0, k), rng.randint(1, 1000)) for _ in range(rng.randint(1, 60))]
            tests.append(test(rng, k, es))
        return "\n".join(tests)
    raise SystemExit(f"seed {seed} は 0 から 11 か、1000 以上にしてください")


def small_case(seed: int) -> str:
    rng = random.Random(seed * 104729 + 7)
    tests = []
    for _ in range(rng.randint(1, 4)):
        k = rng.randint(2, 10)
        hi = rng.choice((1, 10, 1000))
        es = []
        for _ in range(rng.randint(1, 3 * k)):
            u, v = rng.randrange(k), rng.randrange(k)
            es.append((u, v, rng.randint(1, hi)))
        tests.append(test(rng, k, es))
    return "\n".join(tests)


def main() -> None:
    seed = int(sys.argv[1])
    print(small_case(seed) if seed >= 1000 else main_case(seed))


if __name__ == "__main__":
    main()
