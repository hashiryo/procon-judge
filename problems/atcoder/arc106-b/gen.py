"""arc106-b (Values) の入力を作る。N M、a、b、単純グラフの辺 c_i d_i を出す。

答えは、連結成分ごとに a の和と b の和が等しいかどうか。seed が 0 から count - 1 までは本番のケース。
例の 4 つ、角のケース、N が 10^5 か 2 × 10^5 (または M が 2 × 10^5) のいろいろなグラフ。
b は成分の中で a を並べ替えて作り (答えは Yes)、半分ほどは 1 を別の成分へ動かすか、1 つだけ変えて No にする。
1 を別の成分へ動かすと、全体の和は等しいまま成分の和がずれる。成分の中の a - b の和がちょうど 2^32 に
なるものも入れる。32 ビットで足すと 0 に見えて Yes と答えてしまう。M = 0 は提出が別の分岐で答える。
seed が 1000 以上なら、推移閉包で連結を調べる愚直解で解ける小さい入力を出す (pj testdata crosscheck 用)。
2000 以上なら、愚直解でまだ解ける N = 300 までの入力を出す。
"""

import random
import sys

MAX_N = 2 * 10**5
MAX_M = 2 * 10**5
MAX_V = 10**9

Case = tuple[int, list[int], list[int], list[tuple[int, int]]]

SAMPLES: list[Case] = [
    (3, [1, 2, 3], [2, 2, 2], [(1, 2), (2, 3)]),
    (1, [5], [5], []),
    (2, [1, 1], [2, 1], [(1, 2)]),
    (
        17,
        [
            -905371741, -999219903, 969314057, -989982132, -87720225, -175700172, -993990465, 929461728, 895449935,
            -999016241, 782467448, -906404298, 578539175, 9684413, -619191091, -952046546, 125053320,
        ],
        [
            -440503430, -997661446, -912471383, -995879434, 932992245, -928388880, -616761933, 929461728, 210953513,
            -994677396, 648190629, -530944122, 578539175, 9684413, 595786809, -952046546, 125053320,
        ],
        [(2, 10), (6, 12), (9, 11), (11, 5), (7, 6), (3, 15), (3, 1), (1, 9), (10, 4)],
    ),
]
FIXED: list[Case] = [
    (1, [5], [6], []),
    (1, [-MAX_V], [-MAX_V], []),
    (2, [3, 1], [2, 2], [(2, 1)]),
    (2, [1, 2], [2, 1], []),  # 全体の和は等しいが辺が無い
    (3, [MAX_V, -MAX_V, 7], [-MAX_V, MAX_V, 7], [(1, 2)]),
    # a - b = (2e9, 2e9, 294967296)。和は 2^32 で No。
    (3, [MAX_V, MAX_V, 294967296], [-MAX_V, -MAX_V, 0], [(1, 2), (2, 3)]),
    # a - b = (2e9, 2e9, -2e9, -2e9)。途中の和は 32 ビットを超えるが、全体は 0 で Yes。
    (4, [MAX_V, MAX_V, -MAX_V, -MAX_V], [-MAX_V, -MAX_V, MAX_V, MAX_V], [(1, 2), (2, 3), (3, 4)]),
]
# (N, グラフの形, 答えの作り方, 値の幅)。本番のケースのうち角のケースのあとに並べる。
# yes は成分の中で a を並べ替えたもの、move は 1 を別の成分へ動かしたもの、bump は 1 か所だけ 1 ずらしたもの、
# pow32 は 1 つの成分の a - b の和を 2^32 にしたもの。値の幅は、wide が ±10^9 を多く混ぜたもの、narrow が
# ±1000 まで。入力が大きくなりすぎないよう、wide は一部のケースだけにする。
PLANS = [
    (MAX_N, "isolated", "yes", "narrow"),  # M = 0
    (10**5, "isolated", "move", "narrow"),
    (MAX_N, "path", "yes", "wide"),  # 辺の流量が 10^14 を超える
    (MAX_N, "tree", "yes", "narrow"),
    (10**5, "star", "bump", "narrow"),
    (10**5, "connected", "yes", "narrow"),  # M = 2 × 10^5
    (10**5, "forest", "move", "wide"),
    (10**5, "forest", "pow32", "narrow"),
    (632, "complete", "yes", "wide"),  # M = 199396
    (200, "complete", "bump", "wide"),
    (10**5, "mixed", "yes", "narrow"),  # 大きな成分 1 つと孤立点
    (300, "forest", "move", "wide"),
    (300, "forest", "yes", "wide"),
]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)


def graph(rng: random.Random, n: int, shape: str) -> tuple[list[tuple[int, int]], list[list[int]]]:
    """頂点 0 から n - 1 の単純グラフの辺と、連結成分 (頂点の列) を返す。"""
    groups: list[list[int]]
    if shape == "isolated":
        return [], [[v] for v in range(n)]
    if shape in ("tree", "path", "star", "connected", "complete"):
        groups = [list(range(n))]
    elif shape == "forest":
        # 大きさ 1 から 50 の成分に分ける。
        groups, v = [], 0
        while v < n:
            size = min(n - v, rng.randint(1, 50))
            groups.append(list(range(v, v + size)))
            v += size
    else:
        assert shape == "mixed"
        big = n // 2
        groups = [list(range(big))] + [[v] for v in range(big, n)]
    edges: list[tuple[int, int]] = []
    for g in groups:
        if shape == "path":
            edges += [(g[i - 1], g[i]) for i in range(1, len(g))]
        elif shape == "star":
            edges += [(g[0], g[i]) for i in range(1, len(g))]
        elif shape == "complete":
            edges += [(g[i], g[j]) for i in range(len(g)) for j in range(i + 1, len(g))]
        else:
            edges += [(g[rng.randrange(i)], g[i]) for i in range(1, len(g))]
    if shape == "connected":
        seen = {tuple(sorted(e)) for e in edges}
        while len(edges) < MAX_M:
            u, v = rng.sample(range(n), 2)
            if (min(u, v), max(u, v)) not in seen:
                seen.add((min(u, v), max(u, v)))
                edges.append((u, v))
    return edges, groups


def values(rng: random.Random, n: int, groups: list[list[int]], how: str, width: str = "wide") -> tuple[list[int], list[int]]:
    def one() -> int:
        if width == "narrow":
            return rng.randint(-1000, 1000)
        return rng.choice([rng.randint(-MAX_V, MAX_V), MAX_V, -MAX_V, rng.randint(-5, 5)])

    a = [one() for _ in range(n)]
    b = a[:]
    for g in groups:
        perm = [a[v] for v in g]
        rng.shuffle(perm)
        for v, x in zip(g, perm):
            b[v] = x
    if how == "bump":
        v = rng.randrange(n)
        b[v] += 1 if b[v] < MAX_V else -1
    elif how == "move":
        # 1 増やせる頂点と、別の成分の 1 減らせる頂点を選ぶ。選べなければ 1 か所だけずらす。
        ups = [v for v in range(n) if b[v] < MAX_V]
        up = rng.choice(ups) if ups else 0
        downs = [v for g in groups if up not in g for v in g if b[v] > -MAX_V]
        if ups and downs:
            b[up] += 1
            b[rng.choice(downs)] -= 1
        else:
            b[up] += 1 if b[up] < MAX_V else -1
    elif how == "pow32":
        g = rng.choice([g for g in groups if len(g) >= 3])
        x, y, z = rng.sample(g, 3)
        a[x], b[x], a[y], b[y], a[z], b[z] = MAX_V, -MAX_V, MAX_V, -MAX_V, 294967296, 0
        rest = [v for v in g if v not in (x, y, z)]
        perm = [a[v] for v in rest]
        rng.shuffle(perm)
        for v, w in zip(rest, perm):
            b[v] = w
    return a, b


def build(rng: random.Random, n: int, shape: str, how: str, width: str) -> Case:
    edges, groups = graph(rng, n, shape)
    a, b = values(rng, n, groups, how, width)
    perm = list(range(1, n + 1))
    rng.shuffle(perm)
    # perm[v] が頂点 v の番号。a, b も番号の順に並べ直す。
    a2, b2 = [0] * n, [0] * n
    for v in range(n):
        a2[perm[v] - 1], b2[perm[v] - 1] = a[v], b[v]
    out = [(perm[u], perm[v]) if rng.random() < 0.5 else (perm[v], perm[u]) for u, v in edges]
    rng.shuffle(out)
    return n, a2, b2, out


def small_case(rng: random.Random, n: int) -> Case:
    shape = rng.choice(["isolated", "tree", "path", "star", "forest", "mixed", "complete", "random"])
    how = rng.choice(["yes", "yes", "bump", "move"])
    if shape == "random":
        # 辺を 1 本ずつ確率 p で入れる。成分は後から求める。
        p = rng.choice([0.05, 0.1, 0.3, 0.6])
        edges = [(u, v) for u in range(n) for v in range(u + 1, n) if rng.random() < p]
        parent = list(range(n))

        def find(v: int) -> int:
            while parent[v] != v:
                v = parent[v]
            return v

        for u, v in edges:
            parent[find(u)] = find(v)
        comp: dict[int, list[int]] = {}
        for v in range(n):
            comp.setdefault(find(v), []).append(v)
        groups = list(comp.values())
    else:
        edges, groups = graph(rng, n, shape)
    if how == "move" and len(groups) == 1:
        how = "bump"
    a, b = values(rng, n, groups, how)
    out = [(u + 1, v + 1) if rng.random() < 0.5 else (v + 1, u + 1) for u, v in edges]
    rng.shuffle(out)
    return n, a, b, out


def case_for(seed: int, rng: random.Random) -> Case:
    if seed >= 1000:
        return small_case(rng, rng.randint(50, 300) if seed >= 2000 else rng.randint(1, 12))
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    return build(rng, *PLANS[seed - len(FIXED)])


def main() -> None:
    seed = int(sys.argv[1])
    n, a, b, edges = case_for(seed, random.Random(seed))
    assert 1 <= n <= MAX_N and 0 <= len(edges) <= MAX_M and len(a) == len(b) == n
    assert all(-MAX_V <= v <= MAX_V for v in a + b)
    assert all(1 <= c <= n and 1 <= d <= n and c != d for c, d in edges)
    assert len({(min(c, d), max(c, d)) for c, d in edges}) == len(edges)
    out = [f"{n} {len(edges)}", " ".join(map(str, a)), " ".join(map(str, b))] + [f"{c} {d}" for c, d in edges]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
