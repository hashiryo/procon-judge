// practice2-j の愚直解。配列をそのまま持ち、種類 2 は L から R まで、種類 3 は X から順に端まで調べる。
// セグメント木も二分探索も使わないので、SegmentTree の max_right / min_left とは別の考え方になる。
// 1 クエリが O(N) なので、N と Q が小さいときだけ使う。
#include <algorithm>
#include <cstdio>
#include <vector>

int main() {
  int n, q;
  if (scanf("%d %d", &n, &q) != 2) return 1;
  std::vector<long long> a(n + 1);  // 1 始まり
  for (int i = 1; i <= n; ++i)
    if (scanf("%lld", &a[i]) != 1) return 1;
  while (q--) {
    int t;
    long long x, y;
    if (scanf("%d %lld %lld", &t, &x, &y) != 3) return 1;
    if (t == 1) {
      a[x] = y;
    } else if (t == 2) {
      long long best = a[x];
      for (long long i = x; i <= y; ++i) best = std::max(best, a[i]);
      printf("%lld\n", best);
    } else {
      long long j = x;
      while (j <= n && a[j] < y) ++j;
      printf("%lld\n", j);  // 見つからなければ N + 1
    }
  }
}
