"""abc360-g (Suitable Edit for LIS) の入力を作る。N と A_1 ... A_N を出す。

答えは、変えたあとの最長増加部分列の長さの最大で、変える前の LIS か、それに 1 を足したもの。
seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース、N = 2 × 10^5 と 5 × 10^4 の列。
角のケースには、y を 0 以下や 10^9 より大きくしないと伸ばせない列 ([1, 1, 2, 3] など) と、LIS の隣どうしの
値の差が 1 で間に入れられない列 ([1, 2, 3, 3, 4, 5] など) を入れる。
大きい列は、ランダム、順列、全部同じ、1 から N の増加 (答えは N)、間の空いた増加、減少 (答えは 2)、
値が 1 ずつ増える広義増加で先頭と末尾だけが 1 個のもの (答えは LIS のまま) と先頭が 2 個のもの (LIS + 1)、
そこに値を 1 つ飛ばした所を 1 か所だけ作ったもの、値の幅が狭いランダム、減る塊を増える順に並べたもの
(LIS の各段に候補がたくさんある)、1 から N の増加の 1 つを変えたもの。
seed が 1000 以上なら、変える位置と値を全部試す愚直解で解ける小さい入力を出す (pj testdata crosscheck 用)。
2000 以上なら、同じ愚直解でまだ解ける N = 60 までの入力を出す。
"""

import random
import sys

MAX_N = 2 * 10**5
MAX_A = 10**9

SAMPLES = [
    [3, 2, 2, 4],
    [4, 5, 3, 6, 7],
]
FIXED = [
    [1],
    [MAX_A],
    [1, 1],  # A_1 を 0 にする
    [MAX_A, MAX_A],  # A_2 を 10^9 + 1 にする
    [2, 1],
    [1, 2, 3],
    [2, 2, 2],
    [1, 1, 2, 3],  # A_1 を 0 にしないと 4 にならない
    [MAX_A - 2, MAX_A - 1, MAX_A, MAX_A],
    [1, 2, 3, 3, 4, 5],  # 入れる隙間が無い
    [1, 5, 2, 3],
    [5, 1, 2, 3, 4],
    [1, 2, 3, 4, 1],
    [1, 2, 1, 2],
]
# (N, 形)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    (MAX_N, "random"),
    (MAX_N, "permutation"),
    (MAX_N, "equal"),
    (MAX_N, "consecutive"),
    (MAX_N, "decreasing"),
    (MAX_N, "tight"),
    (MAX_N, "tight_head"),
    (MAX_N, "tight_skip"),
    (MAX_N, "blocks"),
    (50000, "increasing"),
    (50000, "narrow"),
    (50000, "one_changed"),
]
SHAPES = [
    "random", "permutation", "equal", "consecutive", "decreasing", "tight", "tight_head", "tight_skip",
    "blocks", "increasing", "narrow", "one_changed",
]


def tight(rng: random.Random, n: int, head: int, skip: bool) -> list[int]:
    """1 ずつ増える値を並べ、途中の値を何回か重ねた広義増加の列。先頭の値は head 個、末尾の値は 1 個。
    skip なら、途中で値を 1 つ飛ばし、飛ばした所の手前の値を 2 個にする (そこにだけ 1 つ入る)。
    """
    assert n >= head + 1
    counts = [head]
    left = n - head - 1
    while left > 0:
        c = min(left, rng.choice([1, 1, 1, 2, 3]))
        counts.append(c)
        left -= c
    counts.append(1)
    values = list(range(1, len(counts) + 1))
    if skip and len(counts) >= 4:
        # values[k] と values[k + 1] の差を 2 にし、values[k] を 2 個以上にする (個数は 2 個以上の値から借りる)。
        k = rng.randrange(1, len(counts) - 2)
        values = values[: k + 1] + [v + 1 for v in values[k + 1 :]]
        donors = [j for j in range(1, len(counts) - 1) if j != k and counts[j] >= 2]
        if counts[k] == 1 and donors:
            counts[rng.choice(donors)] -= 1
            counts[k] += 1
    out = []
    for v, c in zip(values, counts):
        out += [v] * c
    assert len(out) == n
    return out


def sequence(rng: random.Random, n: int, shape: str) -> list[int]:
    if shape == "random":
        return [rng.randint(1, MAX_A) for _ in range(n)]
    if shape == "permutation":
        perm = list(range(1, n + 1))
        rng.shuffle(perm)
        return perm
    if shape == "equal":
        return [rng.randint(1, MAX_A)] * n
    if shape == "consecutive":
        return list(range(1, n + 1))
    if shape == "increasing":
        return sorted(rng.sample(range(1, MAX_A + 1), n))
    if shape == "decreasing":
        return sorted(rng.sample(range(1, MAX_A + 1), n), reverse=True)
    if shape == "tight":
        return tight(rng, n, 1, False)
    if shape == "tight_head":
        return tight(rng, n, 2, False)
    if shape == "tight_skip":
        return tight(rng, n, 1, True)
    if shape == "narrow":
        return [rng.randint(1, 10) for _ in range(n)]
    if shape == "blocks":
        # 長さ w の減る塊を、値の大きくなる順に並べる。LIS は塊ごとに 1 つずつ取る。
        w = max(1, int(n**0.5))
        out = []
        for b in range((n + w - 1) // w):
            out += [b * w + w - i for i in range(min(w, n - b * w))]
        return out
    assert shape == "one_changed"
    out = list(range(1, n + 1))
    out[rng.randrange(n)] = rng.randint(1, n)
    return out


def small_case(rng: random.Random, n: int) -> list[int]:
    how = rng.randrange(5)
    if how == 0:
        return [rng.randint(1, 6) for _ in range(n)]
    if how == 1:
        return [rng.choice([1, 2, 3, MAX_A - 2, MAX_A - 1, MAX_A]) for _ in range(n)]
    if how == 2 and n >= 3:
        return sequence(rng, n, rng.choice(["tight", "tight_head", "tight_skip"]))
    if how == 3:
        return sequence(rng, n, rng.choice([s for s in SHAPES if n >= 3 or not s.startswith("tight")]))
    return [rng.randint(1, MAX_A) for _ in range(n)]


def case_for(seed: int, rng: random.Random) -> list[int]:
    if seed >= 1000:
        return small_case(rng, rng.randint(11, 60) if seed >= 2000 else rng.randint(1, 10))
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
    assert 1 <= len(a) <= MAX_N and all(1 <= v <= MAX_A for v in a)
    sys.stdout.write(f"{len(a)}\n{' '.join(map(str, a))}\n")


if __name__ == "__main__":
    main()
