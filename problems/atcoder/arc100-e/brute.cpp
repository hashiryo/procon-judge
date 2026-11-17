// arc100-e の愚直解。K ごとに、i < j で (i or j) <= K の組を全部見て A_i + A_j の最大を取る。O(4^N) なので小さい入力でだけ使う。
#include <cstdio>
#include <vector>

int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  const int m = 1 << n;
  std::vector<long long> a(m);
  for (auto &v : a)
    if (scanf("%lld", &v) != 1) return 1;
  for (int k = 1; k < m; ++k) {
    long long best = 0;
    for (int i = 0; i < m; ++i)
      for (int j = i + 1; j < m; ++j)
        if ((i | j) <= k && a[i] + a[j] > best) best = a[i] + a[j];
    printf("%lld\n", best);
  }
}
