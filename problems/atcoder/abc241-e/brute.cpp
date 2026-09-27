// abc241-e の愚直解。問題文のとおり、X に A_(X mod N) を足す操作を K 回そのまま行う。
// 移り方の輪を探して周期で割らないので、Period (関数グラフの HLD) とは別の考え方になる。O(K) なので、K が小さいときだけ使う。
#include <cstdio>
#include <vector>

int main() {
  long long n, k;
  if (scanf("%lld %lld", &n, &k) != 2) return 1;
  std::vector<long long> a(n);
  for (auto& v : a)
    if (scanf("%lld", &v) != 1) return 1;
  long long x = 0;
  for (long long i = 0; i < k; ++i) x += a[x % n];
  printf("%lld\n", x);
}
