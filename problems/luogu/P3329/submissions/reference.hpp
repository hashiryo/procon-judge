#pragma once
// 期待出力を作る参照実装。Gusfield の方法 (最大流を n - 1 回、グラフを縮めない) で Gomory-Hu 木を作る。最大流は素朴な
// Dinic で、隣接リストを vector の vector で持ち、組ごとに網を作り直す。最小カットは、流したあとの残余グラフで s から
// 届く頂点の集合。ライブラリも _shared も include しない (期待出力のキャッシュの鍵はこのファイルの中身だけで決まるため)。
//
// Gusfield の方法 (Gusfield 1990、Very simple methods for all pairs network flow analysis の cut tree の形):
//   根を 0 とし、全頂点の親を 0 にする。s = 1, ..., n - 1 の順に、t = p[s] として s-t の最小カット (X, s ∈ X) を求め、
//   fl[s] をその値にする。s 以外で X にあって親が t の頂点は、処理済みかどうかによらず親を s に付け替える。t の親が X に
//   あれば、s を t と t の親の間に入れる (p[s] = p[t]、p[t] = s、重みも入れ替える)。
// 後ろの頂点だけを付け替えて入れ替えもしない簡単な形は、全組の λ は合うが、木の辺で切った集合が最小カットにならない
// ことがある (3 頂点で辺 0-1 が 1、1-2 が 2、0-2 が 3 のとき、辺 2-0 の重みは 4 だが {2} のカットの容量は 5)。
#include <algorithm>
#include <limits>
#include <queue>
#include <vector>

struct Solver {
  struct Edge {
    int to, rev;
    long long cap;
  };
  int n;
  const vector<array<i64, 3>> &edges;
  vector<array<i64, 3>> res;

  Solver(int n, const vector<array<i64, 3>> &edges) : n(n), edges(edges) {}

  struct Dinic {
    int n, s, t;
    vector<vector<Edge>> g;
    vector<int> lv, it;
    Dinic(int n) : n(n), g(n), lv(n), it(n) {}
    void add(int u, int v, long long c) {
      g[u].push_back({v, (int)g[v].size(), c});
      g[v].push_back({u, (int)g[u].size() - 1, c});
    }
    bool bfs() {
      fill(lv.begin(), lv.end(), -1);
      queue<int> q;
      lv[s] = 0, q.push(s);
      while (!q.empty()) {
        int u = q.front();
        q.pop();
        for (auto &e : g[u])
          if (e.cap > 0 && lv[e.to] < 0) lv[e.to] = lv[u] + 1, q.push(e.to);
      }
      return lv[t] >= 0;
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
    long long flow(int s_, int t_) {
      s = s_, t = t_;
      long long f = 0;
      while (bfs()) {
        fill(it.begin(), it.end(), 0);
        while (long long d = dfs(s, numeric_limits<long long>::max())) f += d;
      }
      return f;  // 最後の bfs の lv が、残余グラフで s から届く頂点
    }
  };

  void run() {
    vector<int> p(n, 0);
    vector<long long> fl(n, 0);
    for (int s = 1; s < n; ++s) {
      const int t = p[s];
      Dinic d(n);
      for (auto &e : edges)
        if (e[0] != e[1]) d.add((int)e[0], (int)e[1], e[2]);
      const long long f = d.flow(s, t);
      fl[s] = f;
      for (int i = 0; i < n; ++i)
        if (i != s && d.lv[i] >= 0 && p[i] == t) p[i] = s;
      if (t != 0 && d.lv[p[t]] >= 0) {
        p[s] = p[t], p[t] = s;
        fl[s] = fl[t], fl[t] = f;
      }
    }
    res.clear();
    for (int v = 1; v < n; ++v) res.push_back({v, p[v], fl[v]});
  }
  const vector<array<i64, 3>> &tree() const { return res; }
};
