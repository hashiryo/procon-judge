"""luogu-P11278 (绝世丑角) の入力を作る。n q、a_1 ... a_n (2^32 未満)、q 個の操作 t l r を出す。最後の操作は質問にする。

t = 1 は区間の各要素を nim 平方する操作、t = 2 は区間の xor、t = 3 は区間の普通の和を聞く。
seed 0 は問題文の説明 (6⊕1⊕4⊕2 = 1、3+6+1+4 = 14 など) に合わせて組み立てた例。角のケースは n = 1、全部 0、全部 2^32 - 1、
全体の平方を 32 回 (元に戻る) と 33 回、1 点の平方だけ。残りは n = 2.5 × 10^5、q = 10^5 で、元の問題の部分点の形
(t = 1 が無い、t = 1 が 1 点だけ、値が 64 未満、t = 3 が無い、t = 1 が全体だけ、t = 1 が質問より前に全部ある) とランダム、
長い区間の平方が多いもの。seed が 1000 以上なら、配列をそのまま書き換える愚直解で解ける n, q <= 2000 の入力を出す。
"""

import random
import sys

MAX_N = 250000
MAX_Q = 100000
TOP = 2**32 - 1

Op = tuple[int, int, int]

SAMPLE = ([3, 6, 1, 4, 2, 6], [(2, 2, 5), (3, 1, 4), (1, 5, 6), (2, 1, 6), (1, 1, 2), (3, 5, 6)])


def interval(rng: random.Random, n: int, how: str) -> tuple[int, int]:
    if how == "point":
        i = rng.randint(1, n)
        return i, i
    if how == "all":
        return 1, n
    if how == "long":
        l = rng.randint(1, max(1, n // 10))
        return l, rng.randint(max(l, n - n // 10), n)
    l = rng.randint(1, n)
    return l, rng.randint(l, n)


def ops(rng: random.Random, n: int, q: int, kinds: list[int], sq: str = "random", qr: str = "random") -> list[Op]:
    out = []
    for _ in range(q):
        t = rng.choice(kinds)
        out.append((t, *interval(rng, n, sq if t == 1 else qr)))
    if out[-1][0] == 1:
        out[-1] = (rng.choice([k for k in kinds if k != 1] or [2]), *interval(rng, n, qr))
    return out


def values(rng: random.Random, n: int, top: int = TOP) -> list[int]:
    return [rng.randint(0, top) for _ in range(n)]


def plan(seed: int, rng: random.Random) -> tuple[list[int], list[Op]]:
    n, q = MAX_N, MAX_Q
    if seed == 1:
        return [rng.randint(0, TOP)], [(1, 1, 1), (2, 1, 1), (1, 1, 1), (3, 1, 1)]
    if seed == 2:
        return [0] * n, ops(rng, n, q, [1, 2, 3])
    if seed == 3:
        return [TOP] * n, ops(rng, n, q, [1, 2, 3])
    if seed == 4:  # 全体の平方を 32 回 (元に戻る) と 33 回、その間に質問
        a = values(rng, n)
        out = []
        for i in range(33):
            out.append((1, 1, n))
            out += [(rng.choice([2, 3]), *interval(rng, n, "random")) for _ in range(3)]
        return a, out + [(3, 1, n), (2, 1, n)]
    if seed == 5:  # t = 1 が無い
        return values(rng, n), ops(rng, n, q, [2, 3])
    if seed == 6:  # t = 1 は 1 点だけ
        return values(rng, n), ops(rng, n, q, [1, 2, 3], sq="point")
    if seed == 7:  # 値が 64 未満
        return values(rng, n, 63), ops(rng, n, q, [1, 2, 3])
    if seed == 8:  # t = 3 が無い
        return values(rng, n), ops(rng, n, q, [1, 2])
    if seed == 9:  # t = 1 は全体だけ
        return values(rng, n), ops(rng, n, q, [1, 2, 3], sq="all")
    if seed == 10:  # t = 1 が質問より前に全部ある
        squares = [(1, *interval(rng, n, "random")) for _ in range(q // 2)]
        return values(rng, n), squares + ops(rng, n, q - q // 2, [2, 3])
    if seed == 11:
        return values(rng, n), ops(rng, n, q, [1, 2, 3])
    if seed == 12:  # 長い区間の平方と長い区間の質問
        return values(rng, n), ops(rng, n, q, [1, 1, 2, 3], sq="long", qr="long")
    assert False


COUNT = 13


def case_for(seed: int) -> tuple[list[int], list[Op]]:
    rng = random.Random(seed)
    if seed >= 1000:
        n = rng.randint(1, rng.choice([5, 50, 2000]))
        q = rng.randint(1, rng.choice([5, 50, 2000]))
        top = rng.choice([1, 3, 63, 255, TOP])
        return values(rng, n, top), ops(rng, n, q, rng.choice([[1, 2, 3], [1, 2], [1, 3], [2, 3]]), rng.choice(["random", "point", "all", "long"]))
    if seed == 0:
        return SAMPLE
    return plan(seed, rng)


def main() -> None:
    seed = int(sys.argv[1])
    a, qs = case_for(seed)
    n = len(a)
    assert 1 <= n <= MAX_N and 1 <= len(qs) <= MAX_Q and all(0 <= x <= TOP for x in a)
    assert all(t in (1, 2, 3) and 1 <= l <= r <= n for t, l, r in qs) and qs[-1][0] != 1
    out = [f"{n} {len(qs)}", " ".join(map(str, a))] + [f"{t} {l} {r}" for t, l, r in qs]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
