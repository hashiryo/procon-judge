"""cf-147-b (Smile House) の入力を作る。n m と、続く m 行の i j c_ij c_ji を出す。

seed が 0 から count - 1 までは本番のケース。例の 1 つ、n = 2 の角のケース、n = 300 前後のケース。
答えを決めたケースは、各部屋に電位 phi を置いて扉の重みを phi(j) - phi(i) - 1 とし (どの閉じた歩みも負になる)、L 個の部屋を
回る閉路の向きの辺だけに 1 を足し、そのうち 1 本には 2 を足す。閉路の和は +1 になる。L 歩より短い閉じた歩みは、2 を足した辺を
通るたびに、戻るまでに普通の辺 (電位を除くと -1) を 1 本以上通るので、正にならない。答えは L になる。
ほかに、乱数の重み、全部が負の重み、電位だけの重み (どの閉じた歩みも 0 で答えは 0)、±10^4 だけの重み、往復が 0 以下に
なる重み (c_ji = -c_ij - r) を入れる。
seed が 1000 以上なら、ちょうど k 歩の歩みを k = 1, 2, ... と伸ばす愚直解で解ける小さい入力 (n <= 9) を出す
(pj testdata crosscheck --brute brute.hpp 用)。
"""

import random
import sys

W = 10**4

SAMPLES = [
    "4 4\n1 2 -10 3\n1 3 1 -10\n2 4 -10 -1\n3 4 0 -3\n",
]


def all_pairs(n: int) -> list[tuple[int, int]]:
    return [(i, j) for i in range(n) for j in range(i + 1, n)]


def some_pairs(rng: random.Random, n: int, m: int, must: set[tuple[int, int]]) -> list[tuple[int, int]]:
    """must を含む m 本 (足りなければ全部) の扉の組を選ぶ。"""
    m = min(m, n * (n - 1) // 2)
    got = set(must)
    if m * 3 >= n * (n - 1) // 2:
        rest = [p for p in all_pairs(n) if p not in got]
        rng.shuffle(rest)
        got.update(rest[: max(0, m - len(got))])
    else:
        while len(got) < m:
            i, j = rng.sample(range(n), 2)
            got.add((min(i, j), max(i, j)))
    return sorted(got)


def emit(rng: random.Random, n: int, doors: dict[tuple[int, int], tuple[int, int]]) -> str:
    """doors は (i, j) (i < j) から (c_ij, c_ji)。並びと向きを乱数で崩して出す。"""
    items = list(doors.items())
    rng.shuffle(items)
    lines = [f"{n} {len(items)}"]
    for (i, j), (c, d) in items:
        assert -W <= c <= W and -W <= d <= W
        if rng.random() < 0.5:
            i, j, c, d = j, i, d, c
        lines.append(f"{i + 1} {j + 1} {c} {d}")
    return "\n".join(lines) + "\n"


def planted(rng: random.Random, n: int, lens: list[int], m: int | None) -> str:
    """部屋の重ならない閉路を lens の長さで植える。m が None なら扉は全部の組。"""
    order = list(range(n))
    rng.shuffle(order)
    phi = [rng.randint(0, W // 2 - 1) for _ in range(n)]
    cycles = []
    at = 0
    for L in lens:
        cycles.append(order[at : at + L])
        at += L
    must = set()
    for cyc in cycles:
        for t in range(len(cyc)):
            a, b = cyc[t], cyc[(t + 1) % len(cyc)]
            must.add((min(a, b), max(a, b)))
    pairs = all_pairs(n) if m is None else some_pairs(rng, n, m, must)
    w = {}  # 向きのある重み
    for i, j in pairs:
        w[(i, j)] = phi[j] - phi[i] - 1
        w[(j, i)] = phi[i] - phi[j] - 1
    for cyc in cycles:
        special = rng.randrange(len(cyc))
        for t in range(len(cyc)):
            a, b = cyc[t], cyc[(t + 1) % len(cyc)]
            w[(a, b)] += 2 if t == special else 1
    return emit(rng, n, {(i, j): (w[(i, j)], w[(j, i)]) for i, j in pairs})


def weights(rng: random.Random, n: int, m: int | None, how: str) -> str:
    pairs = all_pairs(n) if m is None else some_pairs(rng, n, m, set())
    phi = [rng.randint(0, W // 2 - 1) for _ in range(n)]
    rank = list(range(n))
    rng.shuffle(rank)
    doors = {}
    for i, j in pairs:
        if how == "random":
            c, d = rng.randint(-W, W), rng.randint(-W, W)
        elif how == "negative":
            c, d = rng.randint(-W, -1), rng.randint(-W, -1)
        elif how == "potential":  # どの閉じた歩みも 0
            c, d = phi[j] - phi[i], phi[i] - phi[j]
        elif how == "updown":  # 順位の上へ +10^4、下へ -10^4。往復は 0、3 部屋の閉路で正になる
            c = W if rank[i] < rank[j] else -W
            c, d = c, -c
        elif how == "antisym":  # 往復は 0 以下
            c = rng.randint(-W + 3, W - 3)
            c, d = c, -c - rng.randint(0, 3)
        else:
            raise ValueError(how)
        doors[(i, j)] = (c, d)
    return emit(rng, n, doors)


def pair(c: int, d: int) -> str:
    return f"2 1\n1 2 {c} {d}\n"


PLANS = [
    lambda rng: pair(3, -2),
    lambda rng: pair(-7, 7),
    lambda rng: weights(rng, 300, None, "random"),
    lambda rng: weights(rng, 300, None, "negative"),
    lambda rng: weights(rng, 300, None, "potential"),
    lambda rng: weights(rng, 300, None, "updown"),
    lambda rng: weights(rng, 300, None, "antisym"),
    lambda rng: weights(rng, 300, 900, "antisym"),
    lambda rng: planted(rng, 300, [300], None),
    lambda rng: planted(rng, 300, [256], None),
    lambda rng: planted(rng, 300, [257], None),
    lambda rng: planted(rng, 300, [129], 900),
    lambda rng: planted(rng, 300, [3], None),
    lambda rng: planted(rng, 300, [2], None),
    lambda rng: planted(rng, 299, [150], 299 * 298 // 4),
    lambda rng: planted(rng, 300, [100, 40], None),
    lambda rng: planted(rng, 300, [300], 1000),
]
COUNT = len(SAMPLES) + len(PLANS)


def main() -> None:
    seed = int(sys.argv[1])
    if seed >= 1000:
        rng = random.Random(seed)
        n = rng.randint(2, 9)
        kind = rng.randrange(4)
        if kind == 0:  # 小さい値の乱数 (正、0、負の閉路が混ざる)
            pairs = some_pairs(rng, n, rng.randint(1, n * (n - 1) // 2), set())
            doors = {p: (rng.randint(-5, 4), rng.randint(-5, 4)) for p in pairs}
            sys.stdout.write(emit(rng, n, doors))
        elif kind == 1:
            lens = [rng.randint(2, n)]
            lo = 1 if lens[0] == 2 else lens[0]  # 植える閉路の扉の数
            sys.stdout.write(planted(rng, n, lens, rng.choice([None, rng.randint(lo, n * (n - 1) // 2)])))
        else:
            how = ["antisym", "updown", "potential", "negative"][rng.randrange(4)]
            sys.stdout.write(weights(rng, n, rng.choice([None, rng.randint(1, n * (n - 1) // 2)]), how))
        return
    if seed >= COUNT:
        raise SystemExit(f"seed {seed} is out of range (count = {COUNT})")
    if seed < len(SAMPLES):
        sys.stdout.write(SAMPLES[seed])
        return
    rng = random.Random(seed)
    sys.stdout.write(PLANS[seed - len(SAMPLES)](rng))


if __name__ == "__main__":
    main()
