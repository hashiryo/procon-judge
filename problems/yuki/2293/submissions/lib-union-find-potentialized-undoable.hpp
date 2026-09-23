#pragma once
#include "common.hpp"
#include "mylib/data_structure/UnionFind_Potentialized_Undoable.hpp"

// 巻き戻せるポテンシャル付き Union-Find。頂点は N 個で、x_u xor x_v を辺の
// 重みに持つ。2N 頂点に倍化する手と比べて unite が 1 回で済む代わりに、根まで
// 登りながら xor を畳む。
struct Solver {
  int n0, comps;
  bool ok = true;
  UnionFind_Potentialized_Undoable<bool> uf;
  vector<i64> pw;

  explicit Solver(int n) : n0(n), comps(n), uf(n), pw(pow2_table(n)) {}

  i64 same(int u, int v) { return add(u, v, false); }
  i64 differ(int u, int v) { return add(u, v, true); }

  i64 reset() {
    uf.rollback(0);
    comps = n0, ok = true;
    return count();
  }

  // unite の戻り値は「矛盾しなかったか」で、繋いだかどうかは分からないので、
  // 成分数は先に connected で見る。
  i64 add(int u, int v, bool w) {
    if (!uf.connected(u, v)) --comps;
    ok &= uf.unite(u, v, w);
    return count();
  }

  i64 count() const { return ok ? pw[comps] : 0; }
};
