// s8pc-3-g の愚直解。定義どおりに d_1 = (a_1, ..., a_m) (フィボナッチ数列) を作り、
// 累積和を n - 1 回取って d_{n,m} を出す。多項式の標本点の移動も線形漸化式の項の計算も使わないので、
// 提出とは別の考え方になる。O(n m) なので、小さい入力でだけ使う。
#include <cstdio>
#include <vector>

int main() {
  const long long P = 998244353;
  long long n, m;
  if (scanf("%lld %lld", &n, &m) != 2) return 1;
  std::vector<long long> d(m + 1, 0);
  for (long long j = 1; j <= m; ++j) d[j] = j <= 2 ? 1 : (d[j - 1] + d[j - 2]) % P;
  for (long long i = 2; i <= n; ++i)
    for (long long j = 1; j <= m; ++j) d[j] = (d[j] + d[j - 1]) % P;
  printf("%lld\n", d[m]);
}
