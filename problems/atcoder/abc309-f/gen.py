"""abc309-f (Box in Box) の入力を作る。N と、N 個の箱の h w d を出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、小さいランダム、N = 2 × 10^5 のケース。
N = 2 × 10^5 のケースは、ランダムのほかは、どの箱もほかの箱に入らない形 (答えは No) にする。
3 辺の和が一定、最短の辺が全部同じ、残り 2 辺の和が一定、全部同じ、最短の辺だけ同じで残り 2 辺が増えていくもの、
辺が 1 と 2 だけのものである。最短の辺が同じ箱の組は、並べる順番を間違えると入ると判定されやすい。
いくつかには、ほかの箱が 1 つだけ入る箱を最後に足す。入力の順でも最短の辺の順でも最後に調べる箱なので、
提出は最後まで調べてやっと Yes を見つける。箱の向きは、形を決めたあとで 1 つずつランダムに回す。
seed が 1000 以上なら、全部の組と向きを比べる愚直解で解ける小さい入力を出す (pj testdata crosscheck 用)。
"""

import random
import sys

MAX_N = 2 * 10**5
MAX_V = 10**9

SAMPLES = [
    [(19, 8, 22), (10, 24, 12), (15, 25, 11)],
    [(19, 8, 22), (10, 25, 12), (15, 24, 11)],
    [(1, 1, 2), (1, 2, 2)],
]
# 角のケース。回さずにそのまま出す。
FIXED = [
    [(1, 1, 1), (1, 1, 1)],  # 同じ箱は入らない
    [(1, 1, 1), (2, 2, 2)],
    [(MAX_V, MAX_V, MAX_V), (MAX_V - 1, MAX_V - 1, MAX_V - 1)],
    [(MAX_V, MAX_V, MAX_V), (MAX_V - 1, MAX_V - 1, MAX_V)],  # 1 辺が同じなら入らない
    [(3, 2, 1), (2, 4, 3)],  # 回すと (1, 2, 3) が (2, 3, 4) に入る
    [(1, 2, 3), (3, 3, 2)],  # 回しても (1, 2, 3) と (2, 3, 3) で 1 辺が同じ
]
# 小さいランダム。(N, 値の上限)
SMALL = [(5, 3), (10, 5), (300, 40)]
# N = 2 × 10^5 のケース。(形, 最後に箱を 1 つ足すか)
PLANS = [
    ("random", False),
    ("plane", False),
    ("plane", True),
    ("same_min", False),
    ("antidiagonal", False),
    ("antidiagonal", True),
    ("equal", False),
    ("chain_ties", False),
    ("two_values", False),
    ("two_values", True),
]
# 形ごとの値の大きさ。比べ方しか効かないので、ランダムのほかは小さめにして入力を軽くする。
SCALE = {
    "random": MAX_V,
    "plane": 3 * 10**5,
    "same_min": 3 * 10**5,
    "antidiagonal": 2 * 10**5,
    "equal": 9,
    "chain_ties": 0,
    "two_values": 0,
}
COUNT = len(SAMPLES) + len(FIXED) + len(SMALL) + len(PLANS)


def shape(rng: random.Random, how: str, n: int, scale: int) -> list[list[int]]:
    """n 個の箱を、3 辺を小さい順に並べた形で作る。"""
    if how == "random":
        return [sorted(rng.randint(1, scale) for _ in range(3)) for _ in range(n)]
    if how == "plane":
        # 3 辺の和が一定。入るなら和が真に大きくなるので、どれも入らない。
        s = rng.randint(max(3, scale // 2), max(3, scale))
        boxes = []
        for _ in range(n):
            x, y = sorted(rng.sample(range(1, s), 2))
            boxes.append(sorted((x, y - x, s - y)))
        return boxes
    if how == "same_min":
        # 最短の辺が全部同じ。
        v = rng.randint(1, max(1, scale // 10))
        return [[v] + sorted(rng.randint(v, max(v, scale)) for _ in range(2)) for _ in range(n)]
    if how == "antidiagonal":
        # 残り 2 辺の和が一定。2 番目の辺が長ければ 3 番目の辺は短い。
        t = 2 * scale
        boxes = []
        for _ in range(n):
            b = rng.randint(1, scale)
            boxes.append([rng.randint(1, b), b, t - b])
        return boxes
    if how == "equal":
        v = rng.randint(1, scale)
        return [[v, v, v] for _ in range(n)]
    if how == "chain_ties":
        # 最短の辺が全部 1 で、残り 2 辺は箱ごとに真に増えていく。入力は小さい順に並べたまま出す。
        return [[1, 2 * i + 2, 2 * i + 2 + rng.randint(0, 1)] for i in range(n)]
    # two_values: 辺が 1 と 2 だけで、(2, 2, 2) は無い。最短の辺が全部 1 なので入らない。
    return [list(rng.choice([(1, 1, 1), (1, 1, 2), (1, 2, 2)])) for _ in range(n)]


def extra_box(boxes: list[list[int]]) -> list[int]:
    """最短の辺が一番長い箱のうち、残りの辺が一番短いものより 1 ずつ大きい箱。"""
    a, b, c = min(boxes, key=lambda box: (-box[0], box[1], box[2]))
    return [a + 1, b + 1, c + 1]


def finish(rng: random.Random, boxes: list[list[int]], extra: bool) -> list[list[int]]:
    """必要なら最後に箱を足し、1 つずつ向きをランダムに回す。"""
    if extra:
        boxes = boxes + [extra_box(boxes)]
    out = []
    for box in boxes:
        box = list(box)
        rng.shuffle(box)
        out.append(box)
    return out


def small_case(rng: random.Random) -> list[list[int]]:
    """愚直解が全部の組を比べられる大きさ。値の幅を狭くして、Yes と No の両方が出るようにする。"""
    n = rng.randint(2, 8) if rng.random() < 0.8 else rng.randint(9, 300)
    how = rng.choice(["random", "random", "plane", "same_min", "antidiagonal", "equal", "chain_ties", "two_values"])
    scale = rng.choice([2, 3, 5, 10, 100, MAX_V // 2])
    extra = how != "random" and rng.random() < 0.4
    return finish(rng, shape(rng, how, n - extra, scale), extra)


def boxes_for(seed: int, rng: random.Random) -> list[list[int]]:
    if seed >= 1000:
        return small_case(rng)
    if seed < len(SAMPLES):
        return [list(box) for box in SAMPLES[seed]]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return [list(box) for box in FIXED[seed]]
    seed -= len(FIXED)
    if seed < len(SMALL):
        n, scale = SMALL[seed]
        return finish(rng, shape(rng, "random", n, scale), False)
    how, extra = PLANS[seed - len(SMALL)]
    return finish(rng, shape(rng, how, MAX_N - extra, SCALE[how]), extra)


def main() -> None:
    seed = int(sys.argv[1])
    boxes = boxes_for(seed, random.Random(seed))
    assert 2 <= len(boxes) <= MAX_N
    assert all(len(box) == 3 and all(1 <= v <= MAX_V for v in box) for box in boxes)
    out = [str(len(boxes))] + [f"{h} {w} {d}" for h, w, d in boxes]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
