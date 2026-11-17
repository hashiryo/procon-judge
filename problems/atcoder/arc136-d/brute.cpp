// arc136-d の愚直解。組 (i, j) を全部見て、桁ごとの和が 9 以下かを確かめる。O(N^2) なので小さい入力でだけ使う。
#include <cstdio>
#include <vector>

int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  std::vector<int> a(n);
  for (auto &v : a)
    if (scanf("%d", &v) != 1) return 1;
  long long ans = 0;
  for (int i = 0; i < n; ++i)
    for (int j = i + 1; j < n; ++j) {
      bool ok = true;
      for (int x = a[i], y = a[j]; x || y; x /= 10, y /= 10) ok &= x % 10 + y % 10 <= 9;
      ans += ok;
    }
  printf("%lld\n", ans);
}
