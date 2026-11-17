"""luogu-P4298 ([CTSC2008] 祭祀) の入力を作る。N (1 以上 100 以下)、M (0 以上 1000 以下) と、閉路の無い有向辺 M 本を出す。

seed が 0 から count - 1 までは本番のケース。問題文の例、N = 1、辺の無い N = 100、1 本の道、完全二部グラフ (左から右)、
10 × 10 の格子 (右と下)、外向きの木と内向きの木、層の幅を変えた層状のグラフ、2 本の鎖の間に辺を足したもの、密度を変えた
ランダム (辺 1000 本まで、多重辺も混ぜる)。頂点の番号は混ぜ、トポロジカル順が番号の順にならないようにする。
seed が 1000 以上なら、頂点が 12 個以下の入力を出す (愚直解 brute.cpp 用)。
"""

import random
import sys

N_MAX = 100
M_MAX = 1000


def emit(n: int, edges: list[tuple[int, int]], rng: random.Random, relabel: bool = True) -> None:
    assert 1 <= n <= N_MAX and len(edges) <= M_MAX
    perm = list(range(1, n + 1))
    if relabel:
        rng.shuffle(perm)
    es = [(perm[u], perm[v]) for u, v in edges]
    rng.shuffle(es)
    print(n, len(es))
    for u, v in es:
        print(u, v)


def random_dag(rng: random.Random, n: int, m: int, multi: bool = False) -> list[tuple[int, int]]:
    """頂点 0..n-1 の順をトポロジカル順として、u < v の辺を m 本選ぶ。"""
    pairs = n * (n - 1) // 2
    m = min(m, pairs if not multi else M_MAX)
    es: set[tuple[int, int]] = set()
    out: list[tuple[int, int]] = []
    while len(out) < m:
        u, v = sorted(rng.sample(range(n), 2))
        if (u, v) in es and not (multi and rng.random() < 0.3):
            continue
        es.add((u, v))
        out.append((u, v))
    return out


def layered(rng: random.Random, n: int, w: int, m: int) -> list[tuple[int, int]]:
    layers = [list(range(i, min(n, i + w))) for i in range(0, n, w)]
    out = []
    while len(out) < m:
        li = rng.randrange(len(layers) - 1)
        lj = min(len(layers) - 1, li + 1 + (rng.random() < 0.2))
        out.append((rng.choice(layers[li]), rng.choice(layers[lj])))
    return out


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 1000:
        n = rng.randint(1, 12)
        m = rng.randint(0, min(M_MAX, n * (n - 1) // 2 + 3))
        emit(n, random_dag(rng, n, m, multi=rng.random() < 0.3) if n > 1 else [], rng)
        return
    if seed == 0:
        print("4 4\n1 2\n3 4\n3 2\n4 2")
    elif seed == 1:
        emit(1, [], rng)
    elif seed == 2:
        emit(N_MAX, [], rng)
    elif seed == 3:
        emit(N_MAX, [(i, i + 1) for i in range(N_MAX - 1)], rng)
    elif seed == 4:
        n = 60
        emit(n, [(u, 30 + v) for u in range(30) for v in range(30)][:M_MAX], rng)
    elif seed == 5:
        es = []
        for i in range(10):
            for j in range(10):
                if j + 1 < 10:
                    es.append((i * 10 + j, i * 10 + j + 1))
                if i + 1 < 10:
                    es.append((i * 10 + j, (i + 1) * 10 + j))
        emit(100, es, rng)
    elif seed == 6:
        emit(N_MAX, [(rng.randrange(v), v) for v in range(1, N_MAX)], rng)
    elif seed == 7:
        emit(N_MAX, [(v, rng.randrange(v)) for v in range(1, N_MAX)], rng)
    elif seed in (8, 9, 10):
        w = {8: 3, 9: 10, 10: 25}[seed]
        emit(N_MAX, layered(rng, N_MAX, w, {8: 150, 9: 400, 10: 900}[seed]), rng)
    elif seed == 11:
        # 2 本の鎖 (0..49 と 50..99) と、位置の小さい方から大きい方への鎖の間の辺。
        es = [(i, i + 1) for i in range(49)] + [(50 + i, 51 + i) for i in range(49)]
        for _ in range(60):
            a, b = sorted(rng.sample(range(50), 2))
            es.append((a, 50 + b) if rng.random() < 0.5 else (50 + a, b))
        emit(N_MAX, es, rng)
    elif seed == 12:
        emit(N_MAX, random_dag(rng, N_MAX, 1000), rng)
    elif seed == 13:
        emit(N_MAX, random_dag(rng, N_MAX, 300), rng)
    elif seed == 14:
        emit(N_MAX, random_dag(rng, N_MAX, 120), rng)
    elif seed == 15:
        emit(N_MAX, random_dag(rng, N_MAX, 1000, multi=True), rng)
    else:
        emit(70, random_dag(rng, 70, 1000), rng)


main()
