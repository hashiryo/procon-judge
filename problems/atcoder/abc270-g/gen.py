"""abc270-g (Sequence in mod P) の入力を作る。T と、T 行の P A B S G を出す。
答えはケースごとに、X_0 = S、X_i = (A X_{i-1} + B) mod P で X_i = G となる最小の i (なければ -1)。

seed が 0 から count - 1 までは本番のケース。1 ファイルに 100 ケースまで入るので、似たケースをまとめる。
例、P = 2 と P = 3 の全部の組、P ≤ 13 のランダム、A = 0 (X_1 から先は B のまま)、A = 1 (等差数列)、
不動点 c = -B / (A - 1) から始まる列 (ずっと S) と不動点を探すもの (-1)、A が原始根で答えが P - 2 に
近いもの、位数の小さい A、ランダム、G をある X_i にしたもの、10^9 以下で最大の素数 999999937、
998244353 (P - 1 = 2^23 × 7 × 17)、答えが -1 で探す範囲を最後まで見るもの (A が原始根で G が不動点、
A の位数が (P - 1) / 2 で G が軌道の外)。
X_i は A ≠ 0, 1 なら c + A^i (S - c) なので、G をある X_i にするのは i から直接求められる。
seed が 1000 以上なら、X_i を最大 P 個まで順に求める愚直解で解ける、P の小さい入力を出す
(pj testdata crosscheck 用)。2000 以上なら、愚直解でまだ解ける P ≤ 10^6 の入力を出す。
"""

import random
import sys

MAX_T = 100
MAX_P = 10**9

Case = tuple[int, int, int, int, int]

SAMPLE: list[Case] = [(5, 2, 1, 1, 0), (5, 2, 2, 3, 0), (11, 1, 1, 0, 10)]
BIG = [999999937, 998244353, 999999929, 999999893, 999999757]


def is_prime(n: int) -> bool:
    """決定的な Miller-Rabin (n < 3.2 × 10^9 なら底 2, 3, 5, 7 で足りる)。"""
    if n < 2:
        return False
    for p in (2, 3, 5, 7):
        if n % p == 0:
            return n == p
    d, s = n - 1, 0
    while d % 2 == 0:
        d, s = d // 2, s + 1
    for a in (2, 3, 5, 7):
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


def random_prime(rng: random.Random, low: int, high: int) -> int:
    while True:
        p = rng.randint(low, high)
        while p <= high and not is_prime(p):
            p += 1
        if p <= high:
            return p


def prime_factors(n: int) -> list[int]:
    out, d = [], 2
    while d * d <= n:
        if n % d == 0:
            out.append(d)
            while n % d == 0:
                n //= d
        d += 1
    return out + ([n] if n > 1 else [])


def divisors(n: int) -> list[int]:
    out = [1]
    for q in prime_factors(n):
        e = 0
        while n % q == 0:
            n //= q
            e += 1
        out = [d * q**k for d in out for k in range(e + 1)]
    return sorted(out)


def element_of_order(rng: random.Random, p: int, order: int) -> int:
    """位数がちょうど order (P - 1 の約数) の元。原始根を探して冪にする。"""
    qs = prime_factors(p - 1)
    while True:
        g = rng.randint(1, p - 1)
        if p == 2 or all(pow(g, (p - 1) // q, p) != 1 for q in qs):
            return pow(g, (p - 1) // order, p)


def term(p: int, a: int, b: int, s: int, i: int) -> int:
    """X_i を直接求める。"""
    if i == 0:
        return s
    if a == 0:
        return b
    if a == 1:
        return (s + i * b) % p
    c = fixed_point(p, a, b)
    return (c + pow(a, i, p) * (s - c)) % p


def fixed_point(p: int, a: int, b: int) -> int:
    """A ≠ 1 のときの不動点 -B / (A - 1)。"""
    return -b * pow(a - 1, p - 2, p) % p


def random_case(rng: random.Random, p: int) -> Case:
    return (p, *(rng.randrange(p) for _ in range(4)))


def hit_case(rng: random.Random, p: int, a: int | None = None, i: int | None = None) -> Case:
    """G をある X_i にする。答えは i 以下 (周期が短ければ小さくなる)。"""
    a = rng.randrange(p) if a is None else a
    b, s = rng.randrange(p), rng.randrange(p)
    i = rng.randrange(p) if i is None else i
    return p, a, b, s, term(p, a, b, s, i)


def file_for(seed: int, rng: random.Random) -> list[Case]:
    if seed >= 1000:
        # 愚直解が P 項まで並べて解ける大きさ。
        high = rng.choice([10**4, 10**5, 10**6]) if seed >= 2000 else rng.choice([3, 20, 200, 3000])
        t = rng.randint(1, 10 if seed >= 2000 else 30)
        out = []
        for _ in range(t):
            p = random_prime(rng, 2, high)
            kind = rng.randrange(5)
            if kind == 0:
                out.append(random_case(rng, p))
            elif kind == 1:
                out.append(hit_case(rng, p))
            elif kind == 2:
                # A = 0 か 1。
                out.append(hit_case(rng, p, a=rng.randrange(min(2, p))))
            elif kind == 3 and p > 2:
                # 不動点から始めるか、不動点を探す。
                a, b = rng.randrange(2, p), rng.randrange(p)
                c = fixed_point(p, a, b)
                if rng.random() < 0.5:
                    out.append((p, a, b, c, rng.choice([c, rng.randrange(p)])))
                else:
                    out.append((p, a, b, rng.randrange(p), c))
            else:
                # いろいろな位数の A (原始根も含む)。
                order = rng.choice(divisors(p - 1))
                out.append(hit_case(rng, p, a=element_of_order(rng, p, order)))
        return out
    if seed == 0:
        return SAMPLE
    if seed == 1:
        return [(p, a, b, s, g) for p in (2, 3) for a in range(p) for b in range(p) for s in range(p) for g in range(p)]
    if seed == 2:
        return [random_case(rng, rng.choice([5, 7, 11, 13])) for _ in range(MAX_T)]
    if seed == 3:
        # A = 0。G = S なら 0、G = B なら 1。
        out = []
        for _ in range(MAX_T):
            p = rng.choice([random_prime(rng, 2, MAX_P), rng.choice(BIG)])
            b, s = rng.randrange(p), rng.randrange(p)
            if rng.random() < 0.2:
                s = b
            out.append((p, 0, b, s, rng.choice([s, b, rng.randrange(p)])))
        return out
    if seed == 4:
        # A = 1。B = 0 なら S のまま。答えが P - 1 になる G = S - B も入れる。
        out = []
        for k in range(MAX_T):
            p = rng.choice([random_prime(rng, 2, MAX_P), rng.choice(BIG)])
            b = 0 if k % 5 == 0 else rng.randrange(1, p)
            s = rng.randrange(p)
            g = rng.choice([s, (s - b) % p, rng.randrange(p)])
            out.append((p, 1, b, s, g))
        return out
    if seed == 5:
        # 不動点から始める (ずっと S) か、不動点を探す (-1)。
        out = []
        for k in range(MAX_T):
            p = rng.choice([random_prime(rng, 3, MAX_P), rng.choice(BIG)])
            a, b = rng.randrange(2, p), rng.randrange(p)
            c = fixed_point(p, a, b)
            if k % 2 == 0:
                out.append((p, a, b, c, rng.choice([c, rng.randrange(p)])))
            else:
                out.append((p, a, b, rng.randrange(p), c))
        return out
    if seed == 6:
        # A が原始根で、答えが P - 2 の近く。
        out = []
        for _ in range(MAX_T):
            p = rng.choice([random_prime(rng, 10**8, MAX_P), rng.choice(BIG)])
            out.append(hit_case(rng, p, a=element_of_order(rng, p, p - 1), i=p - 2 - rng.randint(0, 100)))
        return out
    if seed == 7:
        # 位数の小さい A。周期が短いので、G が軌道にあれば答えは小さく、なければ -1。
        out = []
        for _ in range(MAX_T):
            p = rng.choice([random_prime(rng, 10**8, MAX_P), rng.choice(BIG)])
            order = rng.choice([d for d in divisors(p - 1) if d < 1000])
            case = hit_case(rng, p, a=element_of_order(rng, p, order), i=rng.randrange(2 * order))
            out.append(case if rng.random() < 0.7 else case[:4] + (rng.randrange(p),))
        return out
    if seed == 8:
        return [random_case(rng, random_prime(rng, 2, MAX_P)) for _ in range(MAX_T)]
    if seed == 9:
        return [hit_case(rng, random_prime(rng, 2, MAX_P)) for _ in range(MAX_T)]
    if seed == 10:
        return [hit_case(rng, 999999937) for _ in range(MAX_T)]
    if seed == 11:
        return [rng.choice([random_case, hit_case])(rng, 998244353) for _ in range(MAX_T)]
    if seed == 12:
        # 答えが -1 で、探す範囲を最後まで見る。A が原始根で G が不動点。
        out = []
        for _ in range(MAX_T):
            p = rng.choice([random_prime(rng, 9 * 10**8, MAX_P), rng.choice(BIG)])
            a, b = element_of_order(rng, p, p - 1), rng.randrange(p)
            c = fixed_point(p, a, b)
            s = rng.randrange(p)
            if s == c:
                s = (c + 1) % p
            out.append((p, a, b, s, c))
        return out
    if seed == 13:
        # A の位数が (P - 1) / 2 で、G が S の軌道の外 (A^i が平方剰余にしかならない)。
        out = []
        for _ in range(MAX_T):
            p = rng.choice([random_prime(rng, 9 * 10**8, MAX_P), rng.choice(BIG)])
            a, b = element_of_order(rng, p, (p - 1) // 2), rng.randrange(p)
            c = fixed_point(p, a, b)
            s = (c + rng.randrange(1, p)) % p
            # X_i - c = A^i (S - c) なので、G - c = (S - c) × (平方非剰余) にすると G には着かない。
            nonres = element_of_order(rng, p, p - 1)
            out.append((p, a, b, s, (c + (s - c) * nonres) % p))
        return out
    assert seed == 14
    # 小さい P のランダム。
    return [rng.choice([random_case, hit_case])(rng, random_prime(rng, 2, 50)) for _ in range(MAX_T)]


COUNT = 15


def main() -> None:
    seed = int(sys.argv[1])
    cases = file_for(seed, random.Random(seed))
    assert 1 <= len(cases) <= MAX_T
    for p, a, b, s, g in cases:
        assert 2 <= p <= MAX_P and is_prime(p) and all(0 <= v < p for v in (a, b, s, g))
    sys.stdout.write(f"{len(cases)}\n" + "".join(f"{p} {a} {b} {s} {g}\n" for p, a, b, s, g in cases))


if __name__ == "__main__":
    main()
