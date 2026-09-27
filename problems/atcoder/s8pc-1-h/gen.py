"""s8pc-1-h (3人の昼食) の入力を作る。N D E と、N 行の A_i を出す。

seed が 0 から count - 1 までは本番のケース。例の 2 つ、N = 2 と 3 の角のケース、N = 20 の大きいもの。
E は 0、1、2 を混ぜ、E = 2 (半分ずつの列挙で点がいちばん多い) を多めにする。A は、全部 1 で D が
大きく、分け方が全部条件を満たすもの (答えは最大の 10^11 ほど)、全部同じ値で D = 0、2 の累乗で
部分和が全部違い答えが 0 になるもの、小さい値 (同じ点だらけ)、10^9 の近くに固まったもの、
ランダムを混ぜる。D は 0、最大、ランダム、A の大きさに合わせたものを混ぜる。
seed が 1000 以上なら、食品ごとに 4 通りを全部試す愚直解で解ける N <= 11 の入力を出す
(pj testdata crosscheck 用)。
"""

import random
import sys

MAX_N = 20
MAX_D = 10**9
MAX_E = 2
MAX_A = 10**9

SAMPLES = [
    (2, 1, [2, 4, 6]),  # 例 1
    (1, 1, [1, 2, 3, 4]),  # 例 2
]
FIXED = [
    (0, 0, [1, 1]),  # 3 人の和を等しくできないので 0
    (0, 2, [1, 2]),  # 両方残すしかない。答えは 1
    (MAX_D, 2, [MAX_A, MAX_A]),
    (0, 0, [5, 5, 5]),  # 1 つずつ配る 3! 通り
    (MAX_D, MAX_E, [1] * MAX_N),  # 全部の分け方が条件を満たす
    (0, MAX_E, [MAX_A] * MAX_N),  # 2 つ残して 6 つずつ配るものだけ
    (MAX_D, MAX_E, [MAX_A] * MAX_N),
    (0, MAX_E, [2**i for i in range(MAX_N)]),  # 部分和が全部違うので 0
    (MAX_D, 0, [2**i for i in range(MAX_N)]),
]
# (N, E, A の出し方, D の出し方)。本番のケースのうち FIXED のあとに並べる。
PLANS = [
    (MAX_N, 2, "random", "scaled"),
    (MAX_N, 2, "random", "scaled"),
    (MAX_N, 1, "random", "scaled"),
    (MAX_N, 0, "random", "scaled"),
    (MAX_N, 2, "random", "zero"),
    (MAX_N, 2, "random", "max"),
    (MAX_N, 2, "small", "small"),
    (MAX_N, 2, "near_max", "small"),
    (MAX_N, 2, "near_max", "scaled"),
    (MAX_N - 1, 2, "random", "scaled"),  # N が奇数
    (MAX_N - 1, 1, "small", "small"),
    (11, 2, "random", "scaled"),
    (None, None, "random", "scaled"),  # N と E はランダム
    (None, None, "small", "random"),
]
KINDS_A = ["random", "small", "near_max", "equal"]
KINDS_D = ["scaled", "zero", "max", "small", "random"]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)


def prices(rng: random.Random, n: int, how: str, top: int) -> list[int]:
    if how == "small":
        return [rng.randint(1, min(top, 5)) for _ in range(n)]
    if how == "near_max":
        return [top - rng.randint(0, min(top - 1, 1000)) for _ in range(n)]
    if how == "equal":
        return [rng.randint(1, top)] * n
    return [rng.randint(1, top) for _ in range(n)]


def limit(rng: random.Random, a: list[int], how: str) -> int:
    if how == "zero":
        return 0
    if how == "max":
        return MAX_D
    if how == "small":
        return rng.randint(0, 5)
    if how == "scaled":
        # 3 人の和の差は、およそ A の大きさ * sqrt(N) くらいに散らばる。
        return min(MAX_D, rng.randint(0, max(a) * 2))
    return rng.randint(0, MAX_D)


def case_for(seed: int, rng: random.Random) -> tuple[int, int, list[int]]:
    if seed >= 1000:
        # 愚直解が 4^N 通りを試しても間に合う大きさ。値の幅は小さいものも混ぜる。
        n, e = rng.randint(2, 11), rng.randint(0, MAX_E)
        a = prices(rng, n, rng.choice(KINDS_A), rng.choice([5, 100, MAX_A]))
        return limit(rng, a, rng.choice(KINDS_D)), e, a
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    n, e, how_a, how_d = PLANS[seed - len(FIXED)]
    n = n or rng.randint(2, MAX_N)
    e = rng.randint(0, MAX_E) if e is None else e
    a = prices(rng, n, how_a, MAX_A)
    return limit(rng, a, how_d), e, a


def main() -> None:
    seed = int(sys.argv[1])
    d, e, a = case_for(seed, random.Random(seed))
    assert 2 <= len(a) <= MAX_N and 0 <= d <= MAX_D and 0 <= e <= MAX_E
    assert all(1 <= x <= MAX_A for x in a)
    sys.stdout.write(f"{len(a)} {d} {e}\n" + "".join(f"{x}\n" for x in a))


if __name__ == "__main__":
    main()
