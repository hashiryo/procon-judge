"""cf-1634-e (Fair Share) の入力を作る。m と、m 個の配列 (長さ n は偶数、2 <= n、n の和は 2 * 10^5 以下、要素は 1 以上 10^9 以下) を出す。

答えは、各配列の要素をちょうど半分ずつ L と R に分け、L 全体と R 全体を多重集合として等しくする分け方か、無ければ NO。
分け方は 1 つに決まらないので、checker.cpp が入力と提出の出力だけで判定する。どの値も全体で偶数回出てくることが、分け方がある
ことと同値になる (配列と値を頂点、要素を辺にした二部グラフの Euler 分割)。
seed が 0 から count - 1 までは本番のケース。問題文の例、小さい手作りの角のケース (YES と NO)、配列 1 つの最大、長さ 2 の配列を 10^5 個並べて
1 本の長い閉路にしたもの、値の種類が少ないもの、どの値もちょうど 2 回のもの、NO になる最大 (値 2 つだけ奇数回)、値が 10^9 まで
散らばるもの、全部同じ値。seed が 1000 以上なら、要素の数の和が 12 以下の入力を出す (全部の分け方を試す愚直解 brute.cpp 用)。
"""

import random
import sys

SUM_MAX = 200000
M_MAX = 100000
V_MAX = 10**9


def even_counts(rng: random.Random, total: int, pick) -> list[int]:
    """値を pick() で total // 2 個選び、それぞれ 2 回ずつ並べてから混ぜる。どの値も偶数回になる。"""
    vals = [pick() for _ in range(total // 2)]
    out = vals + vals
    rng.shuffle(out)
    return out


def split_lengths(rng: random.Random, total: int, m: int) -> list[int]:
    """total (偶数) を m 個の偶数 (2 以上) に分ける。"""
    lens = [2] * m
    for _ in range((total - 2 * m) // 2):
        lens[rng.randrange(m)] += 2
    return lens


def arrays_from(vals: list[int], lens: list[int]) -> list[list[int]]:
    out, k = [], 0
    for n in lens:
        out.append(vals[k:k + n])
        k += n
    return out


def break_parity(rng: random.Random, arrs: list[list[int]]) -> None:
    """要素を 1 つ別の値に替えて、2 つの値を奇数回にする (答えは NO)。"""
    i = rng.randrange(len(arrs))
    j = rng.randrange(len(arrs[i]))
    arrs[i][j] = V_MAX if arrs[i][j] != V_MAX else 1


def random_case(rng: random.Random, total: int, m: int, pick, no: bool = False) -> list[list[int]]:
    arrs = arrays_from(even_counts(rng, total, pick), split_lengths(rng, total, m))
    if no:
        break_parity(rng, arrs)
    return arrs


def chain(rng: random.Random, m: int) -> list[list[int]]:
    """配列 i を [v_i, v_{i+1}] にして、値と配列が 1 本の長い閉路になるようにする (v_m = v_0)。"""
    v = rng.sample(range(1, V_MAX + 1), m)
    arrs = [[v[i], v[(i + 1) % m]] for i in range(m)]
    rng.shuffle(arrs)
    return arrs


def small(rng: random.Random) -> list[list[int]]:
    m = rng.randint(1, 4)
    lens = [2 * rng.randint(1, 2) for _ in range(m)]
    while sum(lens) > 12:
        lens.pop()
    hi = rng.choice([2, 3, 4])
    return [[rng.randint(1, hi) for _ in range(n)] for n in lens]


PLANS = [
    lambda rng: [[1, 2], [1, 2, 3, 3], [1, 1, 2, 2, 3, 3]],
    lambda rng: [[1, 1]],
    lambda rng: [[1, 2]],
    lambda rng: [[1, 2], [2, 1]],
    lambda rng: random_case(rng, 20, 4, lambda: rng.randint(1, 5)),
    lambda rng: random_case(rng, SUM_MAX, 1, lambda: rng.randint(1, V_MAX)),
    lambda rng: chain(rng, M_MAX),
    lambda rng: random_case(rng, SUM_MAX, 30000, lambda: rng.randint(1, 10)),
    lambda rng: random_case(rng, SUM_MAX, 50000, lambda: rng.randint(1, V_MAX)),
    lambda rng: random_case(rng, SUM_MAX, 30000, lambda: rng.randint(1, 1000), no=True),
    lambda rng: random_case(rng, SUM_MAX, 5000, lambda: rng.randint(1, V_MAX)),
    lambda rng: [[7, 7] for _ in range(M_MAX)],
    lambda rng: random_case(rng, SUM_MAX, M_MAX, lambda: rng.randint(1, 200)),
    lambda rng: random_case(rng, SUM_MAX, M_MAX, lambda: rng.randint(1, V_MAX), no=True),
]


def case_for(seed: int) -> list[list[int]]:
    rng = random.Random(seed)
    if seed >= 1000:
        return small(rng)
    return PLANS[seed](rng)


def main() -> None:
    arrs = case_for(int(sys.argv[1]))
    m = len(arrs)
    assert 1 <= m <= M_MAX and sum(map(len, arrs)) <= SUM_MAX
    assert all(len(a) >= 2 and len(a) % 2 == 0 and all(1 <= x <= V_MAX for x in a) for a in arrs)
    out = [str(m)]
    for a in arrs:
        out.append(str(len(a)))
        out.append(" ".join(map(str, a)))
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
