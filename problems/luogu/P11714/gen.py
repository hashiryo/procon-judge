#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""洛谷 P11714 / UOJ 37 (主旋律) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

1 行目に n m、続く m 行に a b (1 始まり、a → b)。自己ループと同じ向きの多重辺は無い (逆向きの 2 本はよい)。
seed 0 から 11 は本番のケース: 0 は n = 15 の完全な有向グラフ (m = 210)、1 から 3 は n = 15 で辺を確率 0.7、0.4、0.2 で
置いたもの、4 は n = 15 の閉路に辺を足したもの、5 は n = 15 で両向きの辺を組で置いたもの、6 は n = 14 の完全な有向グラフ、
7 は n = 15 で辺の無いもの (答え 0)、8 は n = 1、9 は n = 2 の 2 本、10 は n = 13 で確率 0.5、11 は n = 15 で辺を確率 0.1
で置いたもの (強連結にならないことが多い)。頂点の番号は乱択で並べ替える。
seed 1000 以上は、頂点が 6 個以下、辺が 16 本以下の入力を出す (愚直解 brute.hpp 用)。
乱数は Python の random の版による違いを避けるため splitmix64 で作る。
"""
import sys

MASK64 = (1 << 64) - 1


class SplitMix64:
    def __init__(self, seed: int) -> None:
        self.x = seed & MASK64

    def next(self) -> int:
        self.x = (self.x + 0x9E3779B97F4A7C15) & MASK64
        z = self.x
        z = ((z ^ (z >> 30)) * 0xBF58476D1CE4E5B9) & MASK64
        z = ((z ^ (z >> 27)) * 0x94D049BB133111EB) & MASK64
        return z ^ (z >> 31)

    def below(self, n: int) -> int:
        return self.next() % n

    def prob(self, p: float) -> bool:
        return self.next() < int(p * (1 << 64))


def emit(rng: SplitMix64, n: int, edges: list[tuple[int, int]]) -> str:
    perm = list(range(n))
    for i in range(n - 1, 0, -1):
        j = rng.below(i + 1)
        perm[i], perm[j] = perm[j], perm[i]
    edges = [(perm[a], perm[b]) for a, b in edges]
    for i in range(len(edges) - 1, 0, -1):
        j = rng.below(i + 1)
        edges[i], edges[j] = edges[j], edges[i]
    lines = [f"{n} {len(edges)}"] + [f"{a + 1} {b + 1}" for a, b in edges]
    return "\n".join(lines)


def complete(n: int) -> list[tuple[int, int]]:
    return [(a, b) for a in range(n) for b in range(n) if a != b]


def random_edges(rng: SplitMix64, n: int, p: float) -> list[tuple[int, int]]:
    return [(a, b) for a, b in complete(n) if rng.prob(p)]


def main() -> None:
    seed = int(sys.argv[1])
    rng = SplitMix64(seed * 1000003 + 37)
    if seed >= 1000:
        n = 1 + rng.below(6)
        cand = complete(n)
        for i in range(len(cand) - 1, 0, -1):
            j = rng.below(i + 1)
            cand[i], cand[j] = cand[j], cand[i]
        m = rng.below(min(16, len(cand)) + 1)
        print(emit(rng, n, cand[:m]))
        return
    if seed == 0:
        n, edges = 15, complete(15)
    elif seed in (1, 2, 3):
        n, edges = 15, random_edges(rng, 15, {1: 0.7, 2: 0.4, 3: 0.2}[seed])
    elif seed == 4:
        n = 15
        es = {(i, (i + 1) % n) for i in range(n)}
        for a, b in complete(n):
            if rng.prob(0.15):
                es.add((a, b))
        edges = sorted(es)
    elif seed == 5:
        n, edges = 15, []
        for a in range(15):
            for b in range(a + 1, 15):
                if rng.prob(0.5):
                    edges += [(a, b), (b, a)]
    elif seed == 6:
        n, edges = 14, complete(14)
    elif seed == 7:
        n, edges = 15, []
    elif seed == 8:
        n, edges = 1, []
    elif seed == 9:
        n, edges = 2, [(0, 1), (1, 0)]
    elif seed == 10:
        n, edges = 13, random_edges(rng, 13, 0.5)
    elif seed == 11:
        n, edges = 15, random_edges(rng, 15, 0.1)
    else:
        raise SystemExit(f"unknown seed {seed}")
    print(emit(rng, n, edges))


if __name__ == "__main__":
    main()
