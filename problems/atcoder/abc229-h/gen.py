"""abc229-h (Advance or Eat) の入力を作る。N と N 行の盤面を出す。

seed が 0 から count - 1 までは本番のケース。例の 3 つ、角のケース、N = 8 のランダム、値が 0 の盤面。
値が 0 の盤面は、列とその色を入れ替えた列を組にして並べたもの。色を入れ替えた列は元の列の
符号を反転したゲームなので、和が 0 になり、後手の Snuke が勝つ。
seed が 1000 以上なら、盤面全体を探索する愚直解で解ける小さい盤面を出す (pj testdata crosscheck 用)。
"""

import random
import sys

SWAP = {"W": "B", "B": "W", ".": "."}

FIXED = [
    ["BB.", ".B.", "..."],  # 例 1
    ["..", "WW"],  # 例 2
    ["WWBW", "WWWW", "BWB.", "BBBB"],  # 例 3
    ["."],
    ["W"],
    ["B"],
    ["." * 8] * 8,
    ["W" * 8] * 8,  # Takahashi は動けず、食べる駒も無い
    ["B" * 8] * 8,
    ["WB" * 4 if i % 2 == 0 else "BW" * 4 for i in range(8)],
    ["." * 8] * 7 + ["W" * 8],
    ["." * 8] * 7 + ["B" * 8],
    ["." * 8] * 4 + ["W" * 8] + ["B" * 8] + ["." * 8] * 2,
]
RANDOM_FULL = 16  # N = 8 のランダム
ZERO = 6  # 値が 0 の盤面 (半分は 1 マス変えたもの)
RANDOM_SMALL = 5  # N が 2 から 7 のランダム
COUNT = len(FIXED) + RANDOM_FULL + ZERO + RANDOM_SMALL


def random_board(rng: random.Random, n: int, pieces: int | None = None) -> list[list[str]]:
    density = rng.choice([0.2, 0.4, 0.6, 0.8, 1.0])
    white = rng.random()
    cells = [(i, j) for i in range(n) for j in range(n)]
    chosen = set(rng.sample(cells, pieces)) if pieces is not None else {
        c for c in cells if rng.random() < density
    }
    return [
        ["." if (i, j) not in chosen else ("W" if rng.random() < white else "B") for j in range(n)]
        for i in range(n)
    ]


def zero_board(rng: random.Random, n: int) -> list[list[str]]:
    """列と、色を入れ替えた列を組にして並べる。n は偶数。"""
    base = random_board(rng, n)
    columns = []
    for j in range(n // 2):
        column = [base[i][j] for i in range(n)]
        columns += [column, [SWAP[c] for c in column]]
    rng.shuffle(columns)
    return [[columns[j][i] for j in range(n)] for i in range(n)]


def board_for(seed: int, rng: random.Random) -> list[list[str]]:
    if seed >= 1000:
        # 愚直解が盤面全体を探索できる大きさ。N = 4 は駒を 6 個までにする。
        n = rng.randint(1, 4)
        if n % 2 == 0 and rng.random() < 0.3:
            return zero_board(rng, n)
        return random_board(rng, n, pieces=rng.randint(0, 6) if n == 4 else None)
    if seed < len(FIXED):
        return [list(row) for row in FIXED[seed]]
    seed -= len(FIXED)
    if seed < RANDOM_FULL:
        return random_board(rng, 8)
    seed -= RANDOM_FULL
    if seed < ZERO:
        board = zero_board(rng, 8)
        if seed % 2 == 1:
            i, j = rng.randrange(8), rng.randrange(8)
            board[i][j] = rng.choice("WB.")
        return board
    return random_board(rng, rng.randint(2, 7))


def main() -> None:
    seed = int(sys.argv[1])
    board = board_for(seed, random.Random(seed))
    n = len(board)
    assert 1 <= n <= 8 and all(len(row) == n and set(row) <= set("WB.") for row in board)
    print(n)
    for row in board:
        print("".join(row))


if __name__ == "__main__":
    main()
