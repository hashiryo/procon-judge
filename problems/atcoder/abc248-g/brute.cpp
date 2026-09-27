// abc248-g の愚直解。頂点 s ごとに s から木をたどり、道の頂点の数と gcd を持ち回って、t > s の C(s, t) を足す。
// 約数ごとの配列を持たず、全方位木 DP もしないので、提出とは別の考え方になる。O(N^2 log A) なので、
// 小さい入力でだけ使う。
#include <cstdio>
#include <numeric>
#include <vector>

constexpr long long P = 998244353;

int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  std::vector<int> a(n + 1);
  for (int i = 1; i <= n; ++i)
    if (scanf("%d", &a[i]) != 1) return 1;
  std::vector<std::vector<int>> adj(n + 1);
  for (int i = 0; i < n - 1; ++i) {
    int u, v;
    if (scanf("%d %d", &u, &v) != 2) return 1;
    adj[u].push_back(v), adj[v].push_back(u);
  }
  long long ans = 0;
  for (int s = 1; s <= n; ++s) {
    // (頂点, 来た頂点, s からの道の頂点の数, 道の gcd) を積んでたどる。
    struct Item { int v, from, k, g; };
    std::vector<Item> stack = {{s, 0, 1, a[s]}};
    while (!stack.empty()) {
      auto [v, from, k, g] = stack.back();
      stack.pop_back();
      if (v > s) ans = (ans + (long long)k * g) % P;
      for (int u : adj[v])
        if (u != from) stack.push_back({u, v, k + 1, std::gcd(g, a[u])});
    }
  }
  printf("%lld\n", ans);
}
