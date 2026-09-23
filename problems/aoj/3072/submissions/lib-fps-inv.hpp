#pragma once
#include "common.hpp"
#include "mylib/fft/fps_inv.hpp"

// 母関数の逆数を密な形式的冪級数として求める O(K log K)。
// 分母の項がほとんど 0 でも、長さ K の配列として扱う。
struct Solver {
  int n, k, p;
  Mint ans;

  Solver(int n, int k, int p) : n(n), k(k), p(p) {}

  void run() {
    Mint pr = Mint(p) / 100, q = Mint(1) - pr;
    vector<Mint> f(n + 1, -pr * pr / n);
    f[0] = pr, f.resize(k);
    auto g = inv<Mint, 1 << 20>(f);
    ans = 1;
    for (int i = 1; i < k; ++i) ans -= g[i] * q;
  }

  i64 answer() const { return ans.val(); }
};
