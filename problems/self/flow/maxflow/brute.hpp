#pragma once
// 愚直解。容量を隣接行列に足し合わせ、BFS で見つけた最短の増加路に 1 本ずつ流す Edmonds-Karp。
// 参照実装 (Dinic) と違って層を作らず、辺を頂点の組でまとめる。小さい入力 (gen.py の seed 1000 以上) でだけ使う。
#include <queue>
#include <vector>

struct Solver {
  int n, s, t;
  vector<vector<long long>> c;
  long long ans = 0;

  Solver(int n, int s, int t, const vector<array<int, 3>> &edges) : n(n), s(s), t(t), c(n, vector<long long>(n)) {
    for (auto &e : edges)
      if (e[0] != e[1]) c[e[0]][e[1]] += e[2];
  }

  void run() {
    for (;;) {
      vector<int> par(n, -1);
      par[s] = s;
      queue<int> q;
      q.push(s);
      while (!q.empty() && par[t] < 0) {
        int u = q.front();
        q.pop();
        for (int v = 0; v < n; ++v)
          if (par[v] < 0 && c[u][v] > 0) par[v] = u, q.push(v);
      }
      if (par[t] < 0) break;
      long long d = c[par[t]][t];
      for (int v = t; v != s; v = par[v]) d = min(d, c[par[v]][v]);
      for (int v = t; v != s; v = par[v]) c[par[v]][v] -= d, c[v][par[v]] += d;
      ans += d;
    }
  }

  long long answer() const { return ans; }
};
