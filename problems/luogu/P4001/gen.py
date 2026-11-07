#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""洛谷 P4001 (狼抓兔子) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

3 ≤ N, M ≤ 1000、道の重みは 1 以上 10^6 以下。頂点 (i, j) (0 始まり) と、横 (i, j)-(i, j + 1)、縦 (i, j)-(i + 1, j)、
斜め (i, j)-(i + 1, j + 1) の無向の道を、この順の 3 つの部分で並べる。seed 0 から 9 は本番のケースで、族と大きさは CASES の
表のとおり。seed 1000 以上は愚直解 (brute.hpp) と突き合わせる N, M ≤ 4 の入力で、族を seed で回す。
重みは Mersenne Twister (random.Random) の randbytes から作る。整数の seed からの列は Python の版によらない。

族:
  random W      重みを [1, W] から一様に選ぶ。左上と右下の角の 3 本ずつが細いので、最小カットはたいてい角にある
  equal W       重みを全部 W にする
  band W B      i + j が K - 1 以下の頂点と K 以上の頂点 (K = (N + M - 1) / 2) を結ぶ道を [1, B]、ほかを [W / 2, W] にする。
                最小カットは真ん中の斜めの帯にあり、流れは格子の全体を通る
  snake W       横の道と、行の端で次の行へ下りる縦の道 (偶数の行は右端、奇数の行は左端) を W、ほかを 1 にする。
                最小カットは左上の角 (W + 2) で、流れの大半は蛇行する道を全部たどる
"""
import random
import sys
from array import array

# seed: (族, N, M, 引数)
CASES = {
    0: ("random", 1000, 1000, (1000000,)),
    1: ("random", 1000, 1000, (10,)),
    2: ("band", 1000, 1000, (1000000, 100)),
    3: ("band", 1000, 1000, (1000000, 1)),
    4: ("band", 1000, 300, (1000000, 100)),
    5: ("equal", 1000, 1000, (1000000,)),
    6: ("equal", 1000, 1000, (1,)),
    7: ("random", 3, 1000, (1000000,)),
    8: ("random", 1000, 3, (1000000,)),
    9: ("snake", 1000, 1000, (1000000,)),
}


def uniform(rnd: random.Random, k: int, lo: int, hi: int) -> list[int]:
    """[lo, hi] の一様な整数を k 個。32 bit の乱数に幅を掛けて上位を取る。"""
    if k == 0:
        return []
    a = array("I")
    assert a.itemsize == 4
    a.frombytes(rnd.randbytes(4 * k))
    if sys.byteorder != "little":
        a.byteswap()
    w = hi - lo + 1
    return [lo + (x * w >> 32) for x in a]


def make(rnd: random.Random, fam: str, n: int, m: int, args: tuple[int, ...]) -> tuple[list[int], list[int], list[int]]:
    """横 (n 行 m - 1 列)、縦 (n - 1 行 m 列)、斜め (n - 1 行 m - 1 列) の重みを行優先で返す。"""
    nh, nv, nd = n * (m - 1), (n - 1) * m, (n - 1) * (m - 1)
    if fam == "random":
        (w,) = args
        return uniform(rnd, nh, 1, w), uniform(rnd, nv, 1, w), uniform(rnd, nd, 1, w)
    if fam == "equal":
        (w,) = args
        return [w] * nh, [w] * nv, [w] * nd
    if fam == "band":
        w, b = args
        k = (n + m - 1) // 2
        lo = max(1, w // 2)
        h, v, d = uniform(rnd, nh, lo, w), uniform(rnd, nv, lo, w), uniform(rnd, nd, lo, w)
        light = iter(uniform(rnd, 4 * (n + m), 1, b))
        # 横と縦は i + j = K - 1 から K へ、斜めは i + j = K - 2 か K - 1 から 2 つ先へ渡る道が帯になる
        for i in range(n):
            j = k - 1 - i
            if 0 <= j < m - 1:
                h[i * (m - 1) + j] = next(light)
        for i in range(n - 1):
            j = k - 1 - i
            if 0 <= j < m:
                v[i * m + j] = next(light)
            for j in (k - 2 - i, k - 1 - i):
                if 0 <= j < m - 1:
                    d[i * (m - 1) + j] = next(light)
        return h, v, d
    if fam == "snake":
        (w,) = args
        h = [w] * nh
        v = [1] * nv
        for i in range(n - 1):
            v[i * m + (m - 1 if i % 2 == 0 else 0)] = w
        return h, v, [1] * nd
    raise SystemExit(f"unknown family {fam}")


def emit(n: int, m: int, h: list[int], v: list[int], d: list[int]) -> None:
    out = [f"{n} {m}"]
    for rows, cols, a in ((n, m - 1, h), (n - 1, m, v), (n - 1, m - 1, d)):
        for i in range(rows):
            out.append(" ".join(map(str, a[i * cols:(i + 1) * cols])))
    sys.stdout.write("\n".join(out) + "\n")


def main() -> None:
    seed = int(sys.argv[1])
    rnd = random.Random(seed * 1000003 + 7)
    if seed in CASES:
        fam, n, m, args = CASES[seed]
    elif seed >= 1000:
        fams = ["random", "band", "equal", "snake"]
        fam = fams[seed % len(fams)]
        n, m = rnd.randint(3, 4), rnd.randint(3, 4)
        w = rnd.randint(1, 6)
        args = (w, rnd.randint(1, w)) if fam == "band" else (w,)
    else:
        raise SystemExit(f"seed {seed} は 0 から {len(CASES) - 1} か、1000 以上にしてください")
    emit(n, m, *make(rnd, fam, n, m, args))


if __name__ == "__main__":
    main()
