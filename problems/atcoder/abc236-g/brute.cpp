// abc236-g の愚直解。辺を 1 本足すたびに、それまでの辺だけの隣接行列 (真偽値) を繰り返し 2 乗で L 乗して、
// 頂点 1 からちょうど L 回で行ける頂点を求め、初めて行けた時刻を頂点ごとに記録する。(min, max) 半環の
// 行列を 1 回だけ累乗することはしないので、提出とは別の考え方になる。O(T N^2 log L) なので、N ≤ 64 の
// 小さい入力でだけ使う。
#include <cstdio>
#include <vector>

using u64 = unsigned long long;
using Mat = std::vector<u64>;  // 行 i のビット j が立っていれば i から j へ行ける

Mat mul(const Mat& a, const Mat& b) {
  int n = a.size();
  Mat c(n, 0);
  for (int i = 0; i < n; ++i)
    for (int k = 0; k < n; ++k)
      if (a[i] >> k & 1) c[i] |= b[k];
  return c;
}

int main() {
  int n, t;
  long long l;
  if (scanf("%d %d %lld", &n, &t, &l) != 3 || n > 64) return 1;
  Mat adj(n, 0);
  std::vector<int> first(n, -1);
  for (int time = 1; time <= t; ++time) {
    int u, v;
    if (scanf("%d %d", &u, &v) != 2) return 1;
    adj[u - 1] |= 1ULL << (v - 1);
    Mat power(n, 0), base = adj;
    for (int i = 0; i < n; ++i) power[i] = 1ULL << i;
    for (long long e = l; e > 0; e >>= 1) {
      if (e & 1) power = mul(power, base);
      if (e > 1) base = mul(base, base);
    }
    for (int i = 0; i < n; ++i)
      if (first[i] < 0 && (power[0] >> i & 1)) first[i] = time;
  }
  for (int i = 0; i < n; ++i) printf("%d%c", first[i], i + 1 == n ? '\n' : ' ');
}
