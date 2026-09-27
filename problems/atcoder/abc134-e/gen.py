"""abc134-e (Sequence Decomposing) の入力を作る。N と、1 行に 1 つずつ A_1 から A_N を出す。答えは最長の広義単調減少部分列の長さ。

seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース、N = 10^5 のいろいろな形の列と、
N = 5000 の順列と全部 10^9 の列。形は、ランダム、全部 0、狭義単調増加 (答えは 1)、狭義単調減少 (答えは N)、
同じ値を含む単調減少 (答えは N)、同じ値を含む単調増加 (答えは一番多い値の個数)、値の幅が狭いランダム、
増加する塊を値の大きい塊から並べたもの (答えは塊の数)、0 と 10^9 の交互 (答えは N / 2)、ほぼ単調減少。
単調減少では、提出の二分探索の配列が N まで伸びる。
seed が 1000 以上なら、DAG の最小パス被覆で解ける小さい入力を出す (pj testdata crosscheck 用)。
2000 以上なら、同じ愚直解でまだ解ける N = 300 までの入力を出す。
"""

import random
import sys

MAX_N = 10**5
MAX_A = 10**9

SAMPLES = [
    [2, 1, 4, 5, 3],
    [0, 0, 0, 0],
]
FIXED = [
    [0],
    [MAX_A],
    [0, MAX_A],
    [MAX_A, 0],
    [7, 7],
    [3, 1, 2],
    [1, 2, 3, 3, 2, 1, 0],
]
# (N, 列の形)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    (1000, "random"),
    (MAX_N, "random"),
    (MAX_N, "equal_min"),
    (MAX_N, "increasing"),
    (MAX_N, "decreasing"),
    (MAX_N, "nonincreasing"),
    (MAX_N, "nondecreasing"),
    (MAX_N, "narrow"),
    (MAX_N, "blocks"),
    (MAX_N, "zigzag"),
    (MAX_N, "almost_decreasing"),
    (5000, "permutation"),
    (5000, "equal_max"),
]
SHAPES = [
    "random", "equal_min", "equal_max", "increasing", "decreasing", "nonincreasing", "nondecreasing",
    "narrow", "blocks", "zigzag", "permutation", "almost_decreasing",
]


def sequence(rng: random.Random, n: int, shape: str) -> list[int]:
    if shape == "random":
        return [rng.randint(0, MAX_A) for _ in range(n)]
    if shape == "equal_min":
        return [0] * n
    if shape == "equal_max":
        return [MAX_A] * n
    if shape == "increasing":
        return sorted(rng.sample(range(MAX_A + 1), n))
    if shape == "decreasing":
        return sorted(rng.sample(range(MAX_A + 1), n), reverse=True)
    if shape == "nonincreasing":
        return sorted((rng.randint(0, max(1, n // 10)) for _ in range(n)), reverse=True)
    if shape == "nondecreasing":
        return sorted(rng.randint(0, max(1, n // 100)) for _ in range(n))
    if shape == "narrow":
        return [rng.randint(0, 9) for _ in range(n)]
    if shape == "blocks":
        # 長さ w の増加する塊を、値の大きい塊から順に並べる。答えは塊の数。
        w = max(1, int(n**0.5))
        out = []
        for b in range((n + w - 1) // w):
            base = MAX_A - (b + 1) * 2 * w
            out += [base + i for i in range(min(w, n - b * w))]
        return out
    if shape == "zigzag":
        return [0 if i % 2 == 0 else MAX_A for i in range(n)]
    if shape == "permutation":
        perm = list(range(n))
        rng.shuffle(perm)
        return perm
    assert shape == "almost_decreasing"
    out = sorted((rng.randint(0, MAX_A) for _ in range(n)), reverse=True)
    for _ in range(max(1, n // 100)):
        i, j = rng.randrange(n), rng.randrange(n)
        out[i], out[j] = out[j], out[i]
    return out


def small_values(rng: random.Random, n: int) -> list[int]:
    """愚直解向けの列。値の幅を狭くして同じ値を多くするか、端の値を混ぜる。"""
    how = rng.randrange(4)
    if how == 0:
        return [rng.randint(0, 3) for _ in range(n)]
    if how == 1:
        return [rng.randint(0, n) for _ in range(n)]
    if how == 2:
        return [rng.choice([0, 1, MAX_A - 1, MAX_A]) for _ in range(n)]
    return sequence(rng, n, rng.choice(SHAPES))


def case_for(seed: int, rng: random.Random) -> list[int]:
    if seed >= 1000:
        n = rng.randint(13, 300) if seed >= 2000 else rng.randint(1, 12)
        return small_values(rng, n)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    n, shape = PLANS[seed - len(FIXED)]
    return sequence(rng, n, shape)


def main() -> None:
    seed = int(sys.argv[1])
    a = case_for(seed, random.Random(seed))
    assert 1 <= len(a) <= MAX_N and all(0 <= v <= MAX_A for v in a)
    sys.stdout.write("\n".join(map(str, [len(a)] + a)) + "\n")


if __name__ == "__main__":
    main()
