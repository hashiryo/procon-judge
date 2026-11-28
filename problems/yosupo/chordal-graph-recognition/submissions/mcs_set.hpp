#pragma once
#include <algorithm>
#include <cassert>
#include <set>

#include "pj.hpp"

// 最大基数探索 (MCS) で完全除去順序 (PEO) を試す。弦グラフでなければ、MCS の証人
// (x, z, y) から x と N(x)\{z} を避けた最短経路を作り、z..y-x の誘導閉路を返す。
// 公式解 (graph/chordal_graph_recognition/sol/correct.cpp) と同じ形で、
// MCS の選択を std::set で行う O((N+M) log N) の版。
struct Solver {
  int n;
  vector<array<int, 2>> edges;
  vector<vector<int>> adj;
  vector<int> ord;   // MCS の選択順 (先に選ばれた方が前)
  vector<int> peo;   // ord の逆。弦グラフのときの PEO
  vector<int> cyc;   // 弦グラフでないときの誘導閉路
  bool is_chordal = true;

  Solver(int n_, const vector<array<int, 2>> &e) : n(n_), edges(e) {}

  void run() {
    adj.assign(n, {});
    for (auto &e : edges) {
      adj[e[0]].push_back(e[1]);
      adj[e[1]].push_back(e[0]);
    }
    for (auto &a : adj) sort(a.begin(), a.end());

    vector<int> weight(n, 0), pos(n, -1);
    set<pair<int, int>> pq;
    for (int i = 0; i < n; ++i) pq.emplace(0, i);
    ord.reserve(n);
    while (!pq.empty()) {
      int x = pq.rbegin()->second;
      pq.erase(prev(pq.end()));
      pos[x] = (int)ord.size();
      ord.push_back(x);
      for (int y : adj[x]) {
        if (pos[y] != -1) continue;
        pq.erase({weight[y], y});
        pq.emplace(++weight[y], y);
      }
    }

    // 各 x について、x より前に選ばれた隣接のうち最も遅いもの z を取り、
    // z より前の隣接 y が z に隣接していなければ証人になる。
    for (int x : ord) {
      int prev_pos = -1;
      for (int y : adj[x])
        if (pos[y] < pos[x] && pos[y] > prev_pos) prev_pos = pos[y];
      if (prev_pos == -1) continue;
      int z = ord[prev_pos];
      for (int y : adj[x]) {
        if (pos[y] < pos[z] && !binary_search(adj[z].begin(), adj[z].end(), y)) {
          build_cycle(x, y, z);
          is_chordal = false;
          return;
        }
      }
    }
    peo.assign(ord.rbegin(), ord.rend());
  }

  // x と N(x)\{z} を避けて y から z へ幅優先。見つけた道 z..y に y-x を足す。
  void build_cycle(int x, int y, int z) {
    vector<char> banned(n, 0);
    banned[x] = 1;
    for (int v : adj[x])
      if (v != z) banned[v] = 1;
    vector<int> dist(n, -1), from(n, -1), que;
    que.reserve(n);
    dist[y] = 0;
    que.push_back(y);
    for (size_t head = 0; head < que.size(); ++head) {
      int u = que[head];
      if (u == z) break;
      for (int v : adj[u]) {
        if (banned[v] || dist[v] != -1) continue;
        dist[v] = dist[u] + 1;
        from[v] = u;
        que.push_back(v);
      }
    }
    assert(dist[z] >= 0);
    cyc.clear();
    for (int u = z; u != y; u = from[u]) cyc.push_back(u);
    cyc.push_back(y);
    cyc.push_back(x);
  }

  bool chordal() const { return is_chordal; }
  const vector<int> &order() const { return peo; }
  const vector<int> &cycle() const { return cyc; }
};
