#pragma once
// Library の Matrix<Algebra<max-plus>> (大きさを実行時に決める行列) で、reference と同じ手順を踏む。辺のない所は NEG = -2^29 とし、
// 積は max(a + b, NEG) で NEG 以上に保つ (NEG を含む和は本当の値に勝たない)。
#include "../common.hpp"
#include "mylib/algebra/Algebra.hpp"
#include "mylib/algebra/Matrix.hpp"
struct MaxPlusOp {
 using T= int;
 static constexpr int NEG= -(1 << 29);
 static constexpr int o= NEG;  // 和 (max) の単位元
 static constexpr int i= 0;    // 積 (+) の単位元
 static T add(T a, T b) { return max(a, b); }
 static T mul(T a, T b) { return max(a + b, NEG); }
};
inline int solve(int n, const vector<array<int, 4>>& doors) {
 using R= Algebra<MaxPlusOp>;
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
