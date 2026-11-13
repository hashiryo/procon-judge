#pragma once
// ACL の mcf_graph (atcoder/mincostflow.hpp) の写し。slope を呼ぶたびに辺の一覧から CSR を組み、Dijkstra (縮約費用 0 の
// 辺で着いた頂点は二分ヒープを通さずに積む que_min、t を取り出したら打ち切り、訪れた頂点だけポテンシャルを動かす) で
// 1 本ずつ流し、最後に流量を辺の一覧へ書き戻す。ACL から変えたのは 2 つ。ポテンシャルを呼び出しをまたいで持つことと
// (ACL は呼ぶたびに 0 から始めるので、流したあとに続けて呼べない)、前の呼び出しのあとに足した辺の縮約費用が負なら、
// 流す前にポテンシャルを直すこと (残余グラフが DAG なら位相順に、そうでなければキューを使う Bellman-Ford で)。
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <utility>
#include <vector>

struct Solver {
  using Cap = i64;
  using Cost = i64;
  struct E {
    int from, to;
    Cap cap, flow;
    Cost cost;
  };
  struct Arc {
    int to, rev;
    Cap cap;
    Cost cost;
  };
  int n;
  vector<E> es;
  vector<Cost> dual;
  int done_m = 0;  // ポテンシャルを合わせ済みの辺の数

  explicit Solver(int n) : n(n) {}
  int add_vertex() { return n++; }
  int add_edge(int from, int to, Cap cap, Cost cost) {
    es.push_back({from, to, cap, 0, cost});
    return (int)es.size() - 1;
  }
  i64 flow(int i) const { return es[i].flow; }

  // CSR の残余グラフで、ポテンシャルを直す。新しい頂点の値は今のポテンシャルの最大。
  void repair(const vector<int> &start, const vector<Arc> &arc) {
    Cost mx = 0;
    for (Cost d : dual) mx = max(mx, d);
    dual.resize(n, mx);
    const int m = es.size();
    bool bad = false;
    for (int i = done_m; i < m && !bad; ++i) {
      const E &e = es[i];
      if (e.cap - e.flow > 0 && dual[e.from] + e.cost < dual[e.to]) bad = true;
    }
    done_m = m;
    if (!bad) return;
    // 位相順を試す。
    vector<int> indeg(n, 0), order;
    order.reserve(n);
    for (const Arc &a : arc)
      if (a.cap > 0) ++indeg[a.to];
    for (int v = 0; v < n; ++v)
      if (!indeg[v]) order.push_back(v);
    for (size_t k = 0; k < order.size(); ++k) {
      const int u = order[k];
      for (int j = start[u]; j < start[u + 1]; ++j)
        if (arc[j].cap > 0 && !--indeg[arc[j].to]) order.push_back(arc[j].to);
    }
    if ((int)order.size() == n) {
      for (int u : order)
        for (int j = start[u]; j < start[u + 1]; ++j)
          if (arc[j].cap > 0 && dual[u] + arc[j].cost < dual[arc[j].to]) dual[arc[j].to] = dual[u] + arc[j].cost;
      return;
    }
    // キューを使う Bellman-Ford。すべての頂点から始める。
    vector<int> q(n), cnt(n, 0);
    vector<char> inq(n, 1);
    for (int v = 0; v < n; ++v) q[v] = v;
    size_t head = 0;
    while (head < q.size()) {
      const int u = q[head++];
      inq[u] = 0;
      for (int j = start[u]; j < start[u + 1]; ++j) {
        const Arc &a = arc[j];
        if (a.cap > 0 && dual[u] + a.cost < dual[a.to]) {
          dual[a.to] = dual[u] + a.cost;
          if (!inq[a.to]) {
            if (++cnt[a.to] > n) {
              fprintf(stderr, "acl: 残余グラフに負閉路がある\n");
              exit(1);
            }
            inq[a.to] = 1, q.push_back(a.to);
          }
        }
      }
    }
  }

  vector<pair<Cap, Cost>> slope(int s, int t, Cap flow_limit) {
    const int m = es.size();
    vector<int> edge_idx(m), start(n + 1, 0);
    vector<Arc> arc(2 * m);
    {
      vector<int> degree(n, 0), redge_idx(m);
      for (int i = 0; i < m; ++i) edge_idx[i] = degree[es[i].from]++, redge_idx[i] = degree[es[i].to]++;
      for (int v = 0; v < n; ++v) start[v + 1] = start[v] + degree[v];
      for (int i = 0; i < m; ++i) {
        const E &e = es[i];
        edge_idx[i] += start[e.from], redge_idx[i] += start[e.to];
        arc[edge_idx[i]] = {e.to, redge_idx[i], e.cap - e.flow, e.cost};
        arc[redge_idx[i]] = {e.from, edge_idx[i], e.flow, -e.cost};
      }
    }
    repair(start, arc);
    vector<pair<Cap, Cost>> result = {{Cap(0), Cost(0)}};
    if (s == t) return result;

    vector<pair<Cost, Cost>> dual_dist(n);
    for (int v = 0; v < n; ++v) dual_dist[v].first = dual[v];
    vector<int> prev_e(n);
    vector<bool> vis(n);
    struct Q {
      Cost key;
      int to;
      bool operator<(Q r) const { return key > r.key; }
    };
    vector<int> que_min;
    vector<Q> que;
    auto dual_ref = [&]() {
      for (int i = 0; i < n; i++) dual_dist[i].second = numeric_limits<Cost>::max();
      fill(vis.begin(), vis.end(), false);
      que_min.clear();
      que.clear();
      size_t heap_r = 0;
      dual_dist[s].second = 0;
      que_min.push_back(s);
      while (!que_min.empty() || !que.empty()) {
        int v;
        if (!que_min.empty()) {
          v = que_min.back();
          que_min.pop_back();
        } else {
          while (heap_r < que.size()) {
            heap_r++;
            push_heap(que.begin(), que.begin() + heap_r);
          }
          v = que.front().to;
          pop_heap(que.begin(), que.end());
          que.pop_back();
          heap_r--;
        }
        if (vis[v]) continue;
        vis[v] = true;
        if (v == t) break;
        Cost dual_v = dual_dist[v].first, dist_v = dual_dist[v].second;
        for (int i = start[v]; i < start[v + 1]; i++) {
          auto e = arc[i];
          if (!e.cap) continue;
          Cost cost = e.cost - dual_dist[e.to].first + dual_v;
          if (dual_dist[e.to].second - dist_v > cost) {
            Cost dist_to = dist_v + cost;
            dual_dist[e.to].second = dist_to;
            prev_e[e.to] = e.rev;
            if (dist_to == dist_v) que_min.push_back(e.to);
            else que.push_back(Q{dist_to, e.to});
          }
        }
      }
      if (!vis[t]) return false;
      for (int v = 0; v < n; v++) {
        if (!vis[v]) continue;
        dual_dist[v].first -= dual_dist[t].second - dual_dist[v].second;
      }
      return true;
    };
    Cap flow = 0;
    Cost cost = 0, prev_cost_per_flow = -1;
    bool first = true;
    while (flow < flow_limit) {
      if (!dual_ref()) break;
      Cap c = flow_limit - flow;
      for (int v = t; v != s; v = arc[prev_e[v]].to) c = min(c, arc[arc[prev_e[v]].rev].cap);
      for (int v = t; v != s; v = arc[prev_e[v]].to) {
        auto &e = arc[prev_e[v]];
        e.cap += c;
        arc[e.rev].cap -= c;
      }
      Cost d = dual_dist[t].first - dual_dist[s].first;
      flow += c;
      cost += c * d;
      if (!first && prev_cost_per_flow == d) result.pop_back();
      result.push_back({flow, cost});
      prev_cost_per_flow = d, first = false;
    }
    for (int v = 0; v < n; ++v) dual[v] = dual_dist[v].first;
    for (int i = 0; i < m; i++) es[i].flow = es[i].cap - arc[edge_idx[i]].cap;
    return result;
  }
};
