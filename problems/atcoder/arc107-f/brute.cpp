// arc107-f の愚直解。残す頂点の選び方 2^N 通りを全部試し、残ったグラフの連結成分を DFS で求めて
// (成分ごとの B の和の絶対値の和) - (消した頂点の A の和) の最大を取る。最小カットに直さず定義の
// とおりに数えるので、monge_mincut とは別の考え方になる。N が小さいときだけ使う。
#include <cstdio>
#include <cstdlib>
#include <vector>

int main() {
  int n, m;
  if (scanf("%d %d", &n, &m) != 2) return 1;
  std::vector<long long> a(n), b(n);
  for (auto& x : a)
    if (scanf("%lld", &x) != 1) return 1;
  for (auto& x : b)
    if (scanf("%lld", &x) != 1) return 1;
  std::vector<std::vector<int>> adj(n);
  for (int i = 0; i < m; ++i) {
    int u, v;
    if (scanf("%d %d", &u, &v) != 2) return 1;
    --u, --v;
    adj[u].push_back(v), adj[v].push_back(u);
  }
  long long best = 0;
  bool first = true;
  for (int kept = 0; kept < (1 << n); ++kept) {  // kept の i ビット目が 1 なら頂点 i を残す
    long long profit = 0;
    std::vector<bool> seen(n, false);
    for (int s = 0; s < n; ++s) {
      if (!((kept >> s) & 1)) {
        profit -= a[s];
        continue;
      }
      if (seen[s]) continue;
      long long sum = 0;
      std::vector<int> stack = {s};
      seen[s] = true;
      while (!stack.empty()) {
        int u = stack.back();
        stack.pop_back();
        sum += b[u];
        for (int v : adj[u])
          if (((kept >> v) & 1) && !seen[v]) seen[v] = true, stack.push_back(v);
      }
      profit += std::llabs(sum);
    }
    if (first || profit > best) best = profit, first = false;
  }
  printf("%lld\n", best);
}
