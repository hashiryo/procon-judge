#pragma once
// _shared/mulfunc/dirichlet.hpp の O(N^{2/3}) の畳み込み。
#include "common.hpp"
#include "_shared/mulfunc/dirichlet.hpp"

struct Solver {
  u64 n;
  vector<u32> F, G;
  Quotients q;
  vector<u32> H;

  Solver(u64 n_, vector<u32> f, vector<u32> g)
      : n(n_), F(std::move(f)), G(std::move(g)), q(make_quotients(n_)) {}

  void run() { H = mf_dirichlet::convolve(n, F, G, DC_MOD); }
  const vector<u32> &answer() const { return H; }
};
