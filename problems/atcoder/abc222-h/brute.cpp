// abc222-h の愚直解。根が 1 の部分木と、根が 0 の部分木の数を、中の 1 の数ごとに数える DP。
// 操作は N - 1 回までなので、根以外の 1 はちょうど 1 回だけ、親か親の親にある 1 へ動かすことになる。
// なので条件は、根が 1、葉が 1、0 の頂点の子はどれも 1、と言い換えられる (N <= 4 は操作を全部試して確かめた)。
// a[k]: 根が 1 で 1 が k 個の部分木。左右の子はそれぞれ、無い、根が 1、根が 0 のどれか (e = 空 + a + b)。
// b[k]: 根が 0 で 1 が k 個の部分木。左右の子はそれぞれ、無い、根が 1 のどれかで、両方無いのは除く (f = 空 + a)。
// ラグランジュの反転公式で 1 つの係数に直さず、疎な多項式の累乗も使わないので、提出とは別の考え方になる。O(N^2)。
#include <cstdio>
#include <vector>

using u64 = unsigned long long;
constexpr u64 MOD = 998244353;

int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  std::vector<u64> a(n + 1), e(n + 1), f(n + 1);
  e[0] = f[0] = 1;
  for (int k = 1; k <= n; ++k) {
    for (int i = 0; i < k; ++i) a[k] = (a[k] + e[i] * e[k - 1 - i]) % MOD;
    f[k] = a[k];
    u64 b = 0;
    for (int i = 0; i <= k; ++i) b = (b + f[i] * f[k - i]) % MOD;
    e[k] = (a[k] + b) % MOD;
  }
  printf("%llu\n", a[n]);
}
