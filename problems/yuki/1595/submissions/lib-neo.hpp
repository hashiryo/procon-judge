#pragma once
#include "pj.hpp"
#include "neo/base/ds/tree/FunctionalGraph.hpp"

// 直前 3 項の 1 の位を状態にした写像を、状態を 10 進の 3 桁 (1000 通り) にして NeoLibrary の FunctionalGraph に渡す。
// lib-period は今の Library の Period で、状態の組をライブラリの中で番号付けする。
struct Solver {
  i64 p, q, r, k, ans = 0;

  Solver(i64 p, i64 q, i64 r, i64 k) : p(p), q(q), r(r), k(k) {}

  void run() {
    vector<int> to(1000);
    for (int s = 0; s < 1000; ++s) {
      const int x0 = s / 100, x1 = s / 10 % 10, x2 = s % 10;
      to[s] = (x1 * 10 + x2) * 10 + (x0 + x1 + x2) % 10;
    }
    const int init = int(p % 10) * 100 + int(q % 10) * 10 + int(r % 10);
    ans = FunctionalGraph(to).jump(init, k - 1) / 100;
  }

  i64 answer() const { return ans; }
};
