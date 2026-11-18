// cf-343-e のチェッカ。引数は入力、提出の出力、期待出力の順 (pj の約束)。
//
// 給料の最大は答えが 1 つに決まるので、期待出力の 1 つ目の数と同じことを確かめる。順列はどれでもよいので、1 から n の
// 順列であることと、隣り合う駅の組の最大流 (素朴な Dinic で流し直す) の和が給料に等しいことを確かめる。
#include <algorithm>
#include <cstdio>
#include <limits>
#include <queue>
#include <vector>
using namespace std;
using i64 = long long;
struct Dinic {
  struct E {
    int to, rev;
    i64 cap;
  };
  vector<vector<E>> g;
  vector<int> lv, it;
  Dinic(int n) : g(n), lv(n), it(n) {}
  void add(int u, int v, i64 c) {
    g[u].push_back({v, (int)g[v].size(), c});
    g[v].push_back({u, (int)g[u].size() - 1, c});
  }
  i64 dfs(int u, int t, i64 f) {
    if (u == t) return f;
    for (int &i = it[u]; i < (int)g[u].size(); ++i) {
      E &e = g[u][i];
      if (e.cap > 0 && lv[e.to] == lv[u] + 1) {
        i64 d = dfs(e.to, t, min(f, e.cap));
        if (d > 0) {
          e.cap -= d, g[e.to][e.rev].cap += d;
          return d;
        }
      }
    }
    return 0;
  }
  i64 flow(int s, int t) {
    i64 f = 0;
    for (;;) {
      fill(lv.begin(), lv.end(), -1);
      queue<int> q;
      lv[s] = 0, q.push(s);
      while (!q.empty()) {
        int u = q.front();
        q.pop();
        for (auto &e : g[u])
          if (e.cap > 0 && lv[e.to] < 0) lv[e.to] = lv[u] + 1, q.push(e.to);
      }
      if (lv[t] < 0) return f;
      fill(it.begin(), it.end(), 0);
      while (i64 d = dfs(s, t, numeric_limits<i64>::max())) f += d;
    }
  }
};
int main(int argc, char **argv) {
  if (argc < 4) return 2;
  FILE *in = fopen(argv[1], "r");
  FILE *out = fopen(argv[2], "r");
  FILE *ans = fopen(argv[3], "r");
  if (!in || !out || !ans) return 2;
  int n, m;
  if (fscanf(in, "%d %d", &n, &m) != 2) return 2;
  vector<int> a(m), b(m), c(m);
  for (int i = 0; i < m; ++i) {
    if (fscanf(in, "%d %d %d", &a[i], &b[i], &c[i]) != 3) return 2;
    --a[i], --b[i];
  }
  i64 want, got;
  if (fscanf(ans, "%lld", &want) != 1) return 2;
  if (fscanf(out, "%lld", &got) != 1) return fprintf(stderr, "給料を読めません\n"), 1;
  if (got != want) return fprintf(stderr, "給料が %lld ですが、期待出力は %lld です\n", got, want), 1;
  vector<int> p(n);
  vector<char> used(n, 0);
  for (int i = 0; i < n; ++i) {
    if (fscanf(out, "%d", &p[i]) != 1) return fprintf(stderr, "順列の %d 番目を読めません\n", i + 1), 1;
    if (p[i] < 1 || p[i] > n || used[p[i] - 1]) return fprintf(stderr, "順列の %d 番目の %d が範囲の外か重複です\n", i + 1, p[i]), 1;
    used[--p[i]] = 1;
  }
  i64 sum = 0;
  for (int i = 0; i + 1 < n; ++i) {
    Dinic d(n);
    for (int j = 0; j < m; ++j) d.add(a[j], b[j], c[j]);
    sum += d.flow(p[i], p[i + 1]);
  }
  if (sum != got) return fprintf(stderr, "順列の隣り合う組の最大流の和が %lld で、給料の %lld と違います\n", sum, got), 1;
  return 0;
}
