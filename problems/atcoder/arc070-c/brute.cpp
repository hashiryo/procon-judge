// arc070-c の愚直解。長方形の左端の置き場所 p を、入力の左端の最小値から最大値までの整数で全部試す DP。
// dp[p] は、長方形 1 から i までをつなげ、長方形 i の左端を p に置くときの最小の費用で、前の長方形と重なる置き場所を
// 全部見て最小を取る。どの左端をこの範囲に寄せても隣との重なりは崩れず費用も増えないので、範囲の外は見なくてよい。
// 費用の関数を区分線形凸関数として持たないので、PiecewiseLinearConvex とは別の考え方になる。座標が小さい入力でだけ使う。
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <vector>

int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  std::vector<long long> l(n), w(n);
  for (int i = 0; i < n; ++i) {
    long long r;
    if (scanf("%lld %lld", &l[i], &r) != 2) return 1;
    w[i] = r - l[i];
  }
  long long lo = *std::min_element(l.begin(), l.end()), hi = *std::max_element(l.begin(), l.end());
  int v = hi - lo + 1;
  const long long INF = 1LL << 62;
  std::vector<long long> dp(v);
  for (int p = 0; p < v; ++p) dp[p] = std::llabs(lo + p - l[0]);
  for (int i = 1; i < n; ++i) {
    std::vector<long long> next(v, INF);
    for (int p = 0; p < v; ++p) {
      long long best = INF;
      // 前の区間 [q, q + w[i-1]] と今の区間 [p, p + w[i]] が重なるのは p - w[i-1] <= q <= p + w[i] のとき。
      for (int q = 0; q < v; ++q)
        if (p - w[i - 1] <= q && q <= p + w[i]) best = std::min(best, dp[q]);
      next[p] = best + std::llabs(lo + p - l[i]);
    }
    dp = std::move(next);
  }
  printf("%lld\n", *std::min_element(dp.begin(), dp.end()));
}
