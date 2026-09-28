"""abc324-g (Generate Arrays) の入力を作る。N、順列 A、Q と Q 個の操作 t s x を出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、N と Q が 2 × 10^5 と 5 × 10^4 のもの。
s と x をランダムに選ぶだけだと、ほとんどの操作が空の数列を作る。そこで gen.py は数列の中身を正確に追い、
半分の確率でいちばん長い数列を選んで、x をその数列の長さの範囲や、中にある値の近くから選ぶ。
s を i - 1 にして、取り除いた側をさらに分け続けるものも入れる。
A = (1, 2, ..., N) では数列が区間になるので、いちばん長い区間を半分に分け続けるケースも入れる。
ほかに、ランダムな s と x、x = N で何も取り除かない操作を数列 0 にくり返すもの、x = 0 で全部を次の数列へ
移し続けるものを入れる。入力と出力が大きいので、N = Q = 2 × 10^5 は 4 ケースにする。
seed が 1000 以上なら、数列を配列のまま持って操作どおりに分ける愚直解で解ける小さい入力を出す
(pj testdata crosscheck 用)。2000 以上なら、愚直解でまだ解ける N, Q = 2000 までの入力を出す。
"""

import heapq
import random
import sys

MAX_N = 2 * 10**5
MAX_Q = 2 * 10**5

SAMPLES = [
    ([1, 8, 7, 4, 5, 6, 3, 2, 9, 10], [(2, 0, 4), (1, 1, 2), (2, 0, 2), (2, 2, 5), (1, 0, 1)]),
    ([6, 7, 8, 4, 5, 1, 3, 2], [(2, 0, 0), (1, 1, 0), (2, 2, 0), (1, 3, 8), (2, 2, 3)]),
    (
        [20, 6, 13, 11, 29, 30, 9, 10, 16, 5, 8, 25, 1, 19, 12, 18, 7, 2, 4, 27, 3, 22, 23, 24, 28, 21, 14, 26, 15, 17],
        [(1, 0, 22), (1, 0, 21), (2, 0, 15), (1, 0, 9), (1, 3, 6), (2, 3, 18), (1, 6, 2), (1, 0, 1), (2, 5, 20), (2, 7, 26)],
    ),
]
FIXED = [
    ([1], [(1, 0, 0)]),  # 全部を移すので 1
    ([1], [(2, 0, 0)]),  # 1
    ([1], [(1, 0, 1), (2, 0, 1), (2, 1, 0), (1, 2, 0)]),
    ([2, 1], [(2, 0, 1), (1, 0, 0), (2, 1, 2), (1, 2, 1), (2, 0, 0)]),
    ([3, 1, 2], [(1, 0, 3), (2, 0, 3), (1, 0, 0), (2, 3, 0), (1, 4, 2), (2, 4, 1)]),
]
# (N, Q, A の形, 操作の選び方)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    (MAX_N, MAX_Q, "random", "big"),
    (MAX_N, MAX_Q, "random", "chain"),
    (MAX_N, MAX_Q, "sorted", "halving"),
    (MAX_N, MAX_Q, "random", "type2"),
    (50000, 50000, "random", "uniform"),
    (50000, 50000, "reversed", "big"),
    (50000, 50000, "random", "noop"),
    (50000, 50000, "random", "zero"),
    (50000, 50000, "zigzag", "big"),
    (50000, 50000, "random", "type1"),
    (1, 50000, "sorted", "uniform"),
    (MAX_N, 1, "random", "big"),
    (1000, 50000, "random", "big"),
    (50000, 1000, "sorted", "big"),
]


def permutation(rng: random.Random, n: int, how: str) -> list[int]:
    if how == "sorted":
        return list(range(1, n + 1))
    if how == "reversed":
        return list(range(n, 0, -1))
    if how == "zigzag":
        # 1, N, 2, N - 1, ... と大小を交互に並べる。
        out = []
        lo, hi = 1, n
        while lo <= hi:
            out.append(lo)
            if lo != hi:
                out.append(hi)
            lo, hi = lo + 1, hi - 1
        return out
    a = list(range(1, n + 1))
    rng.shuffle(a)
    return a


def halving(rng: random.Random, n: int, q: int) -> list[tuple[int, int, int]]:
    """A が昇順のとき用。数列は区間 [lo, hi) になり、長さが正確にわかる。いちばん長い区間を半分に分け続ける。"""
    heap = [(-n, 0, 0)]  # (-長さ, 数列の番号, lo)
    ops = []
    for i in range(1, q + 1):
        length, s, lo = heapq.heappop(heap)
        length = -length
        t = rng.randint(1, 2)
        half = length // 2
        # t = 1 なら前から half 個を残す。t = 2 なら lo + half 以下の値を残す。どちらも同じ分け方になる。
        x = half if t == 1 else lo + half
        heapq.heappush(heap, (-half, s, lo))
        heapq.heappush(heap, (-(length - half), i, lo + half))
        ops.append((t, s, x))
    return ops


def longest(seqs: list[list[int]], heap: list[tuple[int, int]]) -> int:
    """いちばん長い数列の番号。heap の古いもの (今の長さと合わないもの) は捨てる。"""
    while -heap[0][0] != len(seqs[heap[0][1]]):
        heapq.heappop(heap)
    return heap[0][1]


def operations(rng: random.Random, a: list[int], q: int, how: str) -> list[tuple[int, int, int]]:
    """操作を q 個作る。数列を位置のリストのまま正確に追い、長い数列と、中身の値の間の x を選ぶ。"""
    n = len(a)
    if how == "halving":
        return halving(rng, n, q)
    seqs = [list(range(n))]  # 数列 i に残っている要素の位置 (昇順)
    heap = [(-n, 0)]  # (-長さ, 数列の番号)。長さは減るだけなので、古いものは長さが合わなくなる
    ops = []
    for i in range(1, q + 1):
        if how == "uniform":
            t, s, x = rng.randint(1, 2), rng.randrange(i), rng.randint(0, n)
        elif how == "noop":
            t, s, x = rng.randint(1, 2), 0, n
            if rng.random() < 0.01:
                x = rng.randint(0, n)
        elif how == "zero":
            # 全部の要素を持っている数列 (いちばん長い数列) から、全部を次の数列へ移す。
            t, s, x = rng.randint(1, 2), longest(seqs, heap) if rng.random() < 0.9 else rng.randrange(i), 0
        else:
            if how == "chain" and seqs[i - 1]:
                s = i - 1
            elif rng.random() < 0.5:
                s = longest(seqs, heap)
            else:
                s = rng.randrange(i)
            t = 1 if how == "type1" else 2 if how == "type2" else rng.randint(1, 2)
            cur, r = seqs[s], rng.random()
            if r < 0.04:
                x = rng.choice([0, n])
            elif t == 1:
                x = rng.randint(0, len(cur))
            else:
                x = a[rng.choice(cur)] - rng.randint(0, 1) if cur else rng.randint(0, n)
        cur = seqs[s]
        if x == 0:  # 全部を移す
            removed, seqs[s] = cur, []
        elif x >= (len(cur) if t == 1 else n):  # 何も移さない
            removed = []
        elif t == 1:
            removed = cur[x:]
            del cur[x:]
        else:
            removed = [p for p in cur if a[p] > x]
            seqs[s] = [p for p in cur if a[p] <= x]
        seqs.append(removed)
        heapq.heappush(heap, (-len(seqs[s]), s))
        heapq.heappush(heap, (-len(removed), i))
        ops.append((t, s, x))
    return ops


def case_for(seed: int, rng: random.Random) -> tuple[list[int], list[tuple[int, int, int]]]:
    if seed >= 1000:
        limit = 2000 if seed >= 2000 else 30
        n, q = rng.randint(1, limit), rng.randint(1, limit)
        shape = rng.choice(["random", "random", "sorted", "reversed", "zigzag"])
        how = rng.choice(["uniform", "big", "big", "chain", "zero", "type1", "type2", "noop"])
        if shape == "sorted" and rng.random() < 0.3:
            how = "halving"
        a = permutation(rng, n, shape)
        return a, operations(rng, a, q, how)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    n, q, shape, how = PLANS[seed - len(FIXED)]
    a = permutation(rng, n, shape)
    return a, operations(rng, a, q, how)


def main() -> None:
    seed = int(sys.argv[1])
    a, ops = case_for(seed, random.Random(seed))
    n, q = len(a), len(ops)
    assert 1 <= n <= MAX_N and 1 <= q <= MAX_Q and sorted(a) == list(range(1, n + 1))
    assert all(t in (1, 2) and 0 <= s < i and 0 <= x <= n for i, (t, s, x) in enumerate(ops, 1))
    out = [str(n), " ".join(map(str, a)), str(q)] + [f"{t} {s} {x}" for t, s, x in ops]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
