// abc253-h の愚直解。辺を 1 本ずつ見て、「ここまでの辺から選んだ森の、連結成分への頂点の分け方」ごとに
// 選び方の数を数える DP。辺の両端が別の成分なら、その辺を足して成分をつなげられる。
// K 本の森の数を F_K とすると、K 回の操作で森になる確率は F_K K! / M^K (同じ辺を 2 回選ぶと閉路になる)。
// 集合冪級数で全域木を数えないので、提出とは別の考え方になる。分け方の数が増えるので、小さい入力でだけ使う。
#include <algorithm>
#include <cstdio>
#include <unordered_map>
#include <vector>

using u64 = unsigned long long;
constexpr u64 P = 998244353;

u64 pow_mod(u64 x, u64 e) {
  u64 r = 1;
  for (x %= P; e; e >>= 1, x = x * x % P)
    if (e & 1) r = r * x % P;
  return r;
}

int n;

// 頂点ごとの成分の番号を 4 bit ずつ詰める。番号は頂点 0 から順に初めて出た順に 0, 1, 2, ... と付け直す。
u64 normalize(const std::vector<int>& comp) {
  int relabel[16], next = 0;
  for (int& r : relabel) r = -1;
  u64 key = 0;
  for (int v = 0; v < n; ++v) {
    if (relabel[comp[v]] < 0) relabel[comp[v]] = next++;
    key |= (u64)relabel[comp[v]] << (4 * v);
  }
  return key;
}

int main() {
  int m;
  if (scanf("%d %d", &n, &m) != 2) return 1;
  std::vector<int> comp(n);
  for (int v = 0; v < n; ++v) comp[v] = v;
  std::unordered_map<u64, u64> ways{{normalize(comp), 1}};
  for (int i = 0; i < m; ++i) {
    int u, v;
    if (scanf("%d %d", &u, &v) != 2) return 1;
    --u, --v;
    auto next = ways;  // この辺を選ばない
    for (auto [key, count] : ways) {
      int cu = key >> (4 * u) & 15, cv = key >> (4 * v) & 15;
      if (cu == cv) continue;  // 選ぶと閉路になる
      for (int w = 0; w < n; ++w) {
        int c = key >> (4 * w) & 15;
        comp[w] = c == cv ? cu : c;
      }
      u64& slot = next[normalize(comp)];
      slot = (slot + count) % P;
    }
    ways = std::move(next);
  }
  // forests[k] = 辺が k 本の森の数 (成分の数が n - k)。
  std::vector<u64> forests(n, 0);
  for (auto [key, count] : ways) {
    int components = 0;
    for (int w = 0; w < n; ++w) components = std::max(components, (int)(key >> (4 * w) & 15) + 1);
    forests[n - components] = (forests[n - components] + count) % P;
  }
  u64 inv_m = pow_mod(m, P - 2), fact = 1, scale = 1;
  for (int k = 1; k < n; ++k) {
    fact = fact * k % P, scale = scale * inv_m % P;
    printf("%llu\n", forests[k] * fact % P * scale % P);
  }
}
