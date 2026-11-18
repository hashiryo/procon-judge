#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""洛谷 P1640 ([SCOI2010] 连续攻击游戏) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

属性値は 1 以上 V = 10^4 以下、装備は N ≤ 10^6 個。seed 0 から 11 は本番のケースで、問題文の例、一様な乱択 (10^6 個)、
値を 3000 以下に限った乱択、6000 を抜いた乱択、道 (i, i + 1) を小さい順に並べた形 (値 i の隣は (i - 1, i) が先に来るので、
空いた装備を先に探さない深さ優先は 1 から i まで下りる)、その道に自分だけの装備 (v, v) を足して 10^6 個にした形、道を
大きい順に並べた形、乱択の木 (根に近い木と深い木)、全部が (1, 2) の形、道の辺を重ねた形、小さい塊を道でつないだ形。
seed 1000 以上は、愚直解 (brute.hpp) と突き合わせる小さい入力 (装備 8 個まで、値 6 まで)。
"""
import random
import sys

V = 10000
SAMPLE = """3
1 2
3 2
4 5"""


def out(items: list[tuple[int, int]]) -> str:
    return "\n".join([str(len(items))] + [f"{a} {b}" for a, b in items])


def flip(rng: random.Random, items: list[tuple[int, int]]) -> list[tuple[int, int]]:
    return [(b, a) if rng.random() < 0.5 else (a, b) for a, b in items]


def main_case(seed: int) -> str:
    rng = random.Random(seed * 7919 + 13)
    N = 10**6
    if seed == 0:
        return SAMPLE
    if seed == 1:
        return out([(rng.randint(1, V), rng.randint(1, V)) for _ in range(N)])
    if seed == 2:
        return out([(rng.randint(1, 3000), rng.randint(1, 3000)) for _ in range(N)])
    if seed == 3:
        pick = lambda: rng.choice((rng.randint(1, 5999), rng.randint(6001, V)))
        return out([(pick(), pick()) for _ in range(N)])
    if seed == 4:
        return out([(i, i + 1) for i in range(1, V)])
    if seed == 5:
        items = [(i, i + 1) for i in range(1, V)]
        items += [(v, v) for v in (rng.randint(1, V) for _ in range(N - len(items)))]
        return out(items)
    if seed == 6:
        return out([(i, i + 1) for i in range(V - 1, 0, -1)])
    if seed == 7:
        # 根 1 に近い乱択の木。値 v の親は v より小さい乱択の値。
        items = [(v, rng.randint(1, v - 1)) for v in range(2, V + 1)]
        rng.shuffle(items)
        return out(flip(rng, items))
    if seed == 8:
        # 深い木。値 v の親は v - 1 から v - 4 のどれか。
        items = [(v, max(1, v - rng.randint(1, 4))) for v in range(2, V + 1)]
        rng.shuffle(items)
        return out(flip(rng, items))
    if seed == 9:
        return out([(1, 2)] * N)
    if seed == 10:
        # 道の辺 (i, i + 1) を乱択に重ねる。重なった辺で閉路ができる。
        items = [(i, i + 1) for i in (rng.randint(1, V - 1) for _ in range(N))]
        return out(flip(rng, items))
    if seed == 11:
        # 値 20 個ずつの塊 (中は木に 1 本だけ足す) を、隣の塊と 1 本でつなぐ。塊の中の辺は 50 回ずつ重ねる。
        items: list[tuple[int, int]] = []
        for lo in range(1, V + 1, 20):
            hi = min(V, lo + 19)
            block = [(v, rng.randint(lo, v - 1)) for v in range(lo + 1, hi + 1)]
            block.append((rng.randint(lo, hi), rng.randint(lo, hi)))
            items += block * 50
            if hi < V:
                items.append((rng.randint(lo, hi), hi + 1))
        rng.shuffle(items)
        return out(flip(rng, items))
    raise SystemExit(f"seed {seed} は 0 から 11 か、1000 以上にしてください")


def small_case(seed: int) -> str:
    rng = random.Random(seed * 104729 + 7)
    n = rng.randint(1, 8)
    k = rng.randint(1, 6)
    return out([(rng.randint(1, k), rng.randint(1, k)) for _ in range(n)])


def main() -> None:
    seed = int(sys.argv[1])
    print(small_case(seed) if seed >= 1000 else main_case(seed))


if __name__ == "__main__":
    main()
