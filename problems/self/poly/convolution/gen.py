#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""素朴な畳み込みの入力を作る (mod 998244353)。seed ごとに (長さの範囲, ケース数) を決める。"""
import random
import sys

P = 998244353
SIZES = {
    1: ((1, 16), (1, 16), 20000),
    2: ((17, 64), (17, 64), 3000),
    3: ((100, 300), (100, 300), 200),
    4: ((1000, 1000), (1000, 1000), 10),
    5: ((4000, 4000), (4000, 4000), 1),
    6: ((8192, 8192), (8192, 8192), 1),
    7: ((10, 10), (100000, 100000), 10),
    8: ((1, 3), (200000, 200000), 5),
}


def main() -> None:
    seed = int(sys.argv[1])
    rng = random.Random(seed * 1_000_003 + 11)
    cs = []
    if seed == 0:
        cs.append(([1, 2], [3, 4, 5]))
        cs.append(([0], [0]))
        cs.append(([P - 1] * 3, [P - 1] * 2))
    elif seed in SIZES:
        (nl, nh), (ml, mh), t = SIZES[seed]
        for _ in range(t):
            n, m = rng.randint(nl, nh), rng.randint(ml, mh)
            if rng.random() < 0.5:
                n, m = m, n
            cs.append(([rng.randrange(P) for _ in range(n)], [rng.randrange(P) for _ in range(m)]))
    else:  # 値がすべて P - 1 (積と和が最大になる)
        n = m = 3000
        cs.append(([P - 1] * n, [P - 1] * m))
    out = [str(len(cs))]
    for a, b in cs:
        out.append(f"{len(a)} {len(b)}")
        out.append(" ".join(map(str, a)))
        out.append(" ".join(map(str, b)))
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
