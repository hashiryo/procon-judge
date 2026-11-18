#pragma once
// k = 2 は hitonanode/cplib-cpp の combinatorial_opt/edge_disjoint_min_spanning_forests.hpp (森 2 本の専用の算法、
// O(NM)) を写したもの (重みは使わず、辺を番号の順に入れる)。辺ごとに 2 本の森を辺の始点から DFS し直し、交換の列は、
// たどった頂点に印を付けて木の上の道を印のある頂点まで登る形で探す。k != 2 は greedy.hpp と同じ union_bfs で解く。
#include "_shared/matroid/union_algos.hpp"

namespace hito {
inline vector<int> two_forests(int n0, const vector<array<int, 2>> &edges) {
  const int M = edges.size();
  // 辺に現れる頂点だけを番号に詰める
  vector<int> id(n0, -1);
  int N = 0;
  vector<pair<int, int>> uvs(M);
  for (int e = 0; e < M; e++) {
    for (int s = 0; s < 2; ++s)
      if (id[edges[e][s]] < 0) id[edges[e][s]] = N++;
    uvs[e] = {id[edges[e][0]], id[edges[e][1]]};
  }
  vector<vector<bool>> I(2, vector<bool>(M));
  vector<vector<pair<int, int>>> to(N);
  int nb_accepted_edges = 0;
  auto accept_edge = [&](int e) {
    nb_accepted_edges++;
    int u = uvs[e].first, v = uvs[e].second;
    to[u].emplace_back(v, e);
    to[v].emplace_back(u, e);
  };
  auto dfs = [&](int head, const vector<bool> &I) -> vector<pair<int, int>> {
    vector<int> st{head};
    vector<pair<int, int>> prv(N, {-1, -1});
    prv[head] = {head, -1};
    while (!st.empty()) {
      int now = st.back();
      st.pop_back();
      for (auto p : to[now]) {
        int nxt = p.first, e = p.second;
        if (!I[e] or prv[nxt].first >= 0) continue;
        prv[nxt] = {now, e}, st.push_back(nxt);
      }
    }
    return prv;
  };
  vector<int> prveid(M, -1);
  vector<vector<int>> L(2), visited(2, vector<int>(N));
  vector<vector<pair<int, int>>> prv(2);
  for (int e = 0; e < M; ++e) {
    if (nb_accepted_edges >= 2 * (N - 1)) break;
    const int u = uvs[e].first, v = uvs[e].second;
    bool found = false;
    for (int d = 0; d < 2; d++) {
      prv[d] = dfs(u, I[d]);
      if (prv[d][v].first < 0) {
        accept_edge(e);
        I[d][e] = 1;
        found = true;
        break;
      }
    }
    if (found) continue;
    visited[0].assign(N, 0);
    visited[1].assign(N, 0);
    visited[0][u] = visited[1][u] = 1;
    L[0] = {e}, L[1] = {};
    int ehead = -1;
    prveid[e] = -1;
    for (int i = 0;; i ^= 1) {
      if (L[i].empty()) break;
      L[i ^ 1].clear();
      while (!L[i].empty()) {
        const int exy = L[i].back();
        L[i].pop_back();
        int x = uvs[exy].first, y = uvs[exy].second;
        if (prv[i][x].first < 0 or prv[i][y].first < 0) {
          ehead = exy;
          break;
        }
        if (!visited[i][x]) swap(x, y);
        while (!visited[i][y]) {
          int nxty = prv[i][y].first, nxte = prv[i][y].second;
          L[i ^ 1].push_back(nxte);
          visited[i][y] = 1;
          y = nxty;
          prveid[nxte] = exy;
        }
      }
      if (ehead >= 0) {
        accept_edge(e);
        int c = I[0][ehead];
        for (; ehead >= 0; ehead = prveid[ehead], c ^= 1) I[c][ehead] = 1, I[c ^ 1][ehead] = 0;
        break;
      }
    }
  }
  vector<int> own(M, -1);
  for (int e = 0; e < M; ++e)
    if (I[0][e]) own[e] = 0;
    else if (I[1][e]) own[e] = 1;
  return own;
}
}  // namespace hito

struct Solver {
  int n, k;
  const vector<array<int, 2>> &edges;
  vector<int> ans;

  Solver(int n, const vector<array<int, 2>> &edges, int k) : n(n), k(k), edges(edges) {}

  void run() {
    if (k == 2) ans = hito::two_forests(n, edges);
    else {
      vector<mu::Graphic> ms(k, mu::Graphic(n, edges));
      ans = mu::union_bfs<true, true, true, true>(ms);
    }
  }

  vector<int> answer() const { return ans; }
};
