// abc360-g の愚直解。変える位置 x と新しい値 y を全部試し、そのたびに LIS を O(N^2) の DP で求める。
// LIS は y と他の値の大小だけで決まるので、y は A_j - 1、A_j、A_j + 1 (j は全部) を試せば足りる
// (一番小さい値より小さい、一番大きい値より大きい、2 つの値の間、どれかと等しい、の全部を含む)。
// LIS の各段の候補を調べる方法を使わないので、提出とは別の考え方になる。O(N^4) なので、小さい入力でだけ使う。
#include <algorithm>
#include <cstdio>
#include <vector>

int lis(const std::vector<long long> &a) {
  int n = a.size(), best = 0;
  std::vector<int> dp(n, 1);
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < i; ++j)
      if (a[j] < a[i]) dp[i] = std::max(dp[i], dp[j] + 1);
    best = std::max(best, dp[i]);
  }
  return best;
}

int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  std::vector<long long> a(n);
  for (auto &v : a)
    if (scanf("%lld", &v) != 1) return 1;
  std::vector<long long> ys;
  for (long long v : a) ys.insert(ys.end(), {v - 1, v, v + 1});
  std::sort(ys.begin(), ys.end());
  ys.erase(std::unique(ys.begin(), ys.end()), ys.end());
  int best = 0;
  for (int x = 0; x < n; ++x) {
    auto b = a;
    for (long long y : ys) {
      b[x] = y;
      best = std::max(best, lis(b));
    }
  }
  printf("%d\n", best);
}
