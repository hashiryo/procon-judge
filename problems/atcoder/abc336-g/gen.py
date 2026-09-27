"""abc336-g (16 Integers) の入力を作る。X_{0000} から X_{1111} の 16 個を 1 行で出す。

窓 (i, j, k, l) を、3 文字の状態 (i, j, k) から (j, k, l) への辺と見ると、答えは辺の個数が X の
オイラー路の数 (同じ種類の辺は区別しない)。
seed が 0 から count - 1 までは本番のケース。例の 4 つ、角のケース、N が 10^6 に近いケース。
角のケースは、N = 1、1 種類だけ (答えは 1)、辺はあるのに繋がっていない (答えは 0)、入次数と出次数の差が
2 (答えは 0)、0101 と 1010 の交互 (個数が同じなら閉路で答えは 2、1 違いなら路で 1)、16 種類が全部 1 (答えは
16 個の de Bruijn 閉路を 16 通りに切って 256)、0 の並びの中に 1 が 1 つの列の窓 (閉路になり、1 が
両端に分かれた列も数えて、答えは N) など。
N が 10^6 に近いケースは、ランダムな列の窓を数えたもの (偏りのある列、長い連、先頭と末尾の 3 文字が
同じで閉路になる列を含む)、周期的な列、閉路のまま 2 つの個数を足したもの、16 個とも 62500、
ランダムな 16 個 (たいてい 0)。
seed が 1000 以上なら、残りの個数を状態にする愚直解で解ける N = 14 までの入力を出す (pj testdata crosscheck 用)。
2000 以上なら、偏りのある列から作った、残りの個数の組み合わせが少ない N = 200 までの入力を出す。
"""

import random
import sys

MAX_N = 10**6


def windows(bits: list[int]) -> list[int]:
    """0 と 1 の列の、長さ 4 の窓の種類ごとの個数。"""
    x = [0] * 16
    for s in range(len(bits) - 3):
        x[bits[s] * 8 + bits[s + 1] * 4 + bits[s + 2] * 2 + bits[s + 3]] += 1
    return x


def random_bits(rng: random.Random, length: int, p: float) -> list[int]:
    return [1 if rng.random() < p else 0 for _ in range(length)]


def run_bits(rng: random.Random, length: int, mean: float) -> list[int]:
    """長さの平均が mean の連を交互に並べる。"""
    bits, b = [], rng.randint(0, 1)
    while len(bits) < length:
        bits += [b] * (1 + int(rng.expovariate(1 / mean)))
        b ^= 1
    return bits[:length]


def closed_bits(rng: random.Random, length: int) -> list[int]:
    """先頭と末尾の 3 文字が同じ列。窓の辺が閉路になり、入次数と出次数が全部揃う。"""
    bits = random_bits(rng, length - 3, 0.5)
    return bits + bits[:3]


def x_of(**kw: int) -> list[int]:
    x = [0] * 16
    for name, v in kw.items():
        x[int(name[1:], 2)] = v
    return x


SAMPLES = [
    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0],
    [1, 1, 2, 0, 1, 2, 1, 1, 1, 1, 1, 2, 1, 0, 1, 0],
    [21, 3, 3, 0, 3, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0],
    [62, 67, 59, 58, 58, 69, 57, 66, 67, 50, 68, 65, 59, 64, 67, 61],
]
H = MAX_N // 2
FIXED = [
    x_of(b0000=1),
    x_of(b1011=1),
    x_of(b0000=MAX_N),
    x_of(b1111=MAX_N),
    x_of(b0000=H, b1111=H),  # 繋がっていない
    x_of(b0101=H, b1010=H),  # 交互の閉路。答えは 2
    x_of(b0101=H, b1010=H - 1),  # 交互の路。答えは 1
    x_of(b0001=2),  # 000 の出次数が入次数より 2 多い
    [1] * 16,
    x_of(b0000=MAX_N - 1, b0001=1),  # 0 の並びのあと最後に 1。答えは 1
    x_of(b0000=MAX_N - 4, b0001=1, b0010=1, b0100=1, b1000=1),  # 1 が 1 つの列の窓。答えは N
    x_of(b0000=H, b0101=H // 2, b1010=H // 2),  # 閉路が 2 つで繋がっていない
]
BIG = ["uniform", "sparse", "runs", "closed", "periodic", "closed_plus", "all_equal", "random_x"]
COUNT = len(SAMPLES) + len(FIXED) + len(BIG)


def big_case(rng: random.Random, shape: str) -> list[int]:
    length = MAX_N + 3 - rng.randint(0, 10)
    if shape == "uniform":
        return windows(random_bits(rng, length, 0.5))
    if shape == "sparse":
        return windows(random_bits(rng, length, 0.02))
    if shape == "runs":
        return windows(run_bits(rng, length, 6.0))
    if shape == "closed":
        return windows(closed_bits(rng, length))
    if shape == "periodic":
        period = [0, 0, 1, 1, 1, 0, 1]
        return windows([period[i % len(period)] for i in range(length)])
    if shape == "closed_plus":
        # 閉路の列に、010 -> 101 -> 010 の 2 本 (0101 と 1010) を 1 本ずつ足す。閉路のまま答えが変わる。
        x = windows(closed_bits(rng, MAX_N + 1))
        x[0b0101] += 1
        x[0b1010] += 1
        return x
    if shape == "all_equal":
        return [MAX_N // 16] * 16
    assert shape == "random_x"
    return [rng.randint(0, MAX_N // 16) for _ in range(16)]


def product_of_counts(x: list[int]) -> int:
    p = 1
    for v in x:
        p *= v + 1
    return p


def small_case(rng: random.Random) -> list[int]:
    n = rng.randint(1, 14)
    kind = rng.randrange(10)
    if kind < 5:
        return windows(random_bits(rng, n + 3, rng.choice([0.5, 0.2, 0.8])))
    if kind < 7:
        x = windows(closed_bits(rng, max(n + 3, 6)) if kind == 5 else random_bits(rng, n + 3, 0.5))
        if kind == 6:
            # 1 つ動かす。たいてい次数が崩れて 0 になる。
            src = rng.choice([e for e in range(16) if x[e] > 0])
            x[src] -= 1
            x[rng.randrange(16)] += 1
        return x
    if kind < 9:
        while True:
            x = [rng.choice([0, 0, 0, 1, 1, 2]) for _ in range(16)]
            if 1 <= sum(x) <= 14:
                return x
    return x_of(b0000=max(0, n - 4), b0001=1, b0010=1, b0100=1, b1000=1)


def medium_case(rng: random.Random) -> list[int]:
    """偏りのある列から作る。残りの個数の組み合わせ (積) が 3 * 10^5 を超えたら作り直す。"""
    while True:
        n = rng.randint(15, 200)
        bits = random_bits(rng, n + 3, rng.choice([0.03, 0.06, 0.1, 0.94])) if rng.random() < 0.7 \
            else run_bits(rng, n + 3, rng.choice([8.0, 15.0]))
        x = windows(bits)
        if product_of_counts(x) <= 3 * 10**5:
            return x


def case_for(seed: int, rng: random.Random) -> list[int]:
    if seed >= 2000:
        return medium_case(rng)
    if seed >= 1000:
        return small_case(rng)
    if seed < len(SAMPLES):
        return SAMPLES[seed]
    seed -= len(SAMPLES)
    if seed < len(FIXED):
        return FIXED[seed]
    return big_case(rng, BIG[seed - len(FIXED)])


def main() -> None:
    seed = int(sys.argv[1])
    x = case_for(seed, random.Random(seed))
    assert len(x) == 16 and all(v >= 0 for v in x) and 1 <= sum(x) <= MAX_N
    print(*x)


if __name__ == "__main__":
    main()
