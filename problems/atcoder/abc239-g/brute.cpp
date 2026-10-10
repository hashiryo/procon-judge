// 愚直解。2 から N - 1 の頂点の集合をすべて試し、除くと 1 から N へ行けなくなるもののうち費用が最小のものを出す。
// 最大流を使わないので、参照実装とは別の考え方になる。N ≤ 12 の入力 (gen.py の seed 1000 以上) でだけ使う。
#include <cstdio>
#include <vector>
using namespace std;
using i64 = long long;
int main() {
  int n, m;
  if (scanf("%d %d", &n, &m) != 2) return 1;
  vector<vector<int>> adj(n + 1);
  for (int i = 0; i < m; ++i) {
    int a, b;
    if (scanf("%d %d", &a, &b) != 2) return 1;
    adj[a].push_back(b), adj[b].push_back(a);
  }
  vector<i64> c(n + 1);
  for (int i = 1; i <= n; ++i) if (scanf("%lld", &c[i]) != 1) return 1;
  const int k = n - 2;
  i64 best = -1;
  int bm = 0;
  for (int mask = 0; mask < (1 << k); ++mask) {
    i64 cost = 0;
    vector<char> blocked(n + 1, 0), vis(n + 1, 0);
    for (int j = 0; j < k; ++j) if (mask >> j & 1) blocked[j + 2] = 1, cost += c[j + 2];
    if (best >= 0 && cost >= best) continue;
    vector<int> st{1};
    vis[1] = 1;
    while (!st.empty()) {
      int u = st.back();
      st.pop_back();
      for (int v : adj[u]) if (!vis[v] && !blocked[v]) vis[v] = 1, st.push_back(v);
    }
    if (!vis[n]) best = cost, bm = mask;
  }
  vector<int> wall;
  for (int j = 0; j < k; ++j) if (bm >> j & 1) wall.push_back(j + 2);
  printf("%lld\n%d\n", best, (int)wall.size());
  for (size_t i = 0; i < wall.size(); ++i) printf("%d%c", wall[i], i + 1 == wall.size() ? '\n' : ' ');
  if (wall.empty()) printf("\n");
}
