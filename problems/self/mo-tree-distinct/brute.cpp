// 愚直解。各クエリで u から木をたどって v への道を見つけ、道の頂点の色を集めて数を数える。
// gen.py の seed 1000 以上が出す小さい入力 (n, q <= 40) でだけ使う。
#include <algorithm>
#include <cstdio>
#include <vector>
using namespace std;

int main() {
  int n, q;
  if (scanf("%d %d", &n, &q) != 2) return 1;
  vector<int> col(n);
  for (int i = 0; i < n; ++i)
    if (scanf("%d", &col[i]) != 1) return 1;
  vector<vector<int>> adj(n);
  for (int i = 0; i < n - 1; ++i) {
    int u, v;
    if (scanf("%d %d", &u, &v) != 2) return 1;
    --u, --v;
    adj[u].push_back(v), adj[v].push_back(u);
  }
  for (int i = 0; i < q; ++i) {
    int u, v;
    if (scanf("%d %d", &u, &v) != 2) return 1;
    --u, --v;
    // u から幅優先で、各頂点の親を記録する。木なので v への道は 1 本に決まる。
    vector<int> par(n, -1), st{u};
    par[u] = u;
    while (!st.empty()) {
      const int x = st.back();
      st.pop_back();
      for (int y : adj[x])
        if (par[y] < 0) par[y] = x, st.push_back(y);
    }
    vector<int> got;
    for (int x = v;; x = par[x]) {
      got.push_back(col[x]);
      if (x == u) break;
    }
    sort(got.begin(), got.end());
    got.erase(unique(got.begin(), got.end()), got.end());
    printf("%d\n", (int)got.size());
  }
}
