#pragma once
// NeoLibrary の KaryOptimization (neo/combinatorial_optimization/KaryOptimization.hpp) で書いたもの。国 i の選び方を
// x_i in {0: 行くがツアーは無し, 1: 行かない, 2: ツアーも行く} とし、満足度の符号を変えた費用を最小にする (common.hpp と
// 同じ帰着)。条件 (D, E) は x_D = 2 かつ x_E = 0 を禁じる 2 変数の項で、禁止の費用も common.hpp と同じ 2^62 にする。
#include "neo/combinatorial_optimization/KaryOptimization.hpp"
struct Solver {
 vector<array<i64, 2>> bc;
 vector<array<int, 2>> de;
 i64 ans= 0;
 Solver(const vector<array<i64, 2>>& bc, const vector<array<int, 2>>& de): bc(bc), de(de) {}
 void run() {
  const int n= bc.size();
  const i64 INF= 1LL << 62;
  KaryOptimization<i64> ko(n, 3);
  for(int i= 0; i < n; ++i) ko.add(i, vector<i64>{-bc[i][1], 0, -bc[i][0]});
  for(auto [d, e]: de) ko.add(d, e, [&](int x, int y) { return x == 2 && y == 0 ? INF : i64(0); });
  ans= -*ko.solve();
 }
 i64 answer() const { return ans; }
};
