// abc228-f の愚直解。黒いはんこの置き方と白いはんこの置き方の組を全部試す。白は盤面のどこに置いてもよく、
// 黒と重なった部分だけを引く (重なりは 2 次元の累積和で数える)。白を黒の中に切り詰めず、置き方ごとの
// 最大を 2 次元のセグメント木で取りもしないので、提出とは別の考え方になる。
// O((HW)^2) なので、小さい盤面でだけ使う。
#include <algorithm>
#include <cstdio>
#include <vector>

int main() {
  int h, w, h1, w1, h2, w2;
  if (scanf("%d %d %d %d %d %d", &h, &w, &h1, &w1, &h2, &w2) != 6) return 1;
  // s[i][j] = 上から i 行、左から j 列の和。
  std::vector<std::vector<long long>> s(h + 1, std::vector<long long>(w + 1, 0));
  for (int i = 0; i < h; ++i)
    for (int j = 0; j < w; ++j) {
      long long a;
      if (scanf("%lld", &a) != 1) return 1;
      s[i + 1][j + 1] = s[i][j + 1] + s[i + 1][j] - s[i][j] + a;
    }
  // 行 [r0, r1)、列 [c0, c1) の和。空なら 0。
  auto sum = [&](int r0, int r1, int c0, int c1) -> long long {
    if (r0 >= r1 || c0 >= c1) return 0;
    return s[r1][c1] - s[r0][c1] - s[r1][c0] + s[r0][c0];
  };
  long long best = -1;
  for (int i = 0; i + h1 <= h; ++i)
    for (int j = 0; j + w1 <= w; ++j) {
      long long black = sum(i, i + h1, j, j + w1), score = black;
      for (int k = 0; k + h2 <= h; ++k)
        for (int l = 0; l + w2 <= w; ++l) {
          // 白と黒の重なりの長方形。
          int r0 = std::max(i, k), r1 = std::min(i + h1, k + h2);
          int c0 = std::max(j, l), c1 = std::min(j + w1, l + w2);
          score = std::min(score, black - sum(r0, r1, c0, c1));
        }
      best = std::max(best, score);
    }
  printf("%lld\n", best);
}
