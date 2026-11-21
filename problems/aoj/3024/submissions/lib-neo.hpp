#pragma once
#include "common.hpp"
#include "neo/base/ds/seq/WeightBalancedTree.hpp"

// NeoLibrary の neo/base/ds/seq/WeightBalancedTree.hpp (永続の形)。列を丸ごと写す操作は根の写しで済む。構築の前に置き場を
// 2^22 個ぶん先に取り (操作のたびに節を新しく作るので、配列を作り直して写す分を省く)、置き場が伸びたら 2 本の列から届く節だけを
// gc で詰める。
struct Solver {
  using P = WeightBalancedTree<RangeMin, false, true>;
  struct Reserve {
    Reserve() { P::reserve(std::size_t(1) << 22); }
  } reserve_;  // メンバは宣言の順に作られるので、木より先に置く
  P t[2];
  Solver(const vector<i64> &a, const vector<i64> &b) : t{P(to_int(a)), P(to_int(b))} {}
  void set(int k, int i, i64 v) {
    t[k].set(i, (int)v);
    if (P::pool_empty()) P::gc(t[0], t[1]);
  }
  i64 min_of(int k, int l, int r) { return t[k].prod(l, r); }
  void assign(int dst, int src) { t[dst] = t[src]; }
};
