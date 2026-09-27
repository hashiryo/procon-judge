// abc179-e の愚直解。to[j][v] = v から 2^j 項進んだ先、sum[j][v] = v から始めた 2^j 項の和、の表を倍々で作り、
// N を 2 進で見て進める。尻尾と輪に分けず、HeavyLightDecomposition で祖先をたどりもしないので、
// Period とは別の考え方になる。O(M log N) なので、実は N = 10^10 でも速い。
#include <cstdio>
#include <vector>

int main() {
  long long n, x, m;
  if (scanf("%lld %lld %lld", &n, &x, &m) != 3) return 1;
  const int LOG = 34;  // 2^34 > 10^10
  std::vector<std::vector<long long>> to(LOG, std::vector<long long>(m)), sum(LOG, std::vector<long long>(m));
  for (long long v = 0; v < m; ++v) to[0][v] = v * v % m, sum[0][v] = v;
  for (int j = 1; j < LOG; ++j)
    for (long long v = 0; v < m; ++v) {
      long long u = to[j - 1][v];
      to[j][v] = to[j - 1][u];
      sum[j][v] = sum[j - 1][v] + sum[j - 1][u];
    }
  long long ans = 0, v = x;
  for (int j = 0; j < LOG; ++j)
    if (n >> j & 1) ans += sum[j][v], v = to[j][v];
  printf("%lld\n", ans);
}
