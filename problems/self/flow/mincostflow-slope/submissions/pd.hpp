#pragma once
// primal-dual。Dijkstra は ACL と同じ (縮約費用 0 の辺で着いた頂点は二分ヒープを通さずに積み、訪れた頂点だけポテンシャル
// を動かす) だが、t を取り出したあとも、t と同じ距離の頂点を取り出し終えるまで続ける。そのあと道を 1 本だけ流すのでなく、
// 縮約費用が 0 になった辺だけを使う DFS (今の弧を覚え、行き止まりの頂点を捨てる形。段数は付けず、今たどっている道の頂点
// へは戻らない) で、その距離の道を流せるだけ流す。DFS は Dijkstra で取り出した頂点の中だけを歩く。CSR は呼び出しを
// またいで持ち、辺が足されたときだけ流量を保って組み直す。初めの負の費用と、あとから足した辺で縮約費用が負になったときは、
// 流す前にポテンシャルを直す (残余グラフが DAG なら位相順に、そうでなければキューを使う Bellman-Ford で)。
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <utility>
#include <vector>

struct Solver {
  using Cap = i64;
  using Cost = i64;
  static constexpr Cost INF = numeric_limits<Cost>::max();
  int n;
  vector<int> efrom, eto;
  vector<Cap> ecap;
  vector<Cost> ecost;
  // CSR
  int built_m = 0, built_n = 0;
  vector<int> start, to, rev, pos;
  vector<Cap> cap;
  vector<Cost> cost;
  vector<Cost> p;  // ポテンシャル
  // 作業
  vector<Cost> dist;
  vector<int> prv, cur, touched, visited, path;
  vector<char> vis, dead, onpath;

  explicit Solver(int n) : n(n) {}
  int add_vertex() { return n++; }
  int add_edge(int from, int to_, Cap c, Cost w) {
    efrom.push_back(from), eto.push_back(to_), ecap.push_back(c), ecost.push_back(w);
    return (int)efrom.size() - 1;
  }
  i64 flow(int i) const { return i < built_m ? ecap[i] - cap[pos[i]] : 0; }

  void build() {
    const int m = efrom.size();
    if (m == built_m && n == built_n) return;
    vector<Cap> f(m, 0);
    for (int i = 0; i < built_m; ++i) f[i] = ecap[i] - cap[pos[i]];
    start.assign(n + 1, 0);
    for (int i = 0; i < m; ++i) ++start[efrom[i] + 1], ++start[eto[i] + 1];
    for (int v = 0; v < n; ++v) start[v + 1] += start[v];
    vector<int> c(start.begin(), start.end() - 1);
    to.resize(2 * m), rev.resize(2 * m), cap.resize(2 * m), cost.resize(2 * m), pos.resize(m);
    for (int i = 0; i < m; ++i) {
      const int a = c[efrom[i]]++, b = c[eto[i]]++;
      to[a] = eto[i], rev[a] = b, cap[a] = ecap[i] - f[i], cost[a] = ecost[i];
      to[b] = efrom[i], rev[b] = a, cap[b] = f[i], cost[b] = -ecost[i];
      pos[i] = a;
    }
    Cost mx = 0;
    for (Cost x : p) mx = max(mx, x);
    p.resize(n, mx);
    dist.resize(n, INF), prv.resize(n), cur.resize(n), vis.resize(n, 0), dead.resize(n, 0), onpath.resize(n, 0);
    // 足した辺の縮約費用が負なら、ポテンシャルを直す。
    bool bad = false;
    for (int i = built_m; i < m && !bad; ++i)
      if (cap[pos[i]] > 0 && p[efrom[i]] + ecost[i] < p[eto[i]]) bad = true;
    built_m = m, built_n = n;
    if (bad) repair();
  }

  void repair() {
    vector<int> indeg(n, 0), order;
    order.reserve(n);
    for (int a = 0; a < (int)to.size(); ++a)
      if (cap[a] > 0) ++indeg[to[a]];
    for (int v = 0; v < n; ++v)
      if (!indeg[v]) order.push_back(v);
    for (size_t k = 0; k < order.size(); ++k) {
      const int u = order[k];
      for (int a = start[u]; a < start[u + 1]; ++a)
        if (cap[a] > 0 && !--indeg[to[a]]) order.push_back(to[a]);
    }
    if ((int)order.size() == n) {
      for (int u : order)
        for (int a = start[u]; a < start[u + 1]; ++a)
          if (cap[a] > 0 && p[u] + cost[a] < p[to[a]]) p[to[a]] = p[u] + cost[a];
      return;
    }
    vector<int> q(n), cnt(n, 0);
    vector<char> inq(n, 1);
    for (int v = 0; v < n; ++v) q[v] = v;
    for (size_t head = 0; head < q.size(); ++head) {
      const int u = q[head];
      inq[u] = 0;
      for (int a = start[u]; a < start[u + 1]; ++a)
        if (cap[a] > 0 && p[u] + cost[a] < p[to[a]]) {
          p[to[a]] = p[u] + cost[a];
          if (!inq[to[a]]) {
            if (++cnt[to[a]] > n) {
              fprintf(stderr, "pd: 残余グラフに負閉路がある\n");
              exit(1);
            }
            inq[to[a]] = 1, q.push_back(to[a]);
          }
        }
    }
  }

  struct Q {
    Cost key;
    int to;
    bool operator<(Q r) const { return key > r.key; }
  };
  vector<int> que_min;
  vector<Q> que;

  bool dijkstra(int s, int t) {
    for (int v : touched) dist[v] = INF;
    for (int v : visited) vis[v] = 0;
    touched.clear(), visited.clear(), que_min.clear(), que.clear();
    size_t heap_r = 0;
    Cost dt = INF;
    dist[s] = 0, touched.push_back(s), que_min.push_back(s);
    while (!que_min.empty() || !que.empty()) {
      int v;
      if (!que_min.empty()) {
        v = que_min.back();
        que_min.pop_back();
      } else {
        while (heap_r < que.size()) ++heap_r, push_heap(que.begin(), que.begin() + heap_r);
        v = que.front().to;
        pop_heap(que.begin(), que.end());
        que.pop_back();
        --heap_r;
      }
      if (vis[v]) continue;
      if (dist[v] > dt) break;
      vis[v] = 1, visited.push_back(v);
      if (v == t) dt = dist[v];
      const Cost dv = dist[v], pv = p[v];
      for (int a = start[v]; a < start[v + 1]; ++a) {
        if (!cap[a]) continue;
        const int w = to[a];
        const Cost nd = dv + cost[a] + pv - p[w];
        if (nd < dist[w]) {
          if (dist[w] == INF) touched.push_back(w);
          dist[w] = nd, prv[w] = a;
          if (nd == dv) que_min.push_back(w);
          else que.push_back(Q{nd, w});
        }
      }
    }
    if (!vis[t]) return false;
    for (int v : visited) p[v] += dist[v] - dt;
    return true;
  }

  // 縮約費用 0 の辺だけで、s から t へ limit まで流す。
  Cap augment(int s, int t, Cap limit) {
    for (int v : visited) cur[v] = start[v], dead[v] = 0;
    Cap pushed = 0;
    path.clear();
    int u = s;
    onpath[s] = 1;
    while (true) {
      if (u == t) {
        Cap f = limit - pushed;
        for (int a : path) f = min(f, cap[a]);
        for (int a : path) cap[a] -= f, cap[rev[a]] += f;
        pushed += f;
        if (pushed == limit) break;
        size_t k = 0;
        while (cap[path[k]] > 0) ++k;
        for (size_t j = k; j < path.size(); ++j) onpath[to[path[j]]] = 0;
        path.resize(k);
        u = k ? to[path[k - 1]] : s;
        continue;
      }
      const Cost pu = p[u];
      int a = cur[u];
      const int e = start[u + 1];
      for (; a < e; ++a) {
        const int w = to[a];
        if (cap[a] > 0 && vis[w] && !dead[w] && !onpath[w] && cost[a] + pu == p[w]) break;
      }
      cur[u] = a;
      if (a < e) {
        path.push_back(a), onpath[to[a]] = 1, u = to[a];
        continue;
      }
      dead[u] = 1;
      if (u == s) break;
      onpath[u] = 0;
      const int b = path.back();
      path.pop_back();
      u = to[rev[b]];
      ++cur[u];
    }
    for (int a : path) onpath[to[a]] = 0;
    onpath[s] = 0;
    return pushed;
  }

  vector<pair<Cap, Cost>> slope(int s, int t, Cap limit) {
    build();
    vector<pair<Cap, Cost>> res = {{0, 0}};
    if (s == t) return res;
    Cap fl = 0;
    Cost total = 0, prev_d = 0;
    while (fl < limit && dijkstra(s, t)) {
      const Cost d = p[t] - p[s];
      const Cap f = augment(s, t, limit - fl);
      fl += f, total += f * d;
      if (res.size() >= 2 && prev_d == d) res.pop_back();
      res.push_back({fl, total}), prev_d = d;
    }
    return res;
  }
};
