// dp-u の愚直解。集合の分割を、各羽が入る塊の番号の列 (制限された増加列) で全部回し、同じ塊の組の a を足して最大を取る。
// 分割の数はベル数 (N = 8 で 4140) なので、小さい入力でだけ使う。
#include <algorithm>
#include <cstdio>
#include <vector>

int n;
long long a[16][16], best;
int c[16];
void rec(int i, int k) {
  if (i == n) {
    long long s = 0;
    for (int x = 0; x < n; ++x)
      for (int y = x + 1; y < n; ++y)
        if (c[x] == c[y]) s += a[x][y];
    best = std::max(best, s);
    return;
  }
  for (int b = 0; b <= k; ++b) c[i] = b, rec(i + 1, std::max(k, b + 1));
}
int main() {
  if (scanf("%d", &n) != 1) return 1;
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j)
      if (scanf("%lld", &a[i][j]) != 1) return 1;
  best = -(1LL << 62);
  rec(0, 0);
  printf("%lld\n", best);
}
