// 愚直解。人の部分集合を全部試し、大学と分野がどれも違うものの強さの和の最大を、人数ごとに取る。N <= 20 ほどまで。
#include <cstdio>
#include <vector>
using namespace std;
using i64 = long long;
int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  vector<int> a(n), b(n);
  vector<i64> c(n);
  for (int i = 0; i < n; ++i)
    if (scanf("%d %d %lld", &a[i], &b[i], &c[i]) != 3) return 1;
  vector<i64> best(n + 1, -1);
  for (int mask = 1; mask < (1 << n); ++mask) {
    bool ok = true;
    i64 s = 0;
    int k = 0;
    for (int i = 0; i < n && ok; ++i)
      if (mask >> i & 1) {
        for (int j = 0; j < i; ++j)
          if ((mask >> j & 1) && (a[i] == a[j] || b[i] == b[j])) ok = false;
        s += c[i], ++k;
      }
    if (ok && s > best[k]) best[k] = s;
  }
  int k = 0;
  while (k < n && best[k + 1] >= 0) ++k;
  printf("%d\n", k);
  for (int i = 1; i <= k; ++i) printf("%lld\n", best[i]);
}
