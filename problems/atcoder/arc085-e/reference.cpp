// 期待出力を作る参照実装。割る宝石の集合は倍数について閉じている (i を割るなら i の倍数も割る) ものに限られ、そういう集合は
// どれも作れる。残す宝石を s の側に置く最大重み閉包として、a_i > 0 なら s から i へ a_i、a_i < 0 なら i から t へ -a_i、
// i の倍数 j について j から i へ大きな容量の辺を張り、正の a_i の和から最小カットを引く。Dinic はここに書く。
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
  int n;
  if (scanf("%d", &n) != 1) return 1;
  vector<long long> a(n + 1);
  for (int i = 1; i <= n; ++i)
    if (scanf("%lld", &a[i]) != 1) return 1;
  const int s = 0, t = n + 1;
  const long long INF = 1LL << 50;
  Dinic d(n + 2);
  long long pos = 0;
  for (int i = 1; i <= n; ++i) {
    if (a[i] > 0) d.add(s, i, a[i]), pos += a[i];
    else if (a[i] < 0) d.add(i, t, -a[i]);
    for (int j = 2 * i; j <= n; j += i) d.add(j, i, INF);
  }
  printf("%lld\n", pos - d.flow(s, t));
}
