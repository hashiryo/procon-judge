#pragma once
// 期待出力を作る参照実装。素朴な Dinic で、隣接リストを vector の vector で持ち、DFS は 1 本ずつ流す。
// ライブラリも _shared も include しない (期待出力のキャッシュの鍵はこのファイルの中身だけで決まるため)。
#include <algorithm>
#include <limits>
#include <queue>
#include <vector>

struct Solver {
  struct Edge {
    int to, rev;
    long long cap;
  };
  int n, s, t;
  vector<vector<Edge>> g;
  vector<int> lv, it;
  long long ans = 0;

  Solver(int n, int s, int t, const vector<array<int, 3>> &edges) : n(n), s(s), t(t), g(n), lv(n), it(n) {
    for (auto &e : edges) {
      if (e[0] == e[1]) continue;
      g[e[0]].push_back({e[1], (int)g[e[1]].size(), e[2]});
      g[e[1]].push_back({e[0], (int)g[e[0]].size() - 1, 0});
    }
  }

  long long dfs(int u, long long f) {
    if (u == t) return f;
    for (int &i = it[u]; i < (int)g[u].size(); ++i) {
      Edge &e = g[u][i];
      if (e.cap > 0 && lv[e.to] == lv[u] + 1) {
        long long d = dfs(e.to, min(f, e.cap));
        if (d > 0) {
          e.cap -= d, g[e.to][e.rev].cap += d;
          return d;
        }
      }
    }
    return 0;
  }

  void run() {
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
      if (lv[t] < 0) break;
      fill(it.begin(), it.end(), 0);
      for (long long f; (f = dfs(s, numeric_limits<long long>::max())) > 0;) ans += f;
    }
  }

  long long answer() const { return ans; }
};
