#pragma once
// 今の Library の UndirectedGraphSetPowerSeries::perfect_matching。組 (n/2 個) の部分集合ごとの閉路の重みを数え、集合の冪級数の
// exp で完全マッチングの重みの和を求める。
#include "../common.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/graph/UndirectedGraphSetPowerSeries.hpp"
inline u32 run(int N, const vector<vector<u32>>& a) {
 using Mint= ModInt<MOD>;
 UndirectedGraphSetPowerSeries g(N);
 for(int i= 0; i < N; ++i)
  for(int j= i + 1; j < N; ++j) g.add_edge(i, j, a[i][j]);
 return g.perfect_matching<Mint>().val();
}
