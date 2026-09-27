// abc234-g の愚直解。ways[i] を「先頭 i 個の分け方の値の和」とし、最後の区間の左端 j を i - 1 から 0 へ
// 動かしながら区間の最大と最小を持ち直して、ways[j] × (最大 - 最小) を足す。デカルト木も
// 範囲に足す BIT も使わないので、提出とは別の考え方になる。O(N^2) なので、小さい入力でだけ使う。
#include <algorithm>
#include <cstdio>
#include <vector>

int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  std::vector<long long> a(n);
  for (auto &v : a)
    if (scanf("%lld", &v) != 1) return 1;
  const long long mod = 998244353;
  std::vector<long long> ways(n + 1, 0);
  ways[0] = 1;
  for (int i = 1; i <= n; ++i) {
    long long hi = a[i - 1], lo = a[i - 1];
    for (int j = i - 1; j >= 0; --j) {  // 最後の区間は a[j], ..., a[i - 1]
      hi = std::max(hi, a[j]), lo = std::min(lo, a[j]);
      ways[i] = (ways[i] + ways[j] * ((hi - lo) % mod)) % mod;
    }
  }
  printf("%lld\n", ways[n]);
}
