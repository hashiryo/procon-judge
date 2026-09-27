// arc115-e の愚直解。値 1 .. max A を、A の値で区切った区間 (c_(j-1), c_j] にまとめる。同じ区間の値はどの位置でも
// 同じように選べるので、f[j] = (ここまでの列で、最後の値が区間 j のある 1 つの値であるものの数) は区間の中で同じになる。
// 位置 i では、c_j <= A_i の区間について f[j] = (i - 1 までの列の数) - f[j]、ほかは 0 にして前から順に求める。
// 包除原理も CartesianTree も使わないので、提出とは別の考え方になる。O(N × 値の種類) なので、N が小さいか、
// 値の種類が少ないときだけ使う。
#include <algorithm>
#include <cstdio>
#include <vector>

int main() {
  const long long P = 998244353;
  int n;
  if (scanf("%d", &n) != 1) return 1;
  std::vector<long long> a(n);
  for (auto &v : a)
    if (scanf("%lld", &v) != 1) return 1;
  std::vector<long long> c(a);  // 区間の右端 c_0 < c_1 < ...
  std::sort(c.begin(), c.end());
  c.erase(std::unique(c.begin(), c.end()), c.end());
  const int k = c.size();
  std::vector<long long> width(k), f(k, 0);
  for (int j = 0; j < k; ++j) width[j] = c[j] - (j ? c[j - 1] : 0);
  long long total = 0;  // ここまでの列の数
  for (int j = 0; j < k; ++j)
    if (c[j] <= a[0]) f[j] = 1, total = (total + width[j]) % P;
  for (int i = 1; i < n; ++i) {
    long long next_total = 0;
    for (int j = 0; j < k; ++j)
      if (c[j] <= a[i]) {
        f[j] = (total - f[j] + P) % P;
        next_total = (next_total + f[j] * (width[j] % P)) % P;
      } else {
        f[j] = 0;
      }
    total = next_total;
  }
  printf("%lld\n", total);
}
