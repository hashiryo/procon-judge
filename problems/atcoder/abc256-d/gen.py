"""abc256-d (Union of Interval) の入力を作る。N と、N 行の L_i R_i を出す。

seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース、N = 2 × 10^5 と 5 × 10^4 の区間の集まり。
集まりの形は、ランダム (ほぼ 1 つに繋がる)、長さ 1 から 3 のランダム (離れた区間が多く残る。座標の幅を
N にしたものと 2 × 10^5 にしたもの)、[i, i + 1) を全部混ぜて並べたもの (端で接するだけで 1 つに繋がる)、
間を空けた [2i - 1, 2i) (出力が 10^5 行)、全部同じ区間、入れ子で内側から外側へ並べたもの、短い区間を
たくさん入れたあとに全体を覆う区間を入れるもの、右端の大きい順に並べて接するもの、L の小さい順に並べたランダム。
提出の RangeSet は、区間を入れるたびに重なる区間を消すので、全体を覆う区間を最後に入れる形で一度に多く消す。
seed が 1000 以上なら、長さ 1 のマスを 1 つずつ塗る愚直解で解ける小さい入力を出す (pj testdata crosscheck 用)。
2000 以上なら、同じ愚直解でまだ解ける N = 2000 までの入力を出す。
"""

import random
import sys

MAX_N = 2 * 10**5
MAX_R = 2 * 10**5

SAMPLES = [
    [(10, 20), (20, 30), (40, 50)],
    [(10, 40), (30, 60), (20, 50)],
]
FIXED = [
    [(1, 2)],
    [(1, MAX_R)],
    [(MAX_R - 1, MAX_R)],
    [(5, 9), (5, 9)],
    [(1, 2), (2, 3)],
    [(2, 3), (1, 2)],
    [(1, 2), (3, 4)],
    [(1, 10), (2, 3)],
    [(2, 3), (1, 10)],
    [(1, 5), (5, 10), (10, 15), (16, 20)],
]
# (N, 形)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    (MAX_N, "random"),
    (MAX_N, "short"),
    (MAX_N, "units"),
    (MAX_N, "gaps"),
    (MAX_N, "cover_last"),
    (MAX_N, "nested"),
    (MAX_N, "touch_desc"),
    (50000, "same"),
    (50000, "sorted_random"),
    (50000, "short_sparse"),
]


def intervals(rng: random.Random, n: int, shape: str) -> list[tuple[int, int]]:
    if shape == "random":
        out = []
        for _ in range(n):
            l, r = sorted(rng.sample(range(1, MAX_R + 1), 2))
            out.append((l, r))
        return out
    if shape in ("short", "short_sparse"):
        # short_sparse は、区間の数に比べて座標を広く取り、繋がらない所を多くする。
        top = MAX_R if shape == "short_sparse" else min(MAX_R, n)
        out = []
        for _ in range(n):
            l = rng.randint(1, top - 1)
            out.append((l, min(top, l + rng.randint(1, 3))))
        return out
    if shape == "units":
        out = [(i, i + 1) for i in range(1, MAX_R)]
        rng.shuffle(out)
        return out[:n]
    if shape == "gaps":
        out = [(2 * i - 1, 2 * i) for i in range(1, MAX_R // 2 + 1)]
        out += [rng.choice(out) for _ in range(n - len(out))]
        rng.shuffle(out)
        return out
    if shape == "cover_last":
        # 間を空けた長さ 1 の区間 10^5 個 (と、その重複) を入れ、最後に全体を覆う区間を入れる。
        out = [(2 * i - 1, 2 * i) for i in range(1, MAX_R // 2 + 1)]
        out += [rng.choice(out) for _ in range(n - 1 - len(out))]
        rng.shuffle(out)
        return out + [(1, MAX_R)]
    if shape == "nested":
        # [c - k, c + k) を k の小さい順に入れる。入れるたびに 1 つ前の区間を飲み込む。
        # 残りは、それまでの区間に含まれるものと、右端 2 × 10^5 まで伸ばすもの。
        c = MAX_R // 2
        out = [(c - k, c + k) for k in range(1, c)]
        for _ in range(n - len(out)):
            l = rng.randint(1, MAX_R - 1)
            out.append((l, rng.randint(l + 1, MAX_R if rng.random() < 0.01 else max(l + 1, MAX_R - 2))))
        return out
    if shape == "touch_desc":
        # 接する長さ 1 の区間を右から順に入れる。
        return [(i, i + 1) for i in range(MAX_R - 1, 0, -1)][:n]
    if shape == "same":
        l, r = sorted(rng.sample(range(1, MAX_R + 1), 2))
        return [(l, r)] * n
    assert shape == "sorted_random"
    out = []
    for _ in range(n):
        l = rng.randint(1, MAX_R - 1)
        out.append((l, min(MAX_R, l + int(rng.expovariate(1 / 5)) + 1)))
    return sorted(out)


def small_case(rng: random.Random, n: int, top: int) -> list[tuple[int, int]]:
    """愚直解向け。座標を 1 から top までに縮め、ときどき 2 × 10^5 の端を混ぜる。"""
    out = []
    for _ in range(n):
        how = rng.randrange(5)
        if how == 0:
            l, r = sorted(rng.sample(range(1, top + 1), 2))
        elif how == 1:
            l = rng.randint(1, top - 1)
            r = min(top, l + rng.randint(1, 3))
        elif how == 2:
            l = rng.randint(MAX_R - top, MAX_R - 1)
            r = rng.randint(l + 1, MAX_R)
        elif how == 3 and out:
            l, r = rng.choice(out)
            if rng.random() < 0.5:
                l, r = r, r + rng.randint(1, 3)
                if r > MAX_R:
                    l, r = MAX_R - 1, MAX_R
        else:
            l = rng.randint(1, MAX_R - 1)
            r = rng.randint(l + 1, min(MAX_R, l + top))
        out.append((l, r))
    return out


def case_for(seed: int, rng: random.Random) -> list[tuple[int, int]]:
    if seed >= 2000:
        return small_case(rng, rng.randint(11, 2000), rng.choice([100, 1000, 10000]))
    if seed >= 1000:
        return small_case(rng, rng.randint(1, 10), rng.choice([5, 10, 30]))
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    n, shape = PLANS[seed - len(FIXED)]
    return intervals(rng, n, shape)


def main() -> None:
    seed = int(sys.argv[1])
    segs = case_for(seed, random.Random(seed))
    assert 1 <= len(segs) <= MAX_N and all(1 <= l < r <= MAX_R for l, r in segs)
    out = [str(len(segs))] + [f"{l} {r}" for l, r in segs]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
