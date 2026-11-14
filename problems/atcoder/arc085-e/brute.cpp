// 愚直解。割る宝石の集合を全部試し、倍数について閉じているもの (i を割るなら i の倍数も割る) のうち、残る宝石の a の和の
// 最大を取る。N <= 20 ほどまで。
#include <cstdio>
#include <vector>
using namespace std;
int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  vector<long long> a(n + 1);
  for (int i = 1; i <= n; ++i)
    if (scanf("%lld", &a[i]) != 1) return 1;
  long long best = 0;
  for (int mask = 0; mask < (1 << n); ++mask) {
    bool ok = true;
    for (int i = 1; i <= n && ok; ++i)
      if (mask >> (i - 1) & 1)
        for (int j = 2 * i; j <= n; j += i)
          if (!(mask >> (j - 1) & 1)) ok = false;
    if (!ok) continue;
    long long s = 0;
    for (int i = 1; i <= n; ++i)
      if (!(mask >> (i - 1) & 1)) s += a[i];
    if (s > best) best = s;
  }
  printf("%lld\n", best);
}
