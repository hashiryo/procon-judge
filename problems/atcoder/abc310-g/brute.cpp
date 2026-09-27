// abc310-g の愚直解。N K が小さければ、x = 1, 2, ..., K 回の操作を問題文のとおり順に行い、各回の玉の数を足す。
// K が大きければ、人 i ごとに玉の行き先 v_0 = i, v_1 = A_i, ... を同じ人が 2 回出るまでたどり、1 <= x <= K の
// うち x 回後に v_j にいる x の個数を数えて B_i を配る。最後に K の逆元を掛ける。Period (関数グラフの HLD) も
// 区間の差分も使わないので、提出とは別の考え方になる。O(NK) か O(N^2) なので、小さい入力でだけ使う。
#include <algorithm>
#include <cstdio>
#include <vector>

using u64 = unsigned long long;
const u64 P = 998244353;

u64 pow_mod(u64 x, u64 e) {
  u64 r = 1;
  for (x %= P; e; e >>= 1, x = x * x % P)
    if (e & 1) r = r * x % P;
  return r;
}

int main() {
  int n;
  u64 k;
  if (scanf("%d %llu", &n, &k) != 2) return 1;
  std::vector<int> a(n);
  std::vector<u64> b(n), total(n, 0);
  for (auto &v : a)
    if (scanf("%d", &v) != 1) return 1;
  for (auto &v : a) --v;
  for (auto &v : b)
    if (scanf("%llu", &v) != 1) return 1;
  if (k <= 10000000ULL / n) {
    std::vector<u64> now(b), next(n);
    for (u64 x = 1; x <= k; ++x) {
      std::fill(next.begin(), next.end(), 0);
      for (int i = 0; i < n; ++i) next[a[i]] = (next[a[i]] + now[i]) % P;
      now.swap(next);
      for (int i = 0; i < n; ++i) total[i] = (total[i] + now[i]) % P;
    }
  } else {
    std::vector<int> first(n, -1), path;
    for (int i = 0; i < n; ++i) {
      path.clear();
      int v = i;
      while (first[v] < 0) first[v] = path.size(), path.push_back(v), v = a[v];
      u64 s = first[v], c = path.size() - s;  // path[s..] が輪、path[..s) が尻尾
      for (u64 j = 0; j < path.size(); ++j) {
        u64 times;  // j, j + c, j + 2c, ... のうち 1 以上 K 以下の個数 (尻尾なら j の 1 つだけ)
        if (j < s) times = (1 <= j && j <= k) ? 1 : 0;
        else if (j == 0) times = k / c;
        else times = j <= k ? (k - j) / c + 1 : 0;
        total[path[j]] = (total[path[j]] + times % P * b[i]) % P;
      }
      for (int u : path) first[u] = -1;
    }
  }
  u64 inv_k = pow_mod(k % P, P - 2);
  for (int i = 0; i < n; ++i) printf("%llu%c", total[i] * inv_k % P, i + 1 == n ? '\n' : ' ');
}
