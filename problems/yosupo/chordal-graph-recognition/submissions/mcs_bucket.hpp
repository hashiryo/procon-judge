#pragma once
#include <algorithm>
#include <cassert>

#include "pj.hpp"

// 最大基数探索 (MCS) で完全除去順序 (PEO) を試す。弦グラフでなければ、MCS の証人
// (x, z, y) から x と N(x)\{z} を避けた最短経路を作り、z..y-x の誘導閉路を返す。
// 公式解 (graph/chordal_graph_recognition/sol/correct.cpp) と同じ形で、
// MCS の選択を遅延バケット (重みごとの vector、選択済みを捨てる) で行う O(N+M) の版。
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

    // 遅延バケットの MCS。重み w のバケットから選ぶ。選択済みの頂点は取り出しで捨てる。
    vector<int> weight(n, 0), pos(n, -1);
    vector<vector<int>> buc(n);
    buc[0].reserve(n);
    for (int i = 0; i < n; ++i) buc[0].push_back(i);
    ord.reserve(n);
    int maxw = 0;
    for (int i = 0; i < n; ++i) {
      int x = -1;
      while (true) {
        while (maxw >= 0 && buc[maxw].empty()) --maxw;
        if (maxw < 0) break;
        x = buc[maxw].back(), buc[maxw].pop_back();
        if (pos[x] == -1) break;
      }
      pos[x] = i;
      ord.push_back(x);
      for (int y : adj[x]) {
        if (pos[y] != -1) continue;
        ++weight[y];
        buc[weight[y]].push_back(y);
        if (weight[y] > maxw) maxw = weight[y];
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
