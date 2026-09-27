"""abc234-g (Divide a Sequence) の入力を作る。N と A_1 ... A_N を出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、N = 10^5 か 3 × 10^5 のいろいろな列、
愚直解でも解ける N = 3000 までのランダム。列は、ランダム、1 から 3 の値 (同じ値だらけ)、全部同じ、
単調増加、単調減少、10^9 と 1 の交互、山形、谷形、998244353 をまたぐ値、2 種類の値、段のある単調増加。
提出は最大と最小のデカルト木で各 A_i が最大 (最小) になる範囲を求めるので、同じ値の扱いで間違えやすい。
998244353 をまたぐ値では、先に法で割ってから大小を比べると間違える。
入力が大きくなりすぎないよう、値の幅はケースで変える。
seed が 1000 以上なら、最後の区間の左端ごとに足す O(N^2) の愚直解で解ける小さい入力を出す
(pj testdata crosscheck 用)。2000 以上なら、愚直解でまだ解ける N = 3000 までの入力を出す。
"""

import random
import sys

MAX_N = 3 * 10**5
MAX_A = 10**9
P = 998244353

SAMPLES = [
    [1, 2, 3],
    [1, 10, 1, 10],
    [699498050, 759726383, 769395239, 707559733, 72435093, 537050110, 880264078, 699299140, 418322627, 134917794],
]
FIXED = [
    [5],
    [MAX_A],
    [1, MAX_A],  # 答えは 10^9 - 1 を法で割ったもの
    [7, 7],
    [1, P + 1],  # 差が P で、答えは 0
    [3, 1, 2],
]
# (N, 列の作り方)。本番のケースのうち角のケースのあとに並べる。
PLANS = [
    (MAX_N, "random"),
    (MAX_N, "ties"),
    (MAX_N, "equal"),
    (MAX_N, "increasing"),
    (MAX_N, "decreasing"),
    (MAX_N, "zigzag"),
    (10**5, "mountain"),
    (10**5, "valley"),
    (MAX_N, "around_p"),
    (10**5, "two"),
    (MAX_N, "steps"),
    (3000, "random"),
    (3000, "ties"),
]
KINDS = ["random", "ties", "equal", "increasing", "decreasing", "zigzag", "mountain", "valley", "around_p", "two", "steps"]


def sequence(rng: random.Random, n: int, kind: str) -> list[int]:
    if kind == "random":
        return [rng.randint(1, MAX_A) for _ in range(n)]
    if kind == "ties":
        return [rng.randint(1, 3) for _ in range(n)]
    if kind == "equal":
        return [rng.choice([1, 5, MAX_A])] * n
    if kind == "increasing":
        return list(range(1, n + 1))
    if kind == "decreasing":
        return sorted((rng.randint(1, MAX_A) for _ in range(n)), reverse=True)
    if kind == "zigzag":
        return [MAX_A if i % 2 == 0 else 1 for i in range(n)]
    if kind in ("mountain", "valley"):
        # 並べた値を 1 つおきに取って、増えてから減る列 (谷形はその逆) にする。
        a = sorted(rng.randint(1, MAX_A) for _ in range(n))
        return a[::2] + a[1::2][::-1] if kind == "mountain" else a[::2][::-1] + a[1::2]
    if kind == "around_p":
        return [rng.randint(P - 500000, P + 500000) for _ in range(n)]
    if kind == "two":
        lo, hi = rng.choice([(1, MAX_A), (1, 2), (P, P + 1)])
        return [rng.choice([lo, hi]) for _ in range(n)]
    assert kind == "steps"
    # 同じ値が続く段を持つ単調増加。段の高さは 1000 まで。
    seq, v = [], 1
    while len(seq) < n:
        seq += [v] * rng.randint(1, 20)
        v = min(1000, v + rng.randint(0, 2))
    return seq[:n]


def case_for(seed: int, rng: random.Random) -> list[int]:
    if seed >= 1000:
        n = rng.randint(100, 3000) if seed >= 2000 else rng.randint(1, 60)
        return sequence(rng, n, rng.choice(KINDS))
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    return sequence(rng, *PLANS[seed - len(FIXED)])


def main() -> None:
    seed = int(sys.argv[1])
    a = case_for(seed, random.Random(seed))
    assert 1 <= len(a) <= MAX_N and all(1 <= v <= MAX_A for v in a)
    sys.stdout.write(f"{len(a)}\n{' '.join(map(str, a))}\n")


if __name__ == "__main__":
    main()
