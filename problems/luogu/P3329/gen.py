#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""洛谷 P3329 ([ZJOI2011] 最小割) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

seed 0 から 9 は本番のケースで、どれもグラフ 10 個 (n = 150、m = 3000、多重辺あり、容量は 0 以上 10^6 以下) と、
グラフごとに問い合わせ 30 個。グラフの形は seed とグラフの番号で回す (乱択、塊、輪と弦、容量の小さい乱択、中心、
道に近い形)。問い合わせは 0、2^31 - 1、[0, 4 × 10^7] の乱択。連結とは限らない。seed 1000 以上は愚直解 (brute.hpp) と
突き合わせる小さい入力で、グラフは 1 個から 3 個、頂点は 12 個まで。
"""
import random
import sys

SHAPES = ["random", "cluster", "cycle", "small", "hub", "path"]


def graph(rng: random.Random, shape: str, n: int, m: int, cmax: int) -> list[tuple[int, int, int]]:
    e: list[tuple[int, int, int]] = []

    def pair(lo: int, hi: int) -> tuple[int, int]:
        u = rng.randrange(lo, hi)
        v = rng.randrange(lo, hi - 1)
        return u, v + (v >= u)

    if n < 2:
        return e
    if shape == "cluster":
        k = max(1, n // 30)
        while len(e) < m:
            if rng.random() < 0.02 or k == 1:
                u, v = pair(0, n)
            else:
                c = rng.randrange(k)
                lo, hi = c * n // k, (c + 1) * n // k
                if hi - lo < 2:
                    continue
                u, v = pair(lo, hi)
            e.append((u, v, rng.randint(0, cmax)))
    elif shape == "cycle":
        for v in range(n):
            if n > 2 or v == 0:
                e.append((v, (v + 1) % n, rng.randint(cmax // 2, cmax)))
        while len(e) < m:
            u, v = pair(0, n)
            e.append((u, v, rng.randint(0, cmax // 50)))
    elif shape == "hub":
        h = min(3, n - 1)
        while len(e) < m:
            if rng.random() < 0.5:
                e.append((rng.randrange(h), rng.randrange(h, n), rng.randint(0, cmax)))
            else:
                u, v = pair(0, n)
                e.append((u, v, rng.randint(0, cmax // 10)))
    elif shape == "path":
        for v in range(1, n):
            e.append((v - 1, v, rng.randint(0, cmax)))
        while len(e) < m:
            u = rng.randrange(n)
            v = min(n - 1, u + rng.randint(1, 4))
            if u != v:
                e.append((u, v, rng.randint(0, cmax)))
    else:
        c = 3 if shape == "small" else cmax
        while len(e) < m:
            u, v = pair(0, n)
            e.append((u, v, rng.randint(0, c)))
    p = list(range(n))
    rng.shuffle(p)
    out = [(p[u] + 1, p[v] + 1, w) if rng.random() < 0.5 else (p[v] + 1, p[u] + 1, w) for u, v, w in e]
    rng.shuffle(out)
    return out


def make(seed: int, tests: list[tuple[str, int, int, int]], q: int, xmax: int) -> str:
    rng = random.Random(seed)
    lines = [str(len(tests))]
    for shape, n, m, cmax in tests:
        e = graph(rng, shape, n, m, cmax)
        lines.append(f"{n} {len(e)}")
        lines += [f"{u} {v} {w}" for u, v, w in e]
        lines.append(str(q))
        for i in range(q):
            lines.append(str(0 if i == 0 else 2147483647 if i == 1 else rng.randint(0, xmax)))
    return "\n".join(lines)


def main() -> None:
    seed = int(sys.argv[1])
    if 0 <= seed < 10:
        tests = [(SHAPES[(seed + i) % len(SHAPES)], 150, 3000, 10**6) for i in range(10)]
        print(make(seed, tests, 30, 4 * 10**7))
    elif seed >= 1000:
        rng = random.Random(seed * 7919 + 5)
        tests = [(rng.choice(SHAPES), rng.randint(1, 12), rng.randint(0, 30), rng.randint(0, 6)) for _ in range(rng.randint(1, 3))]
        print(make(seed, tests, 10, 40))
    else:
        raise SystemExit(f"seed {seed} は 0 から 9 か、1000 以上にしてください")


if __name__ == "__main__":
    main()
