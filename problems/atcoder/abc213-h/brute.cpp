// abc213-h の愚直解。f[v][t] = 地点 1 から出て、道の長さの和が t で地点 v にいる道順の数、を t の小さい順に
// f[b][t] += Σ_(d=1..t) f[a][t - d] p_(i,d) (道 i = (a, b) の両向き) で求め、f[1][T] を出す。
// 形式的冪級数も畳み込みの分け方も使わないので、提出 (RelaxedConvolution) とは別の考え方になる。
// O(M T^2) で、T = 4 × 10^4、M = 10 でも数十秒で終わる。積は 16 個ずつ 64 ビットで足してから割る。
#include <cstdio>
#include <vector>

using u64 = unsigned long long;
const u64 P = 998244353;

// Σ_(d=0..len-1) x[d] y[d] mod P。x と y は P 未満なので、積 16 個の和は 2^64 に収まる。
u64 dot(const unsigned *x, const unsigned *y, int len) {
  u64 total = 0;
  int d = 0;
  for (; d + 16 <= len; d += 16) {
    u64 s = 0;
    for (int e = 0; e < 16; ++e) s += (u64)x[d + e] * y[d + e];
    total = (total + s % P) % P;
  }
  for (; d < len; ++d) total = (total + (u64)x[d] * y[d]) % P;
  return total;
}

int main() {
  int n, m, t;
  if (scanf("%d %d %d", &n, &m, &t) != 3) return 1;
  std::vector<int> a(m), b(m);
  std::vector<std::vector<unsigned>> p(m, std::vector<unsigned>(t + 1, 0));  // p[i][d]
  for (int i = 0; i < m; ++i) {
    if (scanf("%d %d", &a[i], &b[i]) != 2) return 1;
    --a[i], --b[i];
    for (int d = 1; d <= t; ++d)
      if (scanf("%u", &p[i][d]) != 1) return 1;
  }
  // 内積が添字の増える向きに並ぶよう、f は逆順に持つ。rev[v][T - s] = f[v][s]。
  std::vector<std::vector<unsigned>> rev(n, std::vector<unsigned>(t + 1, 0));
  rev[0][t] = 1;  // f[0][0] = 1
  for (int s = 1; s <= t; ++s) {
    // f[v][s - d] = rev[v][t - s + d] なので、d = 1..s の和は rev[v] の t - s + 1 から s 個と p[i] の 1 から s 個の内積。
    std::vector<u64> add(n, 0);
    for (int i = 0; i < m; ++i) {
      add[b[i]] += dot(&p[i][1], &rev[a[i]][t - s + 1], s);
      add[a[i]] += dot(&p[i][1], &rev[b[i]][t - s + 1], s);
    }
    for (int v = 0; v < n; ++v) rev[v][t - s] = add[v] % P;
  }
  printf("%u\n", rev[0][0]);
}
