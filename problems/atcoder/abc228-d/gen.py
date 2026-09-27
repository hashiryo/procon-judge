"""abc228-d (Linear Probing) の入力を作る。Q と、Q 個の t x を出す。N = 2^20 は入力に無い定数。

seed が 0 から count - 1 までは本番のケース。例の 1 つ、角のケース、Q = 10^3 から 2 * 10^4 の
中くらいのケース、大きいケースの順に並べる。
角のケースは、Q = 1、書いた場所をすぐ読む、N - 1 から 0 へ回り込む、回り込んだ先の 0 から始まる
塊を飛び越す (lib.cpp は回り込んだあと塊をもう 1 つだけ見る)、など。
大きいケースは Q = 2 * 10^5 で、ランダム、1 つの余りに集中して N - 1 をまたぐ大きな塊、偶数の位置を
先に埋めて区間を 10^5 個作ってから奇数の位置で繋ぐ、回り込みの前後の狭い窓に集中、余りが全部違う、
x が小さい、の 6 つ。1 つの余りに集中した形は、1 つずつ進める素朴な解では遅い。ほかに、読み込みが
大半のもの (Q = 10^5) と、0 から始まる塊 (Q = 5 * 10^4) を入れる。データが大きくなりすぎないよう、
ランダム以外の大きいケースでは x を 10^12 までにする。
seed が 1000 以上なら、配列の上で 1 つずつ進める愚直解で解ける小さい入力を出す (pj testdata crosscheck 用)。
2000 以上なら、同じ愚直解でまだ解ける Q = 2 * 10^4 までの入力を出す。
"""

import random
import sys

N = 2**20
MAX_Q = 2 * 10**5
MAX_X = 10**18

SAMPLES = [
    [(1, 1048577), (1, 1), (2, 2097153), (2, 3)],
]


def lift(rng: random.Random, residue: int, top: int = MAX_X) -> int:
    """余りが residue で top 以下の x を 1 つ選ぶ。"""
    return residue + rng.randint(0, (top - residue) // N) * N


def fixed_cases(rng: random.Random) -> list[list[tuple[int, int]]]:
    wrap = [(1, lift(rng, N - 2)) for _ in range(4)]  # N - 2, N - 1, 0, 1 に入る
    wrap += [(2, lift(rng, r % N)) for r in (N - 3, N - 2, N - 1, N, N + 1, N + 2)]
    jump = [(1, lift(rng, 0)) for _ in range(3)]  # 0, 1, 2 に入る
    jump += [(1, lift(rng, N - 1)) for _ in range(3)]  # N - 1、3、4 に入る
    jump += [(2, lift(rng, r % N)) for r in (N - 2, N - 1, 0, 1, 2, 3, 4, 5)]
    return [
        [(2, MAX_X)],  # Q = 1
        [(1, MAX_X), (2, MAX_X)],
        [(1, 0), (2, 0), (2, N), (2, 1)],
        wrap,
        jump,
        [(1, N - 1), (1, N - 1), (1, MAX_X), (2, N - 1), (2, 0), (2, MAX_X)],
    ]


def interleave(rng: random.Random, inserts: list[int], queries: list[int]) -> list[tuple[int, int]]:
    """書き込みの順を保ったまま、読み込みをランダムな位置に混ぜる。最後は必ず読み込みにする。"""
    kinds = [1] * len(inserts) + [2] * (len(queries) - 1)
    rng.shuffle(kinds)
    kinds.append(2)
    it, qt = iter(inserts), iter(queries)
    return [(t, next(it) if t == 1 else next(qt)) for t in kinds]


def big_case(rng: random.Random, shape: str) -> list[tuple[int, int]]:
    q = MAX_Q
    if shape == "random":
        ops = [(rng.randint(1, 2), rng.randint(0, MAX_X)) for _ in range(q - 1)]
        return ops + [(2, rng.randint(0, MAX_X))]
    if shape == "one_residue":
        # 15 万個を同じ余りに入れて N - 1 をまたぐ塊を作り、読み込みは塊の中と前後を狙う。
        start = N - 75000
        inserts = [lift(rng, start, 10**12) for _ in range(150000)]
        queries = [lift(rng, (start + rng.randint(-100, 150100)) % N, 10**12) for _ in range(q - 150000)]
        return interleave(rng, inserts, queries)
    if shape == "zero_block":
        # 0 から始まる塊。回り込まない側の大きな塊として、Q = 5 * 10^4 で入れる。
        inserts = [lift(rng, 0, 10**12) for _ in range(30000)]
        queries = [lift(rng, rng.randint(0, 30100) if rng.random() < 0.9 else rng.randrange(N), 10**12)
                   for _ in range(20000)]
        return interleave(rng, inserts, queries)
    if shape == "even_then_odd":
        # 偶数の位置を先に埋めて区間を 10^5 個作り、そのあと奇数の位置へ書いて区間を繋いでいく。
        evens = [2 * i for i in range(100000)]
        rng.shuffle(evens)
        odds = [2 * rng.randrange(100000) + 1 for _ in range(50000)]
        head = [(1, lift(rng, r, 10**12)) for r in evens]
        tail = interleave(rng, [lift(rng, r, 10**12) for r in odds],
                          [lift(rng, rng.randrange(200100), 10**12) for _ in range(q - 150000)])
        return head + tail
    if shape == "wrap_window":
        # 回り込みの前後 1000 か所に集中させる。塊は N - 500 あたりから 0 を越えて伸びる。
        def near() -> int:
            return (N - 500 + rng.randrange(1000)) % N
        inserts = [lift(rng, near(), 10**12) for _ in range(120000)]
        queries = [lift(rng, (N - 600 + rng.randrange(121000)) % N, 10**12) for _ in range(q - 120000)]
        return interleave(rng, inserts, queries)
    if shape == "mostly_queries":
        # 書き込みは 2000 個だけで、あとは読み込み。出力が多いので Q = 10^5 にする。
        residues = rng.sample(range(N), 1000)
        inserts = [lift(rng, rng.choice(residues), 10**12) for _ in range(2000)]
        queries = [lift(rng, (rng.choice(residues) + rng.randint(0, 3)) % N, 10**12) for _ in range(98000)]
        return interleave(rng, inserts, queries)
    if shape == "distinct":
        residues = rng.sample(range(N), 150000)
        inserts = [lift(rng, r, 10**12) for r in residues]
        queries = [lift(rng, rng.choice(residues) if rng.random() < 0.7 else rng.randrange(N), 10**12)
                   for _ in range(q - 150000)]
        return interleave(rng, inserts, queries)
    assert shape == "small_x"
    ops = [(rng.randint(1, 2), rng.randrange(2 * N)) for _ in range(q - 1)]
    return ops + [(2, rng.randrange(2 * N))]


BIG = ["random", "one_residue", "even_then_odd", "wrap_window", "distinct", "small_x", "mostly_queries", "zero_block"]
MEDIUM = 4  # 角のケースのあとに置く、Q = 10^3 から 2 * 10^4 の入力 (愚直解でも解ける)
COUNT = len(SAMPLES) + 6 + MEDIUM + len(BIG)


def small_case(rng: random.Random, q: int) -> list[tuple[int, int]]:
    """余りを回り込みの前後と少数のランダムな値に集め、ぶつかりと回り込みを起こしやすくする。"""
    pool = [N - 3, N - 2, N - 1, 0, 1, 2, 3] + [rng.randrange(N) for _ in range(3)]
    if q > 100:
        pool += [(N - q // 2 + rng.randrange(q)) % N for _ in range(20)]
    ops = []
    for _ in range(q):
        t = 1 if rng.random() < 0.6 else 2
        r = rng.choice(pool)
        if t == 2 and rng.random() < 0.5:
            r = (r + rng.randint(-3, 3)) % N
        ops.append((t, lift(rng, r) if rng.random() < 0.7 else lift(rng, r, 10 * N)))
    if all(t == 1 for t, _ in ops):
        ops[-1] = (2, ops[-1][1])
    return ops


def case_for(seed: int, rng: random.Random) -> list[tuple[int, int]]:
    if seed >= 2000:
        return small_case(rng, rng.randint(1000, 20000))
    if seed >= 1000:
        return small_case(rng, rng.randint(1, 40))
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    fixed = fixed_cases(rng)
    if seed < len(fixed):
        return fixed[seed]
    seed -= len(fixed)
    if seed < MEDIUM:
        return small_case(rng, rng.randint(1000, 20000))
    return big_case(rng, BIG[seed - MEDIUM])


def main() -> None:
    seed = int(sys.argv[1])
    ops = case_for(seed, random.Random(seed))
    assert 1 <= len(ops) <= MAX_Q and any(t == 2 for t, _ in ops)
    assert all(t in (1, 2) and 0 <= x <= MAX_X for t, x in ops)
    sys.stdout.write("\n".join([str(len(ops))] + [f"{t} {x}" for t, x in ops]) + "\n")


if __name__ == "__main__":
    main()
