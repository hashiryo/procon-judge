#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# ///
"""洛谷 P1971 ([NOI2011] 兔兔与蛋蛋游戏) の入力を作る。seed を 1 つ受け取り、その番号のケースを stdout に出す。

盤を作り、両者に合法な手を指させて、兔兔が動かせなくなったところで止めた対局を出す (問題文の約束どおり蛋蛋が勝ち、
k は 1 以上 1000 以下)。蛋蛋が先に動かせなくなったら、指し方を変えて作り直す。seed 0 から 2 は問題文の例。seed 3 から
11 は、空きマスとの偶奇の差が 0 のマスを黒、1 のマスを白にした盤 (どのマスも頂点になる) と、色を乱択にした盤、その中間
の盤で、大きさは 40 × 40、1 × 20、2 × 40、16 × 16、4 × 5。指し方は、乱択と、動かした先で相手の手が最も少なくなる手を
選ぶもの (対局が長くなる)。seed 1000 以上は、愚直解 (brute.hpp) と突き合わせる小さい入力 (マス 12 個まで)。
"""
import random
import sys

SAMPLES = [
    """1 6
XO.OXO
1
1 2
1 1""",
    """3 3
XOX
O.O
XOX
4
2 3
1 3
1 2
1 1
2 1
3 1
3 2
3 3""",
    """4 4
OOXX
OXXO
OO.O
XXXO
2
3 2
2 2
1 2
1 3""",
]
D = ((1, 0), (-1, 0), (0, 1), (0, -1))


def board(rng: random.Random, n: int, m: int, aligned: float) -> list[list[str]]:
    sx, sy = rng.randrange(n), rng.randrange(m)
    g = [["X" if (i + j + sx + sy) % 2 == 0 else "O" for j in range(m)] for i in range(n)]
    for i in range(n):
        for j in range(m):
            if rng.random() >= aligned:
                g[i][j] = rng.choice("XO")
    g[sx][sy] = "."
    return g


def play(rng: random.Random, g0: list[list[str]], greedy: bool) -> list[tuple[int, int]] | None:
    n, m = len(g0), len(g0[0])
    g = [r[:] for r in g0]
    ex, ey = next((i, j) for i in range(n) for j in range(m) if g[i][j] == ".")
    moves: list[tuple[int, int]] = []
    turn = 0  # 0 は兔兔 (白を動かす)、1 は蛋蛋 (黒を動かす)

    def cand(x: int, y: int, col: str) -> list[tuple[int, int]]:
        return [(x + dx, y + dy) for dx, dy in D if 0 <= x + dx < n and 0 <= y + dy < m and g[x + dx][y + dy] == col]

    while True:
        col = "OX"[turn]
        cs = cand(ex, ey, col)
        if not cs:
            break
        if greedy and rng.random() < 0.9:
            nxt = "OX"[turn ^ 1]
            best = min(len(cand(x, y, nxt)) for x, y in cs)
            cs = [(x, y) for x, y in cs if len(cand(x, y, nxt)) == best]
        x, y = rng.choice(cs)
        g[ex][ey], g[x][y] = col, "."
        ex, ey = x, y
        moves.append((x, y))
        turn ^= 1
    if turn != 0 or not moves or len(moves) // 2 > 1000:
        return None
    return moves


def make(rng: random.Random, n: int, m: int, aligned: float, greedy: bool, tries: int | None = None) -> str | None:
    """盤を作り直しながら、蛋蛋が勝つ対局を探す。tries 枚の盤で見つからなければ None (小さい盤には、どう指しても
    兔兔が勝つものがある)。"""
    while tries is None or tries > 0:
        if tries is not None:
            tries -= 1
        g = board(rng, n, m, aligned)
        for _ in range(50):
            mv = play(rng, g, greedy)
            if mv is not None:
                lines = [f"{n} {m}"] + ["".join(r) for r in g] + [str(len(mv) // 2)]
                lines += [f"{x + 1} {y + 1}" for x, y in mv]
                return "\n".join(lines)
    return None


def main_case(seed: int) -> str:
    if seed < 3:
        return SAMPLES[seed]
    rng = random.Random(seed * 7919 + 23)
    table = {
        3: (40, 40, 1.0, False),
        4: (40, 40, 0.0, False),
        5: (1, 20, 1.0, False),
        6: (2, 40, 1.0, True),
        7: (16, 16, 1.0, True),
        8: (40, 40, 1.0, True),
        9: (40, 40, 0.9, True),
        10: (4, 5, 0.7, False),
        11: (40, 40, 0.97, True),
    }
    if seed not in table:
        raise SystemExit(f"seed {seed} は 0 から 11 か、1000 以上にしてください")
    res = make(rng, *table[seed])
    assert res is not None
    return res


def small_case(seed: int) -> str:
    rng = random.Random(seed * 104729 + 7)
    while True:
        n, m = rng.randint(1, 4), rng.randint(1, 4)
        if 3 <= n * m <= 12:
            s = make(rng, n, m, rng.choice((1.0, 0.8, 0.5, 0.0)), rng.random() < 0.5, 20)
            if s is not None:
                return s


def main() -> None:
    seed = int(sys.argv[1])
    print(small_case(seed) if seed >= 1000 else main_case(seed))


if __name__ == "__main__":
    main()
