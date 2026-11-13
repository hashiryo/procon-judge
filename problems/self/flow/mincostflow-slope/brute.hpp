#pragma once
// 愚直解。最短路を使わず、1 単位ずつ「どれでもよい増加路で流してから、残余グラフの負閉路を Bellman-Ford で見つけて
// 消す」ことを繰り返す (Klein の方法)。負閉路の無い流れはその流量で費用が最小なので、1 単位ごとの費用の和がそのまま
// g(x) になる。参照実装 (逐次最短路) と考え方が違う。小さい入力 (gen.py の seed 1000 以上) でだけ使う。呼ぶ時点で負閉路が
// あれば (ハーネスの保証が崩れていれば) stderr に書いて止める。
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

struct Solver {
  struct Arc {
    int from, to;
    i64 cap, cost;
  };
  int n;
  vector<Arc> a;  // 辺 i の順向きが 2 i、逆向きが 2 i + 1

  explicit Solver(int n) : n(n) {}
  int add_vertex() { return n++; }
  int add_edge(int from, int to, i64 cap, i64 cost) {
    a.push_back({from, to, cap, cost});
    a.push_back({to, from, 0, -cost});
    return (int)a.size() / 2 - 1;
  }
  i64 flow(int i) const { return a[2 * i + 1].cap; }

  void push(int k, i64 f) { a[k].cap -= f, a[k ^ 1].cap += f; }

  // 負閉路を 1 つ見つけて流せるだけ流す。無ければ false。
  bool cancel_one() {
    vector<i64> d(n, 0);
    vector<int> pre(n, -1);
    int last = -1;
    for (int it = 0; it < n; ++it) {
      last = -1;
      for (int k = 0; k < (int)a.size(); ++k)
        if (a[k].cap > 0 && d[a[k].from] + a[k].cost < d[a[k].to]) d[a[k].to] = d[a[k].from] + a[k].cost, pre[a[k].to] = k, last = a[k].to;
      if (last < 0) return false;
    }
    int v = last;
    for (int it = 0; it < n; ++it) v = a[pre[v]].from;
    vector<int> cyc;
    i64 f = numeric_limits<i64>::max();
    for (int u = v;;) {
      const int k = pre[u];
      cyc.push_back(k), f = min(f, a[k].cap), u = a[k].from;
      if (u == v) break;
    }
    for (int k : cyc) push(k, f);
    return true;
  }

  // s から t への増加路を BFS で 1 本見つけて 1 単位流す。無ければ false。
  bool augment_one(int s, int t) {
    vector<int> pre(n, -1);
    vector<char> vis(n, 0);
    queue<int> q;
    q.push(s), vis[s] = 1;
    while (!q.empty() && !vis[t]) {
      const int u = q.front();
      q.pop();
      for (int k = 0; k < (int)a.size(); ++k)
        if (a[k].from == u && a[k].cap > 0 && !vis[a[k].to]) vis[a[k].to] = 1, pre[a[k].to] = k, q.push(a[k].to);
    }
    if (!vis[t]) return false;
    for (int v = t; v != s; v = a[pre[v]].from) push(pre[v], 1);
    return true;
  }

  i64 total_cost() const {
    i64 c = 0;
    for (int k = 0; k < (int)a.size(); k += 2) c += a[k + 1].cap * a[k].cost;
    return c;
  }

  vector<pair<i64, i64>> slope(int s, int t, i64 limit) {
    if (cancel_one()) {
      fprintf(stderr, "brute: slope を呼んだ時点で負閉路がある\n");
      exit(1);
    }
    vector<pair<i64, i64>> res = {{0, 0}};
    if (s == t) return res;
    const i64 base = total_cost();
    for (i64 x = 1; x <= limit; ++x) {
      if (!augment_one(s, t)) break;
      while (cancel_one()) {}
      res.push_back({x, total_cost() - base});
    }
    return res;
  }
};
