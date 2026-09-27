// arc116-c の愚直解。cnt[v] を「長さ k で最後が v の列の数」とし、k を 1 つずつ伸ばす DP。
// 次の値は v の倍数なので、v ごとにその倍数へ足す。Dirichlet 級数の累乗を使わないので、
// 提出とは別の考え方になる。O(N M log M) なので、小さい入力でだけ使う。
#include <algorithm>
#include <cstdio>
#include <vector>

int main() {
  long long n, m;
  if (scanf("%lld %lld", &n, &m) != 2) return 1;
  const long long P = 998244353;
  std::vector<long long> cnt(m + 1, 1), next(m + 1);
  cnt[0] = 0;
  for (long long k = 1; k < n; ++k) {
    std::fill(next.begin(), next.end(), 0);
    for (long long v = 1; v <= m; ++v)
      for (long long w = v; w <= m; w += v) next[w] = (next[w] + cnt[v]) % P;
    cnt.swap(next);
  }
  long long ans = 0;
  for (long long v = 1; v <= m; ++v) ans = (ans + cnt[v]) % P;
  printf("%lld\n", ans);
}
