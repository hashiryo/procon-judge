#pragma once
// 愚直解 (pj testdata crosscheck 用)。辺の部分集合を全部試し、頂点 0 から前向きと後ろ向きに全部届くかを見る。m <= 20 を想定。
#include "pj.hpp"
struct Solver {
  int n;
  vector<pair<int, int>> es;
  u32 ans = 0;
  Solver(int n_, const vector<pair<int, int>> &es_) : n(n_), es(es_) {}
  void run() {
    const int m = es.size();
    u64 cnt = 0;
    for (u32 mask = 0; mask < (1u << m); ++mask) {
      vector<u32> fw(n, 0), bw(n, 0);
      for (int i = 0; i < m; ++i)
        if (mask >> i & 1) fw[es[i].first] |= 1u << es[i].second, bw[es[i].second] |= 1u << es[i].first;
      auto reach = [&](const vector<u32> &adj) {
        u32 seen = 1, frontier = 1;
        while (frontier) {
          u32 nx = 0;
          for (int v = 0; v < n; ++v)
            if (frontier >> v & 1) nx |= adj[v];
          frontier = nx & ~seen, seen |= nx;
        }
        return seen;
      };
      const u32 all = (1u << n) - 1;
      if (reach(fw) == all && reach(bw) == all) ++cnt;
    }
    ans = u32(cnt % 1000000007);
  }
  u32 answer() const { return ans; }
};
