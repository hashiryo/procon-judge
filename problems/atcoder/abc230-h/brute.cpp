// abc230-h の愚直解。袋の中身は「金塊と、空でない袋」の多重集合なので、重さ k の品物が a_k 種類あると、
// 中身の数え上げは prod_k (1 - x^k)^(-a_k) になる。a_k = (重さ k の金塊の数) + (重さ k の袋の数 f_k) で、
// f_k は重さ k - 1 の空でない中身の数。k の小さい順に、f_k を読み、(1 - x^k)^(-a_k) = sum_j C(a_k + j - 1, j) x^(kj)
// を直接掛ける。MSET を exp と relaxed な掛け算で解かないので、提出とは別の考え方になる。O(W^2 log W)。
#include <cstdio>
#include <vector>

using u64 = unsigned long long;
constexpr u64 P = 998244353;

u64 pow_mod(u64 x, u64 e) {
  u64 r = 1;
  for (x %= P; e; e >>= 1, x = x * x % P)
    if (e & 1) r = r * x % P;
  return r;
}

int main() {
  int w, k;
  if (scanf("%d %d", &w, &k) != 2) return 1;
  std::vector<u64> gold(w + 1, 0);
  for (int i = 0; i < k; ++i) {
    int x;
    if (scanf("%d", &x) != 1) return 1;
    gold[x] = 1;
  }
  std::vector<u64> inv(w + 1, 1);
  for (int i = 1; i <= w; ++i) inv[i] = pow_mod(i, P - 2);
  // content[n] = 重さ n の中身 (多重集合) の数。重さ k 未満の品物だけ掛けてあれば、n < k の値は確定している。
  std::vector<u64> content(w, 0), bag(w + 1, 0);
  content[0] = 1;
  for (int wt = 1; wt <= w; ++wt) {
    bag[wt] = (content[wt - 1] + P - (wt == 1)) % P;  // 空の袋は数えない
    if (wt == w) break;
    u64 a = (gold[wt] + bag[wt]) % P;
    if (a == 0) continue;
    // binom[j] = C(a + j - 1, j)
    std::vector<u64> binom(w / wt + 1);
    binom[0] = 1;
    for (int j = 1; j <= w / wt; ++j) binom[j] = binom[j - 1] * ((a + j - 1) % P) % P * inv[j] % P;
    std::vector<u64> next(w, 0);
    for (int n = 0; n < w; ++n) {
      u64 s = 0;
      for (int j = 0; j * wt <= n; ++j) s = (s + binom[j] * content[n - j * wt]) % P;
      next[n] = s;
    }
    content = std::move(next);
  }
  for (int i = 2; i <= w; ++i) printf("%llu\n", bag[i]);
}
