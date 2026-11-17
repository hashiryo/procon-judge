#pragma once
#include "_seg.hpp"

// 無向の辺から HLD を隣の番号の xor で葉から剥がして作る (中身は _hld.hpp)。1 本のセグメント木 (_seg.hpp の Seg1) の節に、1 次関数の掛ける数と、
// 前向きと後ろ向きの足す数を持ち、u から lca へ上る区間は後ろ向きに、lca から v へ下りる区間は前向きに、値 x に順に当てる。
struct Solver {
  comp_hld::Tree t;
  comp_seg::Seg1 s;
  Solver(int n, const vector<array<i64, 2>> &f, const vector<array<int, 2>> &edges)
      : t(comp_hld::Tree::make<comp_hld::Build::Xor>(n, edges)), s(n, comp_seg::leaves(t, f).data()) {}
  void set(int p, i64 c, i64 d) { s.set(t.pos[p], {mont::enc(c), mont::enc(d)}); }
  i64 composite(int u, int v, i64 x) {
    u32 val = mont::enc(x);
    t.path_ordered(
        u, v, [&](int l, int r) { val = s.bwd(l, r, val); }, [&](int l, int r) { val = s.fwd(l, r, val); });
    return mont::dec(val);
  }
};
