// abc256-h の愚直解。配列をそのまま持ち、クエリごとに L から R までを 1 つずつ割るか書き換えるか足す。
// セグメント木も遅延も使わないので、提出とは別の考え方になる。O(NQ) なので、小さい入力でだけ使う。
#include <cstdio>
#include <vector>

int main() {
  int n, q;
  if (scanf("%d %d", &n, &q) != 2) return 1;
  std::vector<long long> a(n + 1);
  for (int i = 1; i <= n; ++i)
    if (scanf("%lld", &a[i]) != 1) return 1;
  while (q--) {
    int type, l, r;
    if (scanf("%d %d %d", &type, &l, &r) != 3) return 1;
    if (type == 3) {
      long long sum = 0;
      for (int i = l; i <= r; ++i) sum += a[i];
      printf("%lld\n", sum);
      continue;
    }
    long long v;
    if (scanf("%lld", &v) != 1) return 1;
    for (int i = l; i <= r; ++i) a[i] = type == 1 ? a[i] / v : v;
  }
}
