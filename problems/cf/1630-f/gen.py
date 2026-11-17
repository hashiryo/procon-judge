"""cf-1630-f (Making It Bipartite) の入力を作る。t (1 以上 10^4 以下) と、テストケースごとに n と相異なる値 a_i (1 以上 5 * 10^4
以下) を出す。n の和は 5 * 10^4 以下。

seed が 0 から count - 1 までは本番のケース。問題文の例、n = 1、1 から 50000 の全部 (割り切れる組が最も多い)、25001 以上だけ
(辺が無く答えは 0)、3-smooth と 7-smooth の数 (長い鎖)、1 から 20000、ランダムな部分集合 (大きさ 25000 と 10000)、奇数だけ、
1 と 50000 を含む小さいテストケースを 10^4 個 (倍数を数え上げると重い形)、n = 500 ほどを 100 個、約数の多い数の約数だけ。
seed が 1000 以上なら、頂点が 12 個以下で値が 60 以下のテストケースを 1 から 5 個出す (愚直解 brute.cpp 用)。
"""

import random
import sys

V = 50000
SUM = 50000


def emit(tests: list[list[int]]) -> None:
    assert 1 <= len(tests) <= 10**4 and sum(len(t) for t in tests) <= SUM
    out = [str(len(tests))]
    for t in tests:
        assert t and len(set(t)) == len(t) and all(1 <= x <= V for x in t)
        out.append(str(len(t)))
        out.append(" ".join(map(str, t)))
    print("\n".join(out))


def smooth(primes: list[int]) -> list[int]:
    vals = {1}
    frontier = [1]
    while frontier:
        nxt = []
        for x in frontier:
            for p in primes:
                y = x * p
                if y <= V and y not in vals:
                    vals.add(y)
                    nxt.append(y)
        frontier = nxt
    return sorted(vals)


def divisors(x: int) -> list[int]:
    return [d for d in range(1, x + 1) if x % d == 0]


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 1000:
        tests = []
        for _ in range(rng.randint(1, 5)):
            n = rng.randint(1, 12)
            tests.append(rng.sample(range(1, 61), n))
        emit(tests)
        return
    if seed == 0:
        print("4\n4\n8 4 2 1\n4\n30 2 3 5\n5\n12 4 6 2 3\n10\n85 195 5 39 3 13 266 154 14 2")
        return
    if seed == 1:
        tests = [[1], [V], [7]]
    elif seed == 2:
        t = list(range(1, V + 1))
        rng.shuffle(t)
        tests = [t]
    elif seed == 3:
        t = list(range(25001, V + 1))
        rng.shuffle(t)
        tests = [t]
    elif seed == 4:
        t = smooth([2, 3])
        rng.shuffle(t)
        tests = [t]
    elif seed == 5:
        t = smooth([2, 3, 5, 7])
        rng.shuffle(t)
        tests = [t]
    elif seed == 6:
        t = list(range(1, 20001))
        rng.shuffle(t)
        tests = [t]
    elif seed in (7, 8):
        tests = [rng.sample(range(1, V + 1), 25000 if seed == 7 else 10000)]
    elif seed == 9:
        t = list(range(1, V + 1, 2))
        rng.shuffle(t)
        tests = [t]
    elif seed == 10:
        tests = []
        for _ in range(10**4):
            t = [1, V] + rng.sample(range(2, V), 3)
            rng.shuffle(t)
            tests.append(t)
    elif seed == 11:
        tests = [rng.sample(range(1, V + 1), 500) for _ in range(100)]
    elif seed == 12:
        # 約数の多い数 (45360 = 2^4 * 3^4 * 5 * 7、約数 100 個) と 27720 の約数を混ぜ、残りをランダムで埋める。
        t = sorted(set(divisors(45360)) | set(divisors(27720)))
        rest = [x for x in range(1, V + 1) if x not in set(t)]
        t += rng.sample(rest, 2000)
        rng.shuffle(t)
        tests = [t]
    else:
        # 小さい値ほど多く選ぶ (割り切れる組が多い) 中くらいのテストケースを 10 個。
        tests = []
        for _ in range(10):
            pool = set()
            while len(pool) < 4000:
                pool.add(min(V, int(rng.paretovariate(0.6))))
            t = sorted(pool)
            rng.shuffle(t)
            tests.append(t)
    emit(tests)


main()
