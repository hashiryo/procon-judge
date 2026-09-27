// abc229-h の愚直解。盤面全体の状態 (白の位置、黒の位置、手番) を探索して勝ち負けを決める。
// 列ごとのゲームの和に分けないので、PartisanGame とは別の考え方になる。小さい盤面でだけ使う。
#include <cstdio>
#include <map>
#include <tuple>

using u64 = unsigned long long;

int n;
std::map<std::tuple<u64, u64, int>, bool> memo;

// 手番の人が勝てるか。turn = 0 は Takahashi (白を動かし、黒を食べる)、1 は Snuke。
bool wins(u64 white, u64 black, int turn) {
  auto key = std::make_tuple(white, black, turn);
  if (auto it = memo.find(key); it != memo.end()) return it->second;
  u64 mine = turn == 0 ? white : black, theirs = turn == 0 ? black : white;
  bool result = false;
  for (int c = 0; c < n * n && !result; ++c) {
    u64 bit = 1ULL << c;
    if (theirs & bit) {  // 相手の駒を食べる
      u64 next = theirs & ~bit;
      result = turn == 0 ? !wins(mine, next, 1) : !wins(next, mine, 0);
    } else if ((mine & bit) && c >= n) {  // 自分の駒を 1 マス上の空きへ
      u64 up = 1ULL << (c - n);
      if (!((white | black) & up)) {
        u64 next = (mine & ~bit) | up;
        result = turn == 0 ? !wins(next, theirs, 1) : !wins(theirs, next, 0);
      }
    }
  }
  return memo[key] = result;
}

int main() {
  if (scanf("%d", &n) != 1) return 1;
  u64 white = 0, black = 0;
  for (int i = 0; i < n; ++i) {
    char row[16];
    if (scanf("%15s", row) != 1) return 1;
    for (int j = 0; j < n; ++j) {
      if (row[j] == 'W') white |= 1ULL << (i * n + j);
      if (row[j] == 'B') black |= 1ULL << (i * n + j);
    }
  }
  puts(wins(white, black, 0) ? "Takahashi" : "Snuke");
}
