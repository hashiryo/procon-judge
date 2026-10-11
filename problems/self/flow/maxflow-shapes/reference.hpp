#pragma once
// 期待出力を作る参照実装。素朴な Dinic で、隣接リストを vector の vector で持ち、DFS は 1 本ずつ流す。逆向きの容量は
// 逆向きの弧の初めの容量にする。ライブラリも _shared も include しない (期待出力のキャッシュの鍵はこのファイルの中身だけで
// 決まるため)。DFS は再帰しない (格子では層が数千段になる)。
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

  Solver(int n, int s, int t, const vector<array<int, 4>> &edges) : n(n), s(s), t(t), g(n), lv(n), it(n) {
    for (auto &e : edges) {
      if (e[0] == e[1]) continue;
      g[e[0]].push_back({e[1], (int)g[e[1]].size(), e[2]});
      g[e[1]].push_back({e[0], (int)g[e[0]].size() - 1, e[3]});
    }
  }

  // s から層を 1 つずつ上る道を 1 本探して流す。見つからなければ 0。
  long long augment() {
    vector<int> path;  // 通った頂点の、使った弧の位置 it[u] で辿る
    int u = s;
    for (;;) {
      if (u == t) {
        long long f = numeric_limits<long long>::max();
        for (size_t k = 0; k < path.size(); ++k) f = min(f, g[path[k]][it[path[k]]].cap);
        for (size_t k = 0; k < path.size(); ++k) {
          Edge &e = g[path[k]][it[path[k]]];
          e.cap -= f, g[e.to][e.rev].cap += f;
        }
        return f;
      }
      bool moved = false;
      for (int &i = it[u]; i < (int)g[u].size(); ++i) {
        const Edge &e = g[u][i];
        if (e.cap > 0 && lv[e.to] == lv[u] + 1) {
          path.push_back(u), u = e.to, moved = true;
          break;
        }
      }
      if (!moved) {
        if (path.empty()) return 0;
        lv[u] = -1;  // 行き止まりは外す
        u = path.back(), path.pop_back();
        ++it[u];
      }
    }
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
      for (long long f; (f = augment()) > 0;) ans += f;
    }
  }

  long long answer() const { return ans; }
};
