"""cf17-final-e (Combination Lock) の入力を作る。S、N と N 個の操作 L_i R_i を出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、|S| = N = 10^5 などの大きいケース。
答えが YES のケースは、ランダムな回文に、操作をそれぞれランダムな回数 (0 から 25) だけ施して S を作る
(同じ操作をあと 26 - 回数 だけ施せば回文に戻る)。NO のケースは、YES の S の 1 文字 S_i をずらす。
差分の列 (位置 0 から |S|) を左右で折り返し、操作 [L, R] を位置 L - 1 と R を結ぶ辺と見たとき、
S_i をずらすと位置 i と i + 1 の値が変わる。この 2 つが別の連結成分にあれば、どちらの成分でも
値の和が 26 の倍数でなくなるので NO になる (そういう i を選ぶ)。ほかに、ランダムな S と操作、
長さ 3 以下の操作、左右対称な操作だけ (回文かどうかは変わらない)、先頭からの操作だけ、
1 文字の操作だけ、全体の操作だけ、を混ぜる。|S| は偶数と奇数の両方を入れる。
seed が 1000 以上なら、回文との差を 2 と 13 を法とする連立 1 次方程式として掃き出す愚直解で解ける
小さい入力を出す (pj testdata crosscheck 用)。2000 以上なら、|S| と N が 400 までの入力を出す。
"""

import random
import sys

MAX_LEN = 10**5
MAX_N = 10**5
ALPHA = "abcdefghijklmnopqrstuvwxyz"

SAMPLES = [
    ("bixzja", [(2, 3), (3, 6)]),
    ("abc", [(2, 2)]),
    ("cassert", [(1, 2), (3, 4), (1, 1), (2, 2)]),
]
FIXED = [
    ("a", [(1, 1)]),
    ("ab", [(1, 1)]),
    ("ab", [(1, 2)]),  # 両方ずれるので差は変わらない
    ("az", [(2, 2)]),  # z の次は a
    ("abca", [(1, 4), (2, 3)]),  # 左右対称な操作だけ
    ("abcba", [(3, 3)]),
    ("zyxwvutsrqponmlkjihgfedcbaabcdefghijklmnopqrstuvwxyz", [(1, 52)]),
]
# (|S|, N, 作り方)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    (MAX_LEN, MAX_N, "random"),
    (MAX_LEN, MAX_N, "yes"),
    (MAX_LEN, MAX_N, "no"),
    (MAX_LEN - 1, MAX_N, "yes"),
    (MAX_LEN - 1, MAX_N, "no"),
    (MAX_LEN, MAX_N, "short_yes"),
    (MAX_LEN, MAX_N, "short_no"),
    (MAX_LEN - 1, MAX_N, "symmetric_no"),
    (MAX_LEN, MAX_N, "symmetric_yes"),
    (MAX_LEN, MAX_N, "prefix_no"),
    (MAX_LEN, MAX_N // 2, "singles"),
    (MAX_LEN, 1, "whole"),
    (MAX_LEN, 1, "yes"),
    (MAX_LEN, 1000, "yes"),
    (MAX_LEN, 1000, "no"),
    (1000, MAX_N, "yes"),
    (1000, MAX_N, "random"),
    (2, MAX_N, "random"),
]
KINDS = ["random", "yes", "no", "short_yes", "short_no", "symmetric_yes", "symmetric_no", "prefix_yes", "prefix_no", "singles", "whole"]


def operations(rng: random.Random, n: int, count: int, how: str) -> list[tuple[int, int]]:
    """1 始まりの区間 [L, R] を count 個返す。"""
    ops = []
    for _ in range(count):
        if how == "short":
            left = rng.randint(1, n)
            right = min(n, left + rng.randint(0, 2))
        elif how == "symmetric":
            left = rng.randint(1, (n + 1) // 2)
            right = n + 1 - left
        elif how == "prefix":
            left, right = 1, rng.randint(1, n)
        elif how == "singles":
            left = right = rng.randint(1, n)
        elif how == "whole":
            left, right = 1, n
        else:
            left, right = sorted((rng.randint(1, n), rng.randint(1, n)))
        ops.append((left, right))
    return ops


def fold(x: int, n: int) -> int:
    """差分の列の位置 x (0 から n) を左右で折り返した位置。"""
    return x if 2 * x < n + 1 else n - x


def reachable_from_palindrome(rng: random.Random, n: int, ops: list[tuple[int, int]]) -> list[int]:
    """ランダムな回文に、操作をそれぞれ 0 から 25 回施した文字列 (0 から 25 の列) を返す。"""
    half = [rng.randrange(26) for _ in range((n + 1) // 2)]
    s = [half[min(i, n - 1 - i)] for i in range(n)]
    diff = [0] * (n + 1)
    for left, right in ops:
        c = rng.randrange(26)
        diff[left - 1] += c
        diff[right] -= c
    acc = 0
    for i in range(n):
        acc += diff[i]
        s[i] = (s[i] + acc) % 26
    return s


def break_it(rng: random.Random, n: int, ops: list[tuple[int, int]], s: list[int]) -> list[int]:
    """位置 i と i + 1 が別の連結成分にある S_i を 1 つずらす。そういう i が無ければそのまま返す。"""
    parent = list(range(n + 1))

    def find(v: int) -> int:
        while parent[v] != v:
            parent[v] = parent[parent[v]]
            v = parent[v]
        return v

    for left, right in ops:
        parent[find(fold(left - 1, n))] = find(fold(right, n))
    candidates = [i for i in range(n) if find(fold(i, n)) != find(fold(i + 1, n))]
    if not candidates:
        return s
    i = rng.choice(candidates)
    s = s[:]
    s[i] = (s[i] + rng.randint(1, 25)) % 26
    return s


def build(rng: random.Random, n: int, count: int, how: str) -> tuple[list[int], list[tuple[int, int]]]:
    base = how.removesuffix("_yes").removesuffix("_no")
    ops = operations(rng, n, count, base if base in ("short", "symmetric", "prefix", "singles", "whole") else "random")
    if how in ("random", "singles", "whole"):
        return [rng.randrange(26) for _ in range(n)], ops
    s = reachable_from_palindrome(rng, n, ops)
    if how.endswith("no"):
        s = break_it(rng, n, ops, s)
    return s, ops


def case_for(seed: int, rng: random.Random) -> tuple[str, list[tuple[int, int]]]:
    if seed >= 1000:
        limit = 400 if seed >= 2000 else 20
        n, count = rng.randint(1, limit), rng.randint(1, limit)
        s, ops = build(rng, n, count, rng.choice(KINDS))
        if rng.random() < 0.2:
            s = [rng.choice([0, 25]) for _ in range(n)]  # a と z だけ (z の次の a で間違えやすい)
        return "".join(ALPHA[c] for c in s), ops
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    n, count, how = PLANS[seed - len(FIXED)]
    s, ops = build(rng, n, count, how)
    return "".join(ALPHA[c] for c in s), ops


COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)


def main() -> None:
    seed = int(sys.argv[1])
    s, ops = case_for(seed, random.Random(seed))
    n = len(s)
    assert 1 <= n <= MAX_LEN and set(s) <= set(ALPHA) and 1 <= len(ops) <= MAX_N
    assert all(1 <= left <= right <= n for left, right in ops)
    out = [s, str(len(ops))] + [f"{left} {right}" for left, right in ops]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
