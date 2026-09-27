"""abc354-f (Useless for LIS) の入力を作る。T と、T 個の N と A を出す。

seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース、N の和が 2 * 10^5 のいろいろな形。
角のケースは、N = 1、値が 1 から 4 の長さ 5 までの列を全部並べたもの、T = 2 * 10^5 (N = 1 ばかり) など。
形は、ランダム、順列、単調増加 (全部が LIS に入る)、単調減少と全部同じ (LIS の長さが 1 なので全部入る)、
値の種類が少ないもの、増加する塊を下がりながら並べたもの (いちばん長い塊だけが LIS になる)、ジグザグ、
LIS に入らない値を散らしたもの。
seed が 1000 以上なら、両側からの O(N^2) の DP で解ける小さい入力を出す (pj testdata crosscheck 用)。
2000 以上なら、DP でまだ解ける N <= 2000 の入力を出す。
"""

import itertools
import random
import sys

MAX_SUM = 2 * 10**5
MAX_A = 10**9

SAMPLES = [
    [[2, 1, 4, 5, 3]],
    [[2, 5, 3, 4, 3, 4], [10000, 1000, 100, 1, 10]],
]
# (T, N, 列の形)。本番のケースのうち例のあとに並べる。
PLANS = [
    (1, 1, "min"),
    (1, 1, "max"),
    (0, 0, "all_small"),  # 値が 1 から 4 の、長さ 1 から 5 の列を全部
    (MAX_SUM, 1, "few"),
    (MAX_SUM // 3, 3, "few"),
    (1000, 10, "small_values"),
    (1, 1000, "decreasing"),
    (1, 1000, "same"),
    (200, 1000, "permutation"),
    (1, MAX_SUM, "random"),
    (1, MAX_SUM, "permutation"),
    (1, MAX_SUM, "increasing"),
    (1, MAX_SUM, "few"),
    (1, MAX_SUM, "blocks"),
    (1, MAX_SUM, "zigzag"),
    (1, MAX_SUM, "noise"),
    (10, MAX_SUM // 10, "blocks"),
]
COUNT = len(SAMPLES) + len(PLANS)
SHAPES = ["random", "permutation", "increasing", "decreasing", "same", "few", "blocks", "zigzag", "noise"]


def sequence(rng: random.Random, n: int, shape: str) -> list[int]:
    if shape == "min":
        return [1] * n
    if shape == "max":
        return [MAX_A] * n
    if shape == "random":
        return [rng.randint(1, MAX_A) for _ in range(n)]
    if shape == "permutation":
        a = list(range(1, n + 1))
        rng.shuffle(a)
        return a
    if shape == "increasing":
        return sorted(rng.sample(range(1, MAX_A + 1), n))
    if shape == "decreasing":
        return sorted(rng.sample(range(1, MAX_A + 1), n), reverse=True)
    if shape == "same":
        return [rng.randint(1, MAX_A)] * n
    if shape == "few":
        return [rng.randint(1, 10) for _ in range(n)]
    if shape == "small_values":
        # 小さい入力で同じ値を多くする。
        return [rng.randint(1, max(1, n // 2)) for _ in range(n)]
    if shape == "blocks":
        # 増加する塊を、値の範囲を下げながら並べる。塊の長さはばらばらで、長いものだけが LIS になる。
        out: list[int] = []
        top = MAX_A
        width = max(1, int(n**0.5))
        while len(out) < n:
            size = min(n - len(out), rng.randint(max(1, width - 3), width))
            out += sorted(rng.sample(range(top - 10 * width + 1, top + 1), size))
            top -= 10 * width
        return out
    if shape == "zigzag":
        return [i // 2 + 1 + (i % 2) * 2 for i in range(n)]
    assert shape == "noise"
    # 増加列 1, 2, ... に、LIS に入れない値 (前後より大きすぎる値や小さすぎる値) を混ぜる。
    out = []
    v = 1
    for _ in range(n):
        if rng.random() < 0.3:
            out.append(rng.choice([1, MAX_A]))
        else:
            v += rng.randint(1, 3)
            out.append(v)
    return out


def all_small() -> list[list[int]]:
    return [list(a) for n in range(1, 6) for a in itertools.product(range(1, 5), repeat=n)]


def case_for(seed: int, rng: random.Random) -> list[list[int]]:
    if seed >= 1000:
        limit = 2000 if seed >= 2000 else 10
        t = rng.randint(1, 5)
        return [sequence(rng, rng.randint(1, limit), rng.choice(SHAPES + ["small_values"] * 3)) for _ in range(t)]
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    t, n, shape = PLANS[seed - len(SAMPLES)]
    if shape == "all_small":
        return all_small()
    return [sequence(rng, n, shape) for _ in range(t)]


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    cases = case_for(seed, rng)
    assert 1 <= len(cases) <= MAX_SUM and sum(map(len, cases)) <= MAX_SUM
    assert all(len(a) >= 1 and all(1 <= x <= MAX_A for x in a) for a in cases)
    out = [str(len(cases))]
    for a in cases:
        out += [str(len(a)), " ".join(map(str, a))]
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
