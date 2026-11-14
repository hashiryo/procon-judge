// 期待出力を作る参照実装。(i + j) が奇数のマスの色を反転すると、隣り合う 2 マスの色が違うことが、反転したあとで色が同じ
// ことになる。反転したあとで色の違う隣の組の数の最小を、隣どうしに容量 1 の辺を両向きに張った最小カットで求め、組の総数
// 2N(N - 1) から引く。色の決まったマスは、反転したあとの色に合わせて s か t に大きな容量でつなぐ。Dinic はここに書く。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。
#include <algorithm>
#include <cstdio>
#include <queue>
#include <string>
#include <vector>
using namespace std;
struct Dinic {
  struct E { int to, rev; long long cap; };
  vector<vector<E>> g;
  vector<int> lv, it;
  explicit Dinic(int n) : g(n), lv(n), it(n) {}
  void add(int u, int v, long long c, long long rc = 0) {
    g[u].push_back({v, (int)g[v].size(), c});
    g[v].push_back({u, (int)g[u].size() - 1, rc});
  }
  long long dfs(int v, int t, long long f) {
    if (v == t) return f;
    for (int& i = it[v]; i < (int)g[v].size(); ++i) {
      E& e = g[v][i];
      if (e.cap > 0 && lv[e.to] == lv[v] + 1) {
        long long d = dfs(e.to, t, min(f, e.cap));
        if (d > 0) {
          e.cap -= d, g[e.to][e.rev].cap += d;
          return d;
        }
      }
    }
    return 0;
  }
  long long flow(int s, int t) {
    long long res = 0;
    while (true) {
      fill(lv.begin(), lv.end(), -1);
      queue<int> q;
      lv[s] = 0, q.push(s);
      while (!q.empty()) {
        int v = q.front();
        q.pop();
        for (auto& e : g[v])
          if (e.cap > 0 && lv[e.to] < 0) lv[e.to] = lv[v] + 1, q.push(e.to);
      }
      if (lv[t] < 0) return res;
      fill(it.begin(), it.end(), 0);
      for (long long f; (f = dfs(s, t, 1LL << 60)) > 0;) res += f;
    }
  }
};
int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  vector<string> c(n);
  for (auto& row : c) {
    char buf[128];
    if (scanf("%127s", buf) != 1) return 1;
    row = buf;
  }
  const int s = n * n, t = s + 1;
  const long long INF = 1LL << 40;
  Dinic d(n * n + 2);
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j) {
      const int v = i * n + j;
      if (c[i][j] != '?') {
        const int col = (c[i][j] == 'W') ^ ((i + j) & 1);
        if (col == 0) d.add(s, v, INF);
        else d.add(v, t, INF);
      }
      if (i + 1 < n) d.add(v, v + n, 1, 1);
      if (j + 1 < n) d.add(v, v + 1, 1, 1);
    }
  printf("%lld\n", 2LL * n * (n - 1) - d.flow(s, t));
}
