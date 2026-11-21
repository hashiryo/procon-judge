#pragma once
#include "pj.hpp"
#include "neo/base/ds/tree/FunctionalGraph.hpp"

// 直前 4 項 (mod 17) を状態にした写像を、状態を 17 進の数 (17^4 = 83521 通り) にして NeoLibrary の FunctionalGraph に渡す。
// lib-period は今の Library の Period で、始めの状態から訪れた状態だけをライブラリの中で番号付けする (状態が int でないので
// std::map)。NeoLibrary は int の関数だけを受けるので、全部の状態の行き先を先に作る。
struct Solver {
  vector<i64> ns, ans;

  explicit Solver(const vector<i64> &ns) : ns(ns) {}

  void run() {
    constexpr int P = 17, S = P * P * P * P;
    vector<int> to(S);
    for (int s = 0; s < S; ++s) {
      const int x0 = s / (P * P * P), x1 = s / (P * P) % P, x2 = s / P % P, x3 = s % P;
      to[s] = ((x1 * P + x2) * P + x3) * P + (x0 + x1 + x2 + x3) % P;
    }
    FunctionalGraph g(to);
    const int init = 1;  // (0, 0, 0, 1)
    ans.clear();
    for (i64 n : ns) ans.push_back(g.jump(init, n - 1) / (P * P * P));
  }

  const vector<i64> &answer() const { return ans; }
};
