// abc239-h の愚直解。1 以外の目 (2 以上 N 以下) を順に並べた列で、積が M 以下のものを長さ j ごとに
// 全部数えて c_j とする (j = 0 の空の列も 1 個)。t 回振っても積が M 以下である確率は
// Σ_j C(t, j) c_j / N^t なので、期待値 Σ_t P(t 回で止まっていない) は N Σ_j c_j / (N - 1)^(j + 1) になる。
// 期待値の漸化式もディリクレ級数の割り算も使わないので、DirichletSeries とは別の考え方になる。
// 列の数は M^1.73 ほどなので、M が小さいときだけ使う。
#include <cstdio>
#include <vector>

using u64 = unsigned long long;
constexpr u64 P = 1000000007;

u64 n, m;
std::vector<u64> c;

// 積が prod の長さ len の列を伸ばす。
void extend(u64 prod, size_t len) {
  if (c.size() <= len) c.resize(len + 1, 0);
  ++c[len];
  for (u64 x = 2; x <= n && prod * x <= m; ++x) extend(prod * x, len + 1);
}

u64 pow_mod(u64 x, u64 e) {
  u64 r = 1;
  for (x %= P; e; e >>= 1, x = x * x % P)
    if (e & 1) r = r * x % P;
  return r;
}

int main() {
  if (scanf("%llu %llu", &n, &m) != 2) return 1;
  extend(1, 0);
  u64 inv = pow_mod(n - 1, P - 2), scale = inv, ans = 0;  // scale = 1 / (N - 1)^(j + 1)
  for (size_t j = 0; j < c.size(); ++j, scale = scale * inv % P) ans = (ans + c[j] % P * scale) % P;
  printf("%llu\n", ans * (n % P) % P);
}
