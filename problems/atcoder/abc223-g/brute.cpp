// abc223-g の愚直解。頂点を 1 つずつ本当に消し、残った森の最大マッチングを毎回数え直して、元の木と比べる。
// 最大マッチングは、根から辿った順の逆 (葉の側から) に見て、自分も親もまだ組んでいなければ組ませる貪欲で数える。
// 二部グラフの分解 (Dulmage-Mendelsohn) も全方位木 DP も使わないので、提出とは別の考え方になる。
// O(N^2) なので小さい入力でだけ使う。
#include <cstdio>
#include <vector>

int n;
std::vector<std::vector<int>> adj;

// removed を消した森の最大マッチングの大きさ。removed = -1 なら何も消さない。
int matching(int removed) {
  std::vector<int> parent(n, -1), order;
  std::vector<bool> seen(n, false), matched(n, false);
  for (int root = 0; root < n; ++root) {
    if (root == removed || seen[root]) continue;
    seen[root] = true;
    std::vector<int> stack = {root};
    while (!stack.empty()) {
      int v = stack.back();
      stack.pop_back();
      order.push_back(v);
      for (int u : adj[v])
        if (u != removed && !seen[u]) seen[u] = true, parent[u] = v, stack.push_back(u);
    }
  }
  int size = 0;
  for (int i = (int)order.size() - 1; i >= 0; --i) {
    int v = order[i], p = parent[v];
    if (p != -1 && !matched[v] && !matched[p]) matched[v] = matched[p] = true, ++size;
  }
  return size;
}

int main() {
  if (scanf("%d", &n) != 1) return 1;
  adj.assign(n, {});
  for (int i = 0; i < n - 1; ++i) {
    int u, v;
    if (scanf("%d %d", &u, &v) != 2) return 1;
    --u, --v;
    adj[u].push_back(v), adj[v].push_back(u);
  }
  int whole = matching(-1), ans = 0;
  for (int i = 0; i < n; ++i) ans += matching(i) == whole;
  printf("%d\n", ans);
}
