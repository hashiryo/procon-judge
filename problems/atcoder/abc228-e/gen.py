"""abc228-e (Integer Sequence Fair) の入力を作る。N K M を 1 行で出す。答えは M^(K^N) mod P。

seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース、制約いっぱいのランダム。
seed が 1000 以上なら、愚直解 (x = x^K mod P を N 回) で解ける小さい N の入力を出す。
pj testdata crosscheck で参照実装と突き合わせるためのもの。
"""

import random
import sys

P = 998244353
PHI = P - 1  # 2^23 * 7 * 17
MAX = 10**18


def top(m: int) -> int:
    """MAX 以下で最大の m の倍数。"""
    return MAX // m * m


# 角のケース。指数を P - 1 で縮める所と、M が P の倍数の所で間違えやすい。
FIXED = [
    (2, 2, 2),  # 例 1
    (3, 14, 15926535),  # 例 2
    (1, 1, 1),
    (MAX, MAX, MAX),
    (1, 1, P),  # M が P の倍数なら 0
    (MAX, MAX, top(P)),
    (MAX, P - 1, 3),  # K が P - 1 の倍数なら指数が 0 に縮み、答えは 1
    (1, top(PHI), MAX),
    (MAX, P, 3),  # K ≡ 1 (mod P - 1) なら答えは M
    (MAX, MAX, P + 1),  # M ≡ 1
    (MAX, MAX, P - 1),  # M ≡ -1。K が偶数なので 1
    (MAX, top(2) - 1, P - 1),  # K が奇数なので -1
    (23, 2 * 7 * 17, 5),  # K^N が初めて P - 1 の倍数になる N
    (22, 2 * 7 * 17, 5),  # その 1 つ手前
    (MAX, 2**23, 3),
    (1, MAX, 1),  # M = 1
]
RANDOM = 10


def random_case(rng: random.Random) -> tuple[int, int, int]:
    kind = rng.randrange(4)
    n, k, m = (rng.randint(1, MAX) for _ in range(3))
    if kind == 1:
        m = rng.randint(1, MAX // P) * P
    elif kind == 2:
        k = rng.randint(1, MAX // PHI) * PHI
    elif kind == 3:
        # P - 1 と公約数を持つ K。
        k = rng.choice([2, 7, 17, 14, 34, 119, 238]) ** rng.randint(1, 8)
    return n, k, m


def small_case(rng: random.Random) -> tuple[int, int, int]:
    """愚直解が N 回の累乗で解ける大きさ。K と M は角の値を多めに混ぜる。"""
    def pick() -> int:
        return rng.choice([
            rng.randint(1, 20),
            rng.randint(1, MAX),
            rng.randint(1, MAX // P) * P,
            rng.randint(1, MAX // PHI) * PHI,
            rng.choice([P - 1, P, P + 1, PHI - 1, PHI + 1, 2 * 7 * 17, 2**23, 7, 17]),
        ])

    return rng.randint(1, 60), pick(), pick()


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 1000:
        n, k, m = small_case(rng)
    elif seed < len(FIXED):
        n, k, m = FIXED[seed]
    else:
        n, k, m = random_case(rng)
    assert all(1 <= v <= MAX for v in (n, k, m))
    print(n, k, m)


if __name__ == "__main__":
    main()
