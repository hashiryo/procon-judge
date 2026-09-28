"""abc236-g (Good Vertices) の入力を作る。N T L と、時刻の順に T 本の有向辺 u_t v_t を出す。
答えは頂点ごとに、頂点 1 からちょうど L 回の移動で行けるようになる最初の時刻 (なければ -1)。

seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース、N = 100 のケース。
角のケースは、N = 2、辺が 1 本、自己ループだけ、L = 1 (答えは 1 から i への辺の時刻)、長さ 3 の閉路と L = 2。
N = 100 のケースは、全部の辺 (T = 10^4) をランダムな順に足すもの (L = 1, 2, 10^9, 10^9 - 1)、
頂点 1 から 100 を回る長さ 100 の閉路を最後に閉じ、そのあとランダムな辺を少し足すもの (閉じた時点では
L mod 100 で着く頂点が 1 つに決まる)、長さの互いに素な 2 つの閉路 (L が大きいと両方を回って着ける頂点が
増える)、2 部グラフ (L の偶奇で半分が -1)、番号の小さい方から大きい方への辺だけの DAG に最後に自己ループを
1 本足すもの (L = 10^9 だと、ほとんどが -1)、ランダムな辺を少しだけ足すもの (頂点 1 から出る長さ 10 の
歩道は入れる)、辺を番号の順に足すもの。
seed が 1000 以上なら、時刻ごとに真偽値の隣接行列を L 乗する愚直解で解ける小さい入力を出す
(pj testdata crosscheck 用)。2000 以上なら、愚直解でまだ解ける N = 40 までの入力を出す。
"""

import random
import sys

MAX_N = 100
MAX_L = 10**9

Case = tuple[int, int, list[tuple[int, int]]]

SAMPLES: list[Case] = [
    (4, 3, [(2, 3), (3, 4), (1, 2), (3, 2), (2, 2)]),
    (2, MAX_L, [(1, 2)]),
]
FIXED: list[Case] = [
    (2, 1, [(1, 2)]),
    (2, 1, [(1, 1)]),
    (2, MAX_L, [(1, 1), (1, 2)]),
    (2, MAX_L, [(2, 2), (2, 1), (1, 2), (1, 1)]),
    (2, MAX_L - 1, [(2, 1), (1, 2)]),  # 長さ 2 の閉路で L が奇数なので、頂点 2 だけに着く
    (3, 2, [(1, 2), (2, 3), (3, 1)]),
    (3, MAX_L, [(3, 3), (2, 3), (1, 2)]),
]
# (N, L, 辺の作り方)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    (MAX_N, 1, "complete"),
    (MAX_N, 2, "complete"),
    (MAX_N, MAX_L, "complete"),
    (MAX_N, MAX_L - 1, "complete"),
    (MAX_N, MAX_L, "cycle"),
    (MAX_N, MAX_L - 37, "cycle"),
    (MAX_N, MAX_L, "two_cycles"),
    (MAX_N, 150, "two_cycles"),
    (MAX_N, MAX_L, "bipartite"),
    (MAX_N, MAX_L - 1, "bipartite"),
    (MAX_N, MAX_L, "dag"),
    (MAX_N, 99, "dag"),  # 頂点 100 には 99 回でちょうど着ける
    (MAX_N, MAX_L, "sparse"),
    (MAX_N, 5, "sparse"),
    (MAX_N, MAX_L, "sorted"),
    (MAX_N, 0, "random"),
]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)
HOWS = ["complete", "cycle", "two_cycles", "bipartite", "dag", "sparse", "sorted", "random"]


def edges_for(rng: random.Random, n: int, how: str) -> list[tuple[int, int]]:
    """頂点 1 から n の辺を時刻の順に返す。"""
    everything = [(u, v) for u in range(1, n + 1) for v in range(1, n + 1)]
    if how == "complete":
        rng.shuffle(everything)
        return everything
    if how == "sorted":
        return everything
    if how == "random":
        return rng.sample(everything, rng.randint(1, n * n))
    if how == "sparse":
        # 出次数の平均が 2 から 4 のランダムな辺と、頂点 1 から出る長さ 10 の歩道の辺。
        out = rng.sample(everything, rng.randint(2 * n, 4 * n) if n > 4 else rng.randint(1, n * n))
        walk = [1] + [rng.randint(1, n) for _ in range(10)]
        for e in zip(walk, walk[1:]):
            if e not in out:
                out.insert(rng.randint(0, len(out)), e)
        return out
    if how == "cycle":
        # 頂点をランダムな順に回る閉路 (1 から始まる)。閉じる辺は最後の方に置く。
        order = [1] + rng.sample(range(2, n + 1), n - 1)
        cycle = [(order[i], order[(i + 1) % n]) for i in range(n)]
        closing = cycle.pop()
        rng.shuffle(cycle)
        used = set(cycle) | {closing}
        extra = rng.sample([e for e in everything if e not in used], rng.randint(0, n))
        return cycle + [closing] + extra
    if how == "two_cycles":
        # 頂点 1 を通る長さ a と b の閉路 (a と b は互いに素)。残りの頂点には辺が無い。
        a, b = rng.choice([(7, 11), (13, 17), (41, 58), (3, 5), (47, 52)])
        a = min(a, n)
        b = min(b, n - a + 1)
        others = rng.sample(range(2, n + 1), a + b - 2)
        c1 = [1] + others[: a - 1]
        c2 = [1] + others[a - 1 :]
        out = [(c1[i], c1[(i + 1) % a]) for i in range(a)] + [(c2[i], c2[(i + 1) % b]) for i in range(b)]
        rng.shuffle(out)
        return out
    if how == "bipartite":
        # 頂点を 2 つに分け、間の辺だけを置く。頂点 1 と 2 は別の側にする。
        side = {v: v == 1 or (v > 2 and rng.random() < 0.5) for v in range(1, n + 1)}
        out = [(u, v) for u, v in everything if side[u] != side[v]]
        rng.shuffle(out)
        return out
    assert how == "dag"
    out = [(u, v) for u, v in everything if u < v]
    rng.shuffle(out)
    return out + [(n, n)]


def case_for(seed: int, rng: random.Random) -> Case:
    if seed >= 1000:
        n = rng.randint(20, 40) if seed >= 2000 else rng.randint(2, 8)
        l = rng.choice([1, 2, 3, rng.randint(1, 20), rng.randint(1, MAX_L), MAX_L, MAX_L - 1])
        return n, l, edges_for(rng, n, rng.choice(HOWS))
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    n, l, how = PLANS[seed - len(FIXED)]
    return n, l or rng.randint(1, MAX_L), edges_for(rng, n, how)


def main() -> None:
    seed = int(sys.argv[1])
    n, l, edges = case_for(seed, random.Random(seed))
    assert 2 <= n <= MAX_N and 1 <= len(edges) <= n * n and 1 <= l <= MAX_L
    assert all(1 <= u <= n and 1 <= v <= n for u, v in edges) and len(set(edges)) == len(edges)
    out = [f"{n} {len(edges)} {l}"] + [f"{u} {v}" for u, v in edges]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
