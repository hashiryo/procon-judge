// agc038-c の愚直解。i < j の組を全部たどって、lcm(A_i, A_j) = A_i / gcd × A_j を 64 ビットで求めて足す。
// 約数や倍数のゼータ変換も、gcd ごとにまとめる数え方もしないので、提出とは別の考え方になる。
// O(N^2 log A) なので、小さい入力でだけ使う。
#include <cstdio>
#include <numeric>
#include <vector>

int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  std::vector<unsigned long long> a(n);
  for (auto& x : a)
    if (scanf("%llu", &x) != 1) return 1;
  const unsigned long long mod = 998244353;
  unsigned long long total = 0;
  for (int i = 0; i < n; ++i)
    for (int j = i + 1; j < n; ++j) total = (total + a[i] / std::gcd(a[i], a[j]) * a[j]) % mod;
  printf("%llu\n", total);
}
