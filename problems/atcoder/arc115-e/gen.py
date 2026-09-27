"""arc115-e (LEQ and NEQ) の入力を作る。N と A_1 ... A_N を出す。答えは 998244353 で割った余り。

seed が 0 から count - 1 までは本番のケース。例の 2 つ、角のケース、N = 5 × 10^5 と 10^5 のいろいろな形の列。
角のケースは、N = 2、A が全部 1 (答え 0)、全部 2 (答え 2)、全部同じ値 K (答えは K (K - 1)^(N - 1))。
A が広義単調増加なら、X_(i+1) は A_(i+1) - 1 通りずつ選べるので、答えは A_1 (A_2 - 1) ... (A_N - 1) になる。
大きいケースは、ランダム、単調増加、単調減少、山と谷、ジグザグ、1 と 10^9 の交互、1 から 3 だけ (答えが 0 に
ならないもの)、値が 30 種類、1 から 1000、10^9 の近く、全部 10^9。CartesianTree は同じ値を添字で比べるので、
同じ値の多い列を多めに入れる。10^9 までの値を 5 × 10^5 個並べると入力が 5 MB になるので、N = 5 × 10^5 の
ケースは値の小さいものを多くする。
seed が 1000 以上なら、値を A_i の区切りでまとめて DP する愚直解で解ける N の小さい入力を出す (pj testdata crosscheck 用)。
2000 以上なら、愚直解でまだ解ける N = 2000 までの入力を出す。愚直解は値の種類が少なければ N = 5 × 10^5 でも速い。
値の上限が小さいと隣り合う 1 で答えが 0 になりやすいので、8 割は隣り合う 1 の後ろを 2 にする。
"""

import random
import sys

MAX_N = 5 * 10**5
MAX_A = 10**9

SAMPLES = [
    [2, 3, 2],
    [158260522, 877914575, 602436426, 24979445, 861648772, 623690081, 433933447, 476190629, 262703497, 211047202],
]
FIXED = [
    [1, 1],  # 答え 0
    [1, 2],
    [2, 1],
    [MAX_A, MAX_A],
    [1, MAX_A, 1],
    [MAX_A, 1, MAX_A],
    [3, 3, 3, 3, 3],  # 3 × 2^4
]
# (N, 列の形)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    (MAX_N, "ones"),  # 答え 0
    (MAX_N, "twos"),  # 答え 2
    (MAX_N, "random"),
    (MAX_N, "increasing"),
    (MAX_N, "tiny"),
    (MAX_N, "kinds30"),
    (MAX_N, "dup1000"),
    (MAX_N, "alternate"),
    (10**5, "decreasing"),
    (10**5, "mountain"),
    (10**5, "valley"),
    (10**5, "zigzag"),
    (10**5, "near_max"),
    (10**5, "max"),
    (1000, "random"),
]
COUNT = len(SAMPLES) + len(FIXED) + len(PLANS)
SHAPES = ["random", "increasing", "decreasing", "mountain", "valley", "zigzag", "tiny", "kinds30", "dup1000",
          "alternate", "near_max", "max", "ones", "twos"]


def sequence(rng: random.Random, n: int, shape: str, top: int = MAX_A) -> list[int]:
    """長さ n の列。値は 1 以上 top 以下。"""
    if shape == "ones":
        return [1] * n
    if shape == "twos":
        return [2] * n
    if shape == "max":
        return [top] * n
    if shape == "tiny":
        # 1 から 3 をでたらめに並べると、2 以下の値が続く所の 1 の偶奇が食い違って答えがほぼ 0 になる。
        # 条件を満たす X を先に決め、A_i を X_i 以上にして、答えが 0 にならないようにする。
        x = [rng.randint(1, min(top, 3))]
        while len(x) < n:
            v = rng.randint(1, min(top, 3))
            if v != x[-1] or top == 1:
                x.append(v)
        return [rng.randint(v, min(top, 3)) for v in x]
    if shape == "kinds30":
        kinds = [rng.randint(1, min(top, 10**6)) for _ in range(30)]
        return [rng.choice(kinds) for _ in range(n)]
    if shape == "dup1000":
        return [rng.randint(1, min(top, 1000)) for _ in range(n)]
    if shape == "alternate":
        return [1 if i % 2 == 0 else top for i in range(n)]
    if shape == "near_max":
        return [rng.randint(max(1, top - 1000), top) for _ in range(n)]
    values = [rng.randint(1, top) for _ in range(n)]
    if shape == "increasing":
        return sorted(values)
    if shape == "decreasing":
        return sorted(values, reverse=True)
    if shape in ("mountain", "valley"):
        values.sort(reverse=shape == "valley")
        return values[0::2] + values[1::2][::-1]
    if shape == "zigzag":
        values.sort()
        low, high = values[: n // 2], values[n // 2:][::-1]
        return [high[i // 2] if i % 2 == 0 else low[i // 2] for i in range(n)]
    assert shape == "random"
    return values


def case_for(seed: int, rng: random.Random) -> list[int]:
    if seed >= 1000:
        # 値の区切りの DP で解ける大きさ。値の上限も小さいものと大きいものを混ぜる。
        n = rng.randint(13, 2000) if seed >= 2000 else rng.randint(2, 12)
        top = rng.choice([2, 3, 5, 10, 100, MAX_A])
        a = sequence(rng, n, rng.choice(SHAPES), top)
        if rng.random() < 0.8:
            # 隣り合う 1 があると答えが 0 に決まるので、たいていは 2 にずらす。
            a = [2 if v == 1 and i > 0 and a[i - 1] == 1 else v for i, v in enumerate(a)]
        return a
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
    assert 2 <= len(a) <= MAX_N and all(1 <= v <= MAX_A for v in a)
    sys.stdout.write(f"{len(a)}\n{' '.join(map(str, a))}\n")


if __name__ == "__main__":
    main()
