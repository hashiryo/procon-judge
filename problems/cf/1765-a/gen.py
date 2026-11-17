"""cf-1765-a (Access Levels) の入力を作る。n と m (どちらも 1 以上 500 以下) と、長さ m の 0 と 1 の列 n 本を出す。

seed が 0 から count - 1 までは本番のケース。問題文の例 2 つ、n = m = 1 の 0 と 1、全部 0 と全部 1、単位行列 (組は m 個)、
階段 (組は 1 つ)、密度を変えたランダム、組の数を決めて埋めた形 (組ごとに開発者の順位を決め、文書ごとに閾値を選ぶ。
組の数は 7 と 50、ノイズを足したもの)、同じ列の繰り返し、開発者が少ない形 (部分集合の束)、m = 1。
seed が 1000 以上なら、文書が 8 個以下、開発者が 6 人以下の入力を出す (愚直解 brute.cpp 用)。
"""

import random
import sys

N = 500


def emit(rows: list[list[int]]) -> None:
    n, m = len(rows), len(rows[0])
    assert 1 <= n <= N and 1 <= m <= N and all(len(r) == m for r in rows)
    print(n, m)
    for r in rows:
        print("".join(map(str, r)))


def planted(rng: random.Random, n: int, m: int, k: int, noise: float = 0.0) -> list[list[int]]:
    """組を k 個決め、組ごとに開発者の順位を混ぜ、文書ごとに組と閾値を選んで、順位が閾値以上の開発者だけが見られるようにする。"""
    ranks = []
    for _ in range(k):
        r = list(range(n))
        rng.shuffle(r)
        ranks.append(r)
    rows = [[0] * m for _ in range(n)]
    for j in range(m):
        g = rng.randrange(k)
        t = rng.randint(0, n)
        for i in range(n):
            if ranks[g][i] >= t:
                rows[i][j] = 1
    for i in range(n):
        for j in range(m):
            if rng.random() < noise:
                rows[i][j] ^= 1
    return rows


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 1000:
        n = rng.randint(1, 6)
        m = rng.randint(1, 8)
        if rng.random() < 0.5:
            rows = planted(rng, n, m, rng.randint(1, 3), noise=rng.choice([0.0, 0.0, 0.1]))
        else:
            p = rng.random()
            rows = [[int(rng.random() < p) for _ in range(m)] for _ in range(n)]
        emit(rows)
        return
    if seed == 0:
        print("3 2\n01\n11\n10")
        return
    if seed == 1:
        print("2 3\n101\n100")
        return
    if seed == 2:
        rows = [[0]]
    elif seed == 3:
        rows = [[1]]
    elif seed == 4:
        rows = [[0] * N for _ in range(N)]
    elif seed == 5:
        rows = [[1] * N for _ in range(N)]
    elif seed == 6:
        rows = [[int(i == j) for j in range(N)] for i in range(N)]
    elif seed == 7:
        perm = list(range(N))
        rng.shuffle(perm)
        rows = [[int(i <= perm[j]) for j in range(N)] for i in range(N)]
    elif seed in (8, 9):
        p = 0.5 if seed == 8 else 0.97
        rows = [[int(rng.random() < p) for _ in range(N)] for _ in range(N)]
    elif seed == 10:
        rows = planted(rng, N, N, 7)
    elif seed == 11:
        rows = planted(rng, N, N, 50)
    elif seed == 12:
        rows = planted(rng, N, N, 20, noise=0.002)
    elif seed == 13:
        base = [[int(rng.random() < 0.6) for _ in range(N)] for _ in range(20)]
        cols = [rng.randrange(20) for _ in range(N)]
        rows = [[base[c][i] for c in cols] for i in range(N)]
    elif seed == 14:
        rows = [[int(rng.random() < 0.5) for _ in range(N)] for _ in range(5)]
    elif seed == 15:
        rows = [[int(rng.random() < 0.5) for _ in range(N)] for _ in range(9)]
    else:
        rows = [[int(rng.random() < 0.5)] for _ in range(N)]
    emit(rows)


main()
