// abc167-d の愚直解。jump[j][v] = v から 2^j 回テレポートした先、の表を倍々で作り、K を 2 進で見て進める。
// 輪と道に分けず、HeavyLightDecomposition で祖先をたどりもしないので、Period とは別の考え方になる。
// O(N log K) なので、実は N = 2 × 10^5 でも速い。
#include <cstdio>
#include <vector>

int main() {
  int n;
  long long k;
  if (scanf("%d %lld", &n, &k) != 2) return 1;
  std::vector<std::vector<int>> jump(60, std::vector<int>(n));
  for (int v = 0; v < n; ++v) {
    if (scanf("%d", &jump[0][v]) != 1) return 1;
    --jump[0][v];
  }
  for (int j = 1; j < 60; ++j)
    for (int v = 0; v < n; ++v) jump[j][v] = jump[j - 1][jump[j - 1][v]];
  int v = 0;
  for (int j = 0; j < 60; ++j)
    if (k >> j & 1) v = jump[j][v];
  printf("%d\n", v + 1);
}
