#pragma once
#include "common.hpp"
#include "mylib/data_structure/UnionFind_Undoable.hpp"

// 巻き戻せる Union-Find を 2N 頂点で使う。頂点 u が「x_u = 1」、u + N が
// 「x_u = 0」を表し、同じ値なら (u, v) と (u+N, v+N)、違う値なら (u, v+N) と
// (u+N, v) を繋ぐ。u と u+N が繋がったら矛盾。1 つの制約で unite が 2 回要る
// 代わりに、重みの計算が無い。
struct Solver {
  int n0, comps;
  bool ok = true;
  UnionFind_Undoable uf;
  vector<i64> pw;

  explicit Solver(int n) : n0(n), comps(n), uf(n + n), pw(pow2_table(n)) {}

  i64 same(int u, int v) {
    comps -= uf.unite(u, v);
    uf.unite(u + n0, v + n0);
    ok &= !uf.connected(u, u + n0);
    return count();
  }

  i64 differ(int u, int v) {
    comps -= uf.unite(u, v + n0);
    uf.unite(u + n0, v);
    ok &= !uf.connected(u, u + n0);
    return count();
  }

  i64 reset() {
    uf.rollback(0);
    comps = n0, ok = true;
    return count();
  }

  i64 count() const { return ok ? pw[comps] : 0; }
};
