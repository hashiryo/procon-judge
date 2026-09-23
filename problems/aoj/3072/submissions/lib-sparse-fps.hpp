#pragma once
#include "common.hpp"
#include "mylib/algebra/sparse_fps.hpp"

// 分母の 0 でない項が 3 つしかないことを使って、漸化式を直接回す O(K)。
// 畳み込みを使わないぶん、項の少ない分母では圧倒的に軽い。
struct Solver {
  int n, k, p;
  Mint ans;

  Solver(int n, int k, int p) : n(n), k(k), p(p) {}

  void run() {
    Mint pr = Mint(p) / 100, c = (Mint(1) - pr) / pr;
    vector<Mint> f(n + 2);
    f[0] = n, f[1] = -(pr + n), f[n + 1] = pr;
    auto g = sfps::div({c * n, -c * n}, f, k);
    ans = 1;
    for (int i = 1; i < k; ++i) ans -= g[i];
  }

  i64 answer() const { return ans.val(); }
};
