#pragma once
#include "pj.hpp"
#include "mylib/data_structure/SegmentTree_Patricia.hpp"
#include "neo/base/ds/tree/HeavyLightDecomposition.hpp"

// common.hpp の PathSolver と同じ解き方を、木を NeoLibrary の HeavyLightDecomposition にして書いたもの。根から各頂点までに現れた
// 値の個数は、今の Library の永続なパトリシア木で持つ (NeoLibrary に永続な木はまだ無い)。根から順に版を作るところは、再帰の
// DFS をやめて HLD の位置の順 (親が子より前) に回す。common.hpp は今の Library の HLD を読むので、名前がぶつからないよう
// ここでは読まない。
struct Solver {
  struct CountSum {
    using T = int;
    static T ti() { return 0; }
    static T op(T l, T r) { return l + r; }
  };
  using Seg = SegmentTree_Patricia<CountSum, true>;
  int n;
  vector<i64> x;
  vector<array<int, 2>> edges;
  vector<array<int, 3>> qs;
  vector<i64> ans;
  Solver(int n, const vector<i64> &x, const vector<array<int, 2>> &edges, const vector<array<int, 3>> &qs)
      : n(n), x(x), edges(edges), qs(qs) {}
  void run() {
    vector<array<int, 2>> es(edges);
    es.push_back({0, 1});  // 仮の根 0 を頂点 1 の親にする
    HeavyLightDecomposition tree(n + 1, es, 0);
    vector<Seg> segs(n + 1);
    for (int i = 1; i <= n; ++i) {
      const int v = tree.to_vertex(i), p = tree.parent(v);
      segs[v] = segs[p];
      segs[v].set((int)x[v], segs[v][(int)x[v]] + 1);
    }
    ans.clear();
    ans.reserve(qs.size());
    for (auto &e : qs) {
      const int v = e[0], w = e[1], l = e[2];
      // segs[v] + segs[w] - segs[lca] - segs[parent(lca)] が道の上の個数。
      auto check = [&](int a, int b, int c, int d) { return a + b - c - d >= l; };
      const int lca = tree.lca(v, w), lcap = tree.parent(lca);
      ans.push_back(Seg::template find_first<4>(0, check, {segs[v], segs[w], segs[lca], segs[lcap]}));
    }
  }
  const vector<i64> &answer() const { return ans; }
};
