#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""cf-1139-e (Maximize Mex) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

seed 0 から 2 は問題文の例。seed 3 から 11 は n = 5000 のケースで、力と部の選び方は、力が 1000 未満の乱択、力が 0 から
4999 の並べ替え、部が 100 個で力が 200 未満、力が 50 未満、部が 1 個、部 j に力 j と j + 1 の学生を置く鎖 (増加路が長い)、
力 i mod 2500 と部 (i + i / 2500) mod 2500 を組む形、部が 3000 個で力が 600 未満、力が 400 未満で d = 1。去る順は乱択で、
d は n (seed 11 だけ 1)。seed 1000 以上は、愚直解 (brute.hpp) と突き合わせる小さい入力 (学生 8 人まで)。
"""
import random
import sys

SAMPLES = [
    """5 3
0 1 2 2 0
1 2 2 3 2
5
3
2
4
5
1""",
    """5 3
0 1 2 2 1
1 3 2 3 2
5
4
2
3
5
1""",
    """5 5
0 1 2 4 5
1 2 3 4 5
4
2
3
5
4""",
]


def out(rng: random.Random, m: int, p: list[int], c: list[int], d: int) -> str:
    n = len(p)
    order = list(range(1, n + 1))
    rng.shuffle(order)
    lines = [f"{n} {m}", " ".join(map(str, p)), " ".join(str(x + 1) for x in c), str(d)]
    lines += [str(x) for x in order[:d]]
    return "\n".join(lines)


def main_case(seed: int) -> str:
    if seed < 3:
        return SAMPLES[seed]
    rng = random.Random(seed * 7919 + 19)
    n = 5000
    if seed == 3:
        return out(rng, n, [rng.randrange(1000) for _ in range(n)], [rng.randrange(n) for _ in range(n)], n)
    if seed == 4:
        p = list(range(n))
        rng.shuffle(p)
        return out(rng, n, p, [rng.randrange(n) for _ in range(n)], n)
    if seed == 5:
        return out(rng, 100, [rng.randrange(200) for _ in range(n)], [rng.randrange(100) for _ in range(n)], n)
    if seed == 6:
        return out(rng, n, [rng.randrange(50) for _ in range(n)], [rng.randrange(n) for _ in range(n)], n)
    if seed == 7:
        return out(rng, 1, [rng.randrange(5) for _ in range(n)], [0] * n, n)
    if seed == 8:
        # 部 j に力 j と j + 1 の学生。力 0 から順に足すと、新しい力は鎖の端から増加路を探す。
        p, c = [], []
        for j in range(n // 2):
            p += [j, j + 1]
            c += [j, j]
        return out(rng, n // 2, p, c, n)
    if seed == 9:
        return out(rng, 2500, [i % 2500 for i in range(n)], [(i + i // 2500) % 2500 for i in range(n)], n)
    if seed == 10:
        return out(rng, 3000, [rng.randrange(600) for _ in range(n)], [rng.randrange(3000) for _ in range(n)], n)
    if seed == 11:
        return out(rng, n, [rng.randrange(400) for _ in range(n)], [rng.randrange(n) for _ in range(n)], 1)
    raise SystemExit(f"seed {seed} は 0 から 11 か、1000 以上にしてください")


def small_case(seed: int) -> str:
    rng = random.Random(seed * 104729 + 7)
    n = rng.randint(1, 8)
    m = rng.randint(1, n)
    hi = rng.randint(1, 6)
    return out(rng, m, [rng.randrange(hi) for _ in range(n)], [rng.randrange(m) for _ in range(n)], rng.randint(1, n))


def main() -> None:
    seed = int(sys.argv[1])
    print(small_case(seed) if seed >= 1000 else main_case(seed))


if __name__ == "__main__":
    main()
