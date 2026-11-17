"""atcoder-abc374-g (Only One Product Name) の入力を作る。N (1 以上 676 以下) と、相異なる英大文字 2 文字の品名 N 個を出す。

seed が 0 から count - 1 までは本番のケース。問題文の例 3 つ、N = 1 (自己ループと普通の品名)、676 個すべて (1 つの強連結成分)、
前半の文字から後半の文字への品名だけ (続けて書ける組が無く、答えは N)、3 段の文字の組で層を作ったもの (反鎖が大きい)、
文字の順に向きをそろえた品名 (縮めた DAG がそのまま品名のグラフで、推移閉包が大きい)、文字の列を 1 本の道にしたもの、
密度を変えたランダム、自己ループの多いもの。seed が 1000 以上なら、品名が 10 個以下の入力を出す (愚直解 brute.cpp 用)。
"""

import random
import string
import sys

L = list(string.ascii_uppercase)
ALL = [a + b for a in L for b in L]
SAMPLES = [
    ["AB", "BC", "CA", "CD", "DE", "DF", "XX"],
    ["AC", "BC", "CD", "DE", "DF"],
    ["AB", "AC", "CB", "AD", "DB", "BA"],
]


def emit(names: list[str]) -> None:
    assert 1 <= len(names) <= 676 and len(set(names)) == len(names)
    print(len(names))
    print("\n".join(names))


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed)
    if seed >= 1000:
        k = rng.randint(1, 4)
        alpha = rng.sample(L, k)
        pool = [a + b for a in alpha for b in alpha]
        n = rng.randint(1, min(10, len(pool)))
        names = rng.sample(pool, n)
        emit(names)
        return
    if seed < 3:
        emit(SAMPLES[seed])
        return
    perm = L[:]
    rng.shuffle(perm)
    if seed == 3:
        names = ["AA"]
    elif seed == 4:
        names = ["QZ"]
    elif seed == 5:
        names = ALL[:]
    elif seed == 6:
        names = [a + b for a in perm[:13] for b in perm[13:]]
    elif seed in (7, 8):
        g1, g2, g3 = perm[:8], perm[8:17], perm[17:]
        pool = [a + b for a in g1 for b in g2] + [a + b for a in g2 for b in g3]
        names = [x for x in pool if rng.random() < (0.9 if seed == 7 else 0.4)]
    elif seed in (9, 10, 11):
        pos = {c: i for i, c in enumerate(perm)}
        p = {9: 0.6, 10: 0.2, 11: 0.05}[seed]
        names = [a + b for a in L for b in L if pos[a] < pos[b] and rng.random() < p]
    elif seed == 12:
        names = [perm[i] + perm[i + 1] for i in range(25)] + [c + c for c in perm if rng.random() < 0.5]
    elif seed in (13, 14, 15, 16):
        p = {13: 0.9, 14: 0.5, 15: 0.1, 16: 0.03}[seed]
        names = [x for x in ALL if rng.random() < p]
    else:
        loops = [c + c for c in L]
        others = rng.sample([x for x in ALL if x[0] != x[1]], 30)
        names = loops + others
    if not names:
        names = [rng.choice(ALL)]
    rng.shuffle(names)
    emit(names)


main()
