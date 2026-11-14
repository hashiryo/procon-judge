// 期待出力を作る参照実装。選んだ行を s の側、選んだ列を t の側に置くと、行 r と列 c をどちらも選ぶことが辺 r から c が切れる
// ことになる。行と列の和 R_r、C_c を得点、両方で選んだマスの A_{r,c} を差し引き (負なら禁止の大きな容量) とする最小カットを
// 組み、得点の上界 (正の R と C の和) から引く。Dinic はここに書く。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。
#include <algorithm>
#include <cstdio>
#include <queue>
#include <vector>
using namespace std;
struct Dinic {
  struct E { int to, rev; long long cap; };
  vector<vector<E>> g;
  vector<int> lv, it;
  explicit Dinic(int n) : g(n), lv(n), it(n) {}
  void add(int u, int v, long long c) {
    g[u].push_back({v, (int)g[v].size(), c});
    g[v].push_back({u, (int)g[u].size() - 1, 0});
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
  int h, w;
  if (scanf("%d %d", &h, &w) != 2) return 1;
  vector<vector<long long>> a(h, vector<long long>(w));
  for (auto& row : a)
    for (auto& x : row)
      if (scanf("%lld", &x) != 1) return 1;
  const int s = h + w, t = s + 1;
  const long long INF = 1LL << 50;
  Dinic d(h + w + 2);
  long long base = 0;
  for (int r = 0; r < h; ++r) {
    long long sum = 0;
    for (int c = 0; c < w; ++c) sum += a[r][c];
    if (sum >= 0) base += sum, d.add(s, r, sum);  // 選ばない (t の側) と失う
    else d.add(r, t, -sum);                       // 選ぶ (s の側) と払う
  }
  for (int c = 0; c < w; ++c) {
    long long sum = 0;
    for (int r = 0; r < h; ++r) sum += a[r][c];
    if (sum >= 0) base += sum, d.add(h + c, t, sum);  // 選ばない (s の側) と失う
    else d.add(s, h + c, -sum);                       // 選ぶ (t の側) と払う
  }
  for (int r = 0; r < h; ++r)
    for (int c = 0; c < w; ++c)
      if (a[r][c] != 0) d.add(r, h + c, a[r][c] > 0 ? a[r][c] : INF);
  printf("%lld\n", base - d.flow(s, t));
}
