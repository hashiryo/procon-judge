"""abc236-h (Distinct Multiples) の入力を作る。N M と D_1 ... D_N を出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、いろいろな D。
角のケースは、M < N、D が全部 1 (答えは M(M - 1)...(M - N + 1))、D が全部 M、D が倍数の鎖 (1, 2, 4, ...)、
M が 998244353 の倍数 (数え上げた数は 0 でないのに、余りは 0)、10^9 の近くの素数を並べたもの。
10^9 の近くの素数では、2 つの最小公倍数が 10^18 の前後に来るので、M を超えたかの判定で間違えやすい。
D が全部 M の約数なら、どの組の最小公倍数も M 以下になる。
sps::exp は N が 12 以上で rnk_zeta を使う計算に切り替わるので、N = 11, 12, 13, 16 を入れる。
seed が 1000 以上なら、A_i を D_i の倍数から 1 つずつ選んで全部試す愚直解で解ける、選び方の少ない入力を出す
(pj testdata crosscheck 用)。
2000 以上なら、同じ愚直解でまだ解ける N = 16 までの入力を、M と D を大きくして出す。
D_i = B c_i、M = B K + r (0 <= r < B) の形にしてあるので、倍数の重なり方は小さい K と c_i の問題と同じになる。
"""

import math
import random
import sys

MAX_M = 10**18
P = 998244353
# 選び方の数 (floor(M / D_i) の積) の上限。愚直解はこれを全部たどる。
BRUTE_LIMIT = 2 * 10**6
# 高度合成数。2^8 3^4 5^2 7^2 11 13 ... 37。
HIGHLY_COMPOSITE = 897612484786617600


def is_prime(n: int) -> bool:
    if n < 2:
        return False
    for p in (2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37):
        if n % p == 0:
            return n == p
    d, s = n - 1, 0
    while d % 2 == 0:
        d, s = d // 2, s + 1
    for a in (2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37):
        x = pow(a, d, n)
        if x in (1, n - 1):
            continue
        for _ in range(s - 1):
            x = x * x % n
            if x == n - 1:
                break
        else:
            return False
    return True


def primes_near(center: int, count: int) -> list[int]:
    """center の下と上から交互に素数を count 個取る。"""
    below, above, out = center, center + 1, []
    while len(out) < count:
        while not is_prime(below):
            below -= 1
        out.append(below)
        below -= 1
        if len(out) == count:
            break
        while not is_prime(above):
            above += 1
        out.append(above)
        above += 1
    return out


SAMPLES = [
    (7, [2, 3, 4]),
    (3, [1, 2, 2]),
    (MAX_M, [380214083, 420492929, 929717250, 666796775, 209977152, 770361643]),
]
# 愚直解でも解ける角のケース。
FIXED_SMALL = [
    (1, [1, 1]),  # 0
    (2, [1, 1]),  # 2
    (MAX_M, [MAX_M, MAX_M]),  # 0
    (MAX_M, [MAX_M] * 16),  # 0
    (40, [5, 10, 20, 40, 8, 4, 2, 1]),
]
# 愚直解では解けない角のケース。
FIXED = [
    (16, [1] * 16),  # 16!
    (15, [1] * 16),  # M < N なので 0
    (MAX_M, [1] * 16),
    (MAX_M, [MAX_M // 16] * 16),  # 倍数がちょうど 16 個なので 16!
    (MAX_M, [MAX_M] + [1] * 15),  # (M - 1)(M - 2)...(M - 15)
    (P * 10**9, [1] * 16),  # M が P の倍数なので、余りは 0
    (P * 10**9 + 7, [1] * 16),  # M - 7 が P の倍数なので、余りは 0
    (MAX_M, [2**i for i in range(16)]),  # 倍数の鎖
    (2**59 - 1, [2**i for i in range(43, 59)]),
    (720720, list(range(1, 17))),  # 720720 = lcm(1, ..., 16)
    (MAX_M, list(range(1, 17))),
    (MAX_M, primes_near(10**9, 16)),
    (MAX_M, [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53]),  # 全部の積は 10^18 を超える
    (MAX_M, [2] * 8 + [3] * 8),
]
# (N, M, D の出し方)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    (16, MAX_M, "small"),
    (16, MAX_M, "mid"),
    (16, MAX_M, "log"),
    (16, MAX_M, "equal"),
    (16, HIGHLY_COMPOSITE, "divisor"),
    (16, "random", "divisor_random"),
    (13, MAX_M, "mid"),
    (12, MAX_M, "small"),
    (12, HIGHLY_COMPOSITE, "divisor"),
    (11, MAX_M, "mid"),
    (16, MAX_M, "near_prime"),
]


def random_divisor(rng: random.Random, m: int) -> int:
    """m = HIGHLY_COMPOSITE の約数をランダムに作る。"""
    d = 1
    for p, e in ((2, 8), (3, 4), (5, 2), (7, 2), (11, 1), (13, 1), (17, 1), (19, 1), (23, 1), (29, 1), (31, 1), (37, 1)):
        d *= p ** rng.randint(0, e)
    assert m % d == 0
    return d


def planned(rng: random.Random, n: int, m: int | str, how: str) -> tuple[int, list[int]]:
    if how == "divisor_random":
        # M を約数の多い数の倍数にして、D はその約数から取る。
        base = random_divisor(rng, HIGHLY_COMPOSITE)
        m = base * rng.randint(1, MAX_M // base)
        return m, [random_divisor(rng, HIGHLY_COMPOSITE) for _ in range(n)]
    assert isinstance(m, int)
    if how == "small":
        return m, [rng.randint(1, 10) for _ in range(n)]
    if how == "mid":
        return m, [rng.randint(1, 10**6) for _ in range(n)]
    if how == "log":
        return m, [max(1, (m >> rng.randint(0, 59)) - rng.randint(0, 3)) for _ in range(n)]
    if how == "equal":
        d = rng.randint(1, m // 100)
        return m, [d] * n
    if how == "divisor":
        return m, [random_divisor(rng, m) for _ in range(n)]
    assert how == "near_prime"
    # 10^9 の近くの素数と、その 2 つの積 (M 以下のもの) を混ぜる。
    ps = primes_near(10**9 + rng.randint(-10**6, 10**6), 12)
    ds = ps[:]
    while len(ds) < n:
        a, b = rng.sample(ps, 2)
        if a * b <= m:
            ds.append(a * b)
    rng.shuffle(ds)
    return m, ds


def small_case(rng: random.Random) -> tuple[int, list[int]]:
    """M が 60 以下で、選び方の数が BRUTE_LIMIT 以下のもの。"""
    while True:
        n = rng.randint(2, 8)
        m = rng.randint(1, 60) if rng.random() < 0.2 else rng.randint(n, 60)
        how = rng.choice(["uniform", "uniform", "small", "divisor", "equal"])
        if how == "uniform":
            ds = [rng.randint(1, max(1, m // rng.choice([1, 2, 3, 4, 8]))) for _ in range(n)]
        elif how == "small":
            ds = [rng.randint(1, min(m, 4)) for _ in range(n)]
        elif how == "divisor":
            divs = [d for d in range(1, m + 1) if m % d == 0]
            ds = [rng.choice(divs) for _ in range(n)]
        else:
            ds = [rng.randint(1, m)] * n
        if math.prod(m // d for d in ds) <= BRUTE_LIMIT:
            return m, ds


def scaled_case(rng: random.Random) -> tuple[int, list[int]]:
    """D_i = B c_i、M = B K + r の形。倍数の重なり方は (K, c) の問題と同じで、数だけ大きい。"""
    while True:
        n = rng.randint(9, 16)
        k = rng.randint(2 * n, 5 * n)
        # c は互いに違うものから選び、倍数が 2 つ以上ある c を少し重ねる。
        # 倍数が 1 つしか無い c を重ねると、答えは必ず 0 になる。
        cs = rng.sample(range(max(1, k // rng.choice([3, 4, 6, 8])), k + 1), n)
        for _ in range(rng.randint(0, 2)):
            many = [c for c in cs if k // c >= 2]
            if many:
                cs[rng.randrange(n)] = rng.choice(many)
        if math.prod(k // c for c in cs) <= BRUTE_LIMIT:
            break
    b = rng.randint(1, MAX_M // (k + 1))
    m = b * k + rng.randint(0, b - 1)
    return m, [b * c for c in cs]


def case_for(seed: int, rng: random.Random) -> tuple[int, list[int]]:
    if seed >= 2000:
        return scaled_case(rng)
    if seed >= 1000:
        return small_case(rng)
    fixed = SAMPLES + FIXED_SMALL + FIXED
    if seed < len(fixed):
        return fixed[seed]
    n, m, how = PLANS[seed - len(fixed)]
    return planned(rng, n, m, how)


def main() -> None:
    seed = int(sys.argv[1])
    m, ds = case_for(seed, random.Random(seed))
    n = len(ds)
    assert 2 <= n <= 16 and 1 <= m <= MAX_M and all(1 <= d <= m for d in ds)
    print(n, m)
    print(*ds)


if __name__ == "__main__":
    main()
