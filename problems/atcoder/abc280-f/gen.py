"""abc280-f (Pay or Receive) の入力を作る。N M Q、道 A_i B_i C_i、質問 X_i Y_i を出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、N = Q = 10^5 のグラフ。
C は町ごとに決めた高さの差にして、つじつまの合う (答えが inf にならない) 連結成分を作り、
一部の成分にだけ差の合わない道を足して inf にする。グラフは、ランダムな木、C = 10^9 のパス
(答えが ±10^14 近くになる)、M = 10^5 のランダム (大きな成分と小さな成分)、そこに合わない道を 1 本足した
もの、M = 0 (X = Y の 0 以外は nan)、自分への道と同じ町どうしの道が多い 10 町ずつの組 (1 割の組だけ inf)、
少ない町に道が集まったもの。
seed が 1000 以上なら、質問ごとに Bellman-Ford で最長路と正の閉路を調べる愚直解で解ける小さい入力を出す
(pj testdata crosscheck 用)。2000 以上なら、愚直解でまだ解ける N = 200 までの入力を出す。
"""

import random
import sys

MAX_N = 10**5
MAX_M = 10**5
MAX_Q = 10**5
MAX_C = 10**9
LOOP_TOWNS = 20000

SAMPLES = [
    (5, [(1, 2, 1), (1, 2, 2), (3, 4, 1), (4, 5, 1), (3, 5, 2)], [(5, 3), (1, 2), (3, 1)]),
    (2, [(1, 1, 1)], [(1, 1)]),
    (
        9,
        [(3, 1, 4), (1, 5, 9), (2, 6, 5), (3, 5, 8), (9, 7, 9), (3, 2, 3), (8, 4, 6)],
        [(2, 6), (4, 3), (3, 8), (3, 2), (7, 9)],
    ),
]
FIXED = [
    (2, [], [(1, 1), (1, 2), (2, 2)]),
    (2, [(1, 1, 0)], [(1, 1), (1, 2), (2, 1)]),  # C = 0 の自分への道は何も変えない
    (2, [(2, 2, MAX_C)], [(2, 2), (1, 1), (1, 2)]),
    (3, [(1, 2, 5), (1, 2, 5)], [(1, 2), (2, 1), (3, 3), (1, 3)]),  # 同じ向きで同じ C なら合う
    (3, [(1, 2, 5), (2, 1, 5)], [(1, 2), (2, 1), (1, 1), (3, 3)]),
    (3, [(1, 2, 3), (2, 3, 4), (1, 3, 7)], [(1, 3), (3, 1), (2, 1)]),
    (3, [(1, 2, 3), (2, 3, 4), (1, 3, 8)], [(1, 3), (3, 1), (2, 2)]),
    (4, [(1, 2, 0), (3, 4, 0), (2, 1, 0)], [(1, 2), (2, 1), (1, 3), (4, 3)]),
]
# (N, M, グラフの形)。Q = 10^5。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    (MAX_N, MAX_N - 1, "tree"),
    (MAX_N, MAX_N - 1, "path"),
    (MAX_N, MAX_M, "random"),
    (MAX_N, MAX_M, "random_bad"),
    (MAX_N, 0, "random"),
    (MAX_N, MAX_M, "loops"),
    (MAX_N, MAX_M, "dense"),
]


def consistent_edges(rng: random.Random, pairs: list[tuple[int, int]], height: list[int]) -> list[tuple[int, int, int]]:
    """町の組を、高さの低い方から高い方へ向けた道にする。C は高さの差なので、つじつまが合う。"""
    out = []
    for u, v in pairs:
        if height[u] > height[v] or (height[u] == height[v] and rng.random() < 0.5):
            u, v = v, u
        out.append((u, v, height[v] - height[u]))
    return out


def bad_edge(u: int, v: int, height: list[int]) -> tuple[int, int, int]:
    """u から v への、高さの差と合わない道。"""
    c = height[v] - height[u]
    if c < 0:
        return v, u, -c + 1 if -c < MAX_C else -c - 1
    return u, v, c + 1 if c < MAX_C else c - 1


def graph(rng: random.Random, n: int, m: int, how: str) -> list[tuple[int, int, int]]:
    """1 始まりの町で道を返す。"""
    height = [rng.randint(0, MAX_C) for _ in range(n + 1)]
    if how == "tree":
        pairs = [(rng.randint(1, i - 1), i) for i in range(2, n + 1)]
        return consistent_edges(rng, pairs, height)
    if how == "path":
        # 町 1 から町 N へ C = 10^9 の道が続く。町の番号は混ぜない。
        return [(i, i + 1, MAX_C) for i in range(1, n)]
    if how in ("random", "random_bad"):
        pairs = [(rng.randint(1, n), rng.randint(1, n)) for _ in range(m)]
        edges = consistent_edges(rng, pairs, height)
        if how == "random_bad":
            # 最初の道の両端がいる成分 (たいてい大きな成分) にだけ、合わない道を 1 本足す。
            u, v, _ = edges[0]
            edges[-1] = bad_edge(u, v, height)
        rng.shuffle(edges)
        return edges
    if how == "loops":
        # 町 1 から LOOP_TOWNS を 10 町ずつの組に分け、組の中にだけ道を張る。自分への道 (C = 0) と、
        # 前に張った道と同じもの (同じ町どうしの道) を多く混ぜる。1 割の組では、そのうち 1 本を
        # C > 0 の自分への道か、C を変えた同じ町どうしの道にして inf にする。
        edges = []
        per = m // (LOOP_TOWNS // 10)
        for base in range(1, LOOP_TOWNS + 1, 10):
            local = []
            for _ in range(per):
                u, v = rng.randint(base, base + 9), rng.randint(base, base + 9)
                r = rng.random()
                if r < 0.3:
                    local.append((u, u, 0))
                elif r < 0.6 and local:
                    local.append(rng.choice(local))
                else:
                    local += consistent_edges(rng, [(u, v)], height)
            if rng.random() < 0.1:
                a, b, c = rng.choice(local)
                i = rng.randrange(len(local))
                local[i] = (a, a, rng.randint(1, MAX_C)) if rng.random() < 0.5 else (a, b, c + 1 if c < MAX_C else c - 1)
            edges += local
        rng.shuffle(edges)
        return edges
    assert how == "dense"
    # 300 町の間に道を集める。質問もこの町どうしを多くする。
    pairs = [(rng.randint(1, 300), rng.randint(1, 300)) for _ in range(m)]
    return consistent_edges(rng, pairs, height)


def queries(rng: random.Random, n: int, q: int, how: str) -> list[tuple[int, int]]:
    out = []
    if how == "path":
        out = [(1, n), (n, 1), (1, 1)]
    if how == "loops":
        # 8 割は同じ組の 2 町を聞く。
        while len(out) < q:
            x = rng.randint(1, LOOP_TOWNS)
            base = (x - 1) // 10 * 10 + 1
            out.append((x, rng.randint(base, base + 9) if rng.random() < 0.8 else rng.randint(1, n)))
        return out
    hi = 300 if how == "dense" else n
    while len(out) < q:
        x = rng.randint(1, hi if rng.random() < 0.9 else n)
        out.append((x, x if rng.random() < 0.05 else rng.randint(1, hi if rng.random() < 0.9 else n)))
    return out


def small_case(rng: random.Random, medium: bool) -> tuple[int, list[tuple[int, int, int]], list[tuple[int, int]]]:
    """愚直解で解ける大きさ。高さの差で作った道に、合わない道と自分への道を混ぜる。町が少ないので、
    同じ町どうしの道もよくできる。
    """
    n = rng.randint(2, 200) if medium else rng.randint(2, 10)
    m = rng.randint(0, 300) if medium else rng.randint(0, 14)
    q = rng.randint(1, 200) if medium else rng.randint(1, 15)
    small_c = rng.random() < 0.5
    height = [rng.randint(0, 5 if small_c else MAX_C) for _ in range(n + 1)]
    edges = []
    for _ in range(m):
        u, v = rng.randint(1, n), rng.randint(1, n)
        r = rng.random()
        if r < 0.05:
            edges.append(bad_edge(u, v, height))
        elif r < 0.12:
            edges.append((u, u, 0 if rng.random() < 0.7 else rng.randint(1, MAX_C)))
        else:
            edges += consistent_edges(rng, [(u, v)], height)
    qs = [(rng.randint(1, n), rng.randint(1, n)) for _ in range(q)]
    return n, edges, qs


def case_for(seed: int, rng: random.Random) -> tuple[int, list[tuple[int, int, int]], list[tuple[int, int]]]:
    if seed >= 1000:
        return small_case(rng, seed >= 2000)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    n, m, how = PLANS[seed - len(FIXED)]
    return n, graph(rng, n, m, how), queries(rng, n, MAX_Q, how)


def main() -> None:
    seed = int(sys.argv[1])
    n, edges, qs = case_for(seed, random.Random(seed))
    assert 2 <= n <= MAX_N and 0 <= len(edges) <= MAX_M and 1 <= len(qs) <= MAX_Q
    assert all(1 <= a <= n and 1 <= b <= n and 0 <= c <= MAX_C for a, b, c in edges)
    assert all(1 <= x <= n and 1 <= y <= n for x, y in qs)
    out = [f"{n} {len(edges)} {len(qs)}"] + [f"{a} {b} {c}" for a, b, c in edges] + [f"{x} {y}" for x, y in qs]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
