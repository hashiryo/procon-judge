"""abc230-h (Bullion) の入力を作る。W K と、K 個の相異なる金塊の重さ w を出す。

seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース、W = 2.5 * 10^5 のいろいろな重さの集合。
角のケースは、W = 2、重さ W だけ (袋が 1 kg あるので答えは全部 0)、重さ 1 だけ (答えは根付き木の数)、
重さ 1 から W まで全部、大きい重さだけ、2 の冪だけ、奇数だけ。W は 2 の冪とその前後も入れる
(MSET は添字が 2 の冪になるたびに表を倍に広げ、RelaxedConvolution も 2 の冪の塊ごとに掛けるため)。
seed が 1000 以上なら、重さの小さい順に (1 - x^k)^(-個数) を掛けていく O(W^2 log W) の愚直解で解ける
小さい入力を出す (pj testdata crosscheck 用)。2000 以上なら、愚直解でまだ解ける W <= 3000 の入力を出す。
"""

import random
import sys

MAX_W = 250000

SAMPLES = [
    (4, [1]),
    (10, list(range(1, 11))),
]
# (W, 重さの選び方)。本番のケースのうち例のあとに並べる。
PLANS = [
    (2, "one"),
    (2, "top"),
    (2, "all"),
    (3, "top"),
    (100, "random"),
    (1000, "all"),
    (MAX_W, "one"),
    (MAX_W, "all"),
    (MAX_W, "top"),
    (MAX_W, "random"),
    (MAX_W, "few"),
    (MAX_W, "large"),
    (MAX_W, "powers"),
    (MAX_W, "odd"),
    (131072, "random"),
    (131073, "one"),
    (65535, "all"),
    (4096, "random"),
]
COUNT = len(SAMPLES) + len(PLANS)
KINDS = ["one", "top", "all", "random", "few", "large", "powers", "odd", "two"]


def weights(rng: random.Random, w: int, kind: str) -> list[int]:
    if kind == "one":
        return [1]
    if kind == "two":
        return [2]
    if kind == "top":
        return [w]
    if kind == "all":
        out = list(range(1, w + 1))
        rng.shuffle(out)
        return out
    if kind == "random":
        return rng.sample(range(1, w + 1), rng.randint(1, w))
    if kind == "few":
        return rng.sample(range(1, min(w, 20) + 1), rng.randint(1, min(w, 5)))
    if kind == "large":
        return rng.sample(range(w // 2 + 1, w + 1), rng.randint(1, w - w // 2))
    if kind == "powers":
        out = [1 << i for i in range(20) if 1 << i <= w]
        rng.shuffle(out)
        return out
    assert kind == "odd"
    return list(range(1, w + 1, 2))


def case_for(seed: int, rng: random.Random) -> tuple[int, list[int]]:
    if seed >= 1000:
        w = rng.randint(101, 3000) if seed >= 2000 else rng.randint(2, 100)
        return w, weights(rng, w, rng.choice(KINDS))
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    w, kind = PLANS[seed - len(SAMPLES)]
    return w, weights(rng, w, kind)


def main() -> None:
    seed = int(sys.argv[1])
    w, ws = case_for(seed, random.Random(seed))
    assert 2 <= w <= MAX_W and 1 <= len(ws) <= w
    assert all(1 <= x <= w for x in ws) and len(set(ws)) == len(ws)
    sys.stdout.write(f"{w} {len(ws)}\n{' '.join(map(str, ws))}\n")


if __name__ == "__main__":
    main()
