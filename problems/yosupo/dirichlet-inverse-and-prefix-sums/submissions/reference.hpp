#pragma once
// _shared/mulfunc/dirichlet.hpp の O(N^{2/3}) の逆元。
#include "common.hpp"
#include "_shared/mulfunc/dirichlet.hpp"

struct Solver {
  u64 n;
  vector<u32> F;
  Quotients q;
  vector<u32> H;

  Solver(u64 n_, vector<u32> f) : n(n_), F(std::move(f)), q(make_quotients(n_)) {}

  void run() { H = mf_dirichlet::inverse(n, F, DC_MOD); }
  const vector<u32> &answer() const { return H; }
};
