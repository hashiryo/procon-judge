// 愚直解。道の数は、切ると 1 から n へ行けなくなる辺の集合のうち最小のものの大きさ (Menger の定理) として、辺の集合を
// すべて試して求める。道そのものは隣接行列の Ford-Fulkerson で作り、その流量が上の最小と食い違えば -1 を出して照合で
// 分かるようにする。m ≤ 12 の入力 (gen.py の seed 1000 以上) でだけ使う。
#include <cstdio>
#include <vector>
using namespace std;
int n, m;
vector<pair<int, int>> es;
bool reach_without(int mask) {
  vector<char> vis(n + 1, 0);
  vector<int> st{1};
  vis[1] = 1;
  while (!st.empty()) {
    int u = st.back();
    st.pop_back();
    for (int i = 0; i < m; ++i)
      if (!(mask >> i & 1) && es[i].first == u && !vis[es[i].second]) vis[es[i].second] = 1, st.push_back(es[i].second);
  }
  return vis[n];
}
int main() {
  if (scanf("%d %d", &n, &m) != 2) return 1;
  es.resize(m);
  for (auto& [a, b] : es) if (scanf("%d %d", &a, &b) != 2) return 1;
  int cut = m;
  for (int mask = 0; mask < (1 << m); ++mask)
    if (__builtin_popcount(mask) < cut && !reach_without(mask)) cut = __builtin_popcount(mask);
  // Ford-Fulkerson (辺ごとの残余の容量を持つ)
  vector<int> f(m, 0);
  int k = 0;
  for (;;) {
    vector<int> pe(n + 1, -2);  // 来た辺 (負なら逆向き、-1 は始点)
    vector<int> st{1};
    pe[1] = -1;
    while (!st.empty() && pe[n] == -2) {
      int u = st.back();
      st.pop_back();
      for (int i = 0; i < m; ++i) {
        if (es[i].first == u && f[i] == 0 && pe[es[i].second] == -2) pe[es[i].second] = i, st.push_back(es[i].second);
        if (es[i].second == u && f[i] == 1 && pe[es[i].first] == -2) pe[es[i].first] = ~i, st.push_back(es[i].first);
      }
    }
    if (pe[n] == -2) break;
    for (int v = n; v != 1;) {
      int e = pe[v];
      if (e >= 0) f[e] = 1, v = es[e].first;
      else f[~e] = 0, v = es[~e].second;
    }
    ++k;
  }
  if (k != cut) return printf("-1\n"), 0;
  vector<vector<int>> out(n + 1);
  for (int i = 0; i < m; ++i) if (f[i]) out[es[i].first].push_back(es[i].second);
  printf("%d\n", k);
  for (int r = 0; r < k; ++r) {
    vector<int> route{1};
    for (int v = 1; v != n;) {
      int w = out[v].back();
      out[v].pop_back();
      route.push_back(w), v = w;
    }
    printf("%d\n", (int)route.size());
    for (size_t i = 0; i < route.size(); ++i) printf("%d%c", route[i], i + 1 == route.size() ? '\n' : ' ');
  }
}
