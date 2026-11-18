#pragma once
#include "common.hpp"
#include "_shared/seqtree/wbt_seq.hpp"

// wbtseq.hpp の写しで、構築の前に置き場を 2^22 個ぶん先に取る (WbtSeq::reserve)。永続では操作のたびに節を新しく作るので、
// 伸びる配列を作り直して写す手間が、今の Library (先に取った静的な配列の番号を進めるだけ) との差になっているかを見る。
struct Solver {
  using P = WbtSeq<RangeMin>;
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
