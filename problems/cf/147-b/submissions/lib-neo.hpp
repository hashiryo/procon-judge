#pragma once
// NeoLibrary の Matrix<MaxPlus<int>> (neo/linalg/Matrix.hpp と neo/algebra/semiring.hpp) で、reference と同じ手順を踏む。
// 重みは -10^4 以上 10^4 以下で、扱う行列はどれも n 歩以下の歩みなので、値は -INF か絶対値 3 × 10^6 以下に収まる。どの積も、
// 両方の行列が両側の範囲 (-INF か、絶対値が INF / 4 以下) に入る速い道 (INF の比べのない i-k-j のあと、行ごとに INF / 2 で直す)
// を通る。
#include "../common.hpp"
#include "neo/algebra/semiring.hpp"
#include "neo/linalg/Matrix.hpp"
inline int solve(int n, const vector<array<int, 4>>& doors) {
 using R= MaxPlus<int>;
 using Mat= Matrix<R>;
 Mat a= Mat::identity(n);
 for(auto [i, j, c, d]: doors) a[i][j]= R(max(a[i][j].x, c)), a[j][i]= R(max(a[j][i].x, d));
 int lg= 0;
 while((1 << lg) <= n) ++lg;
 vector<Mat> P(lg);
 P[0]= a;
 for(int k= 1; k < lg; ++k) P[k]= P[k - 1] * P[k - 1];
 Mat cur= Mat::identity(n);
 int len= 0;
 for(int k= lg - 1; k >= 0; --k) {
  if(len + (1 << k) > n) continue;
  Mat t= cur * P[k];
  bool pos= false;
  for(int i= 0; i < n; ++i) pos|= t[i][i].x > 0;
  if(!pos) cur= std::move(t), len+= 1 << k;
 }
 return len >= n ? 0 : len + 1;
}
