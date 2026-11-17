#pragma once
#include "_seg.hpp"

// 無向の辺から HLD を隣の番号の xor で葉から剥がして作る (中身は _hld.hpp)。前向きの木と、位置を逆に並べた木の 2 本のセグメント木 (_seg.hpp) を
// 持ち、u から lca へ上る区間は逆の木で、lca から v へ下りる区間は前向きの木で、値 x に順に当てる。
struct Solver {
  comp_hld::Tree t;
  comp_seg::Seg fw, rv;
  Solver(int n, const vector<array<i64, 2>> &f, const vector<array<int, 2>> &edges)
      : t(comp_hld::Tree::make<comp_hld::Build::Xor>(n, edges)) {
    auto lv = comp_seg::leaves(t, f);
    fw = comp_seg::Seg(n, lv.data());
    std::reverse(lv.begin(), lv.end());
    rv = comp_seg::Seg(n, lv.data());
  }
  void set(int p, i64 c, i64 d) {
    const comp_seg::F g{mont::enc(c), mont::enc(d)};
    const int x = t.pos[p];
    fw.set(x, g), rv.set(t.n - 1 - x, g);
  }
  i64 composite(int u, int v, i64 x) {
    u32 val = mont::enc(x);
    const int m = t.n - 1;
    t.path_ordered(
        u, v, [&](int l, int r) { val = rv.fwd(m - r, m - l, val); }, [&](int l, int r) { val = fw.fwd(l, r, val); });
    return mont::dec(val);
  }
};
