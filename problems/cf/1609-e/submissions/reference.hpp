#pragma once
// 期待出力を作る参照実装。min-plus の 3 × 3 の行列を int の配列で持ち、セグメント木も手で書く。
// brute.hpp と小さい入力で突き合わせてある (pj testdata crosscheck --brute brute.hpp)。
#include "../common.hpp"
#include "_char_mat.hpp"
struct Solver {
 using M3= array<int, 9>;
 static M3 mul(const M3& x, const M3& y) {
  M3 z;
  for(int i= 0; i < 3; ++i)
   for(int j= 0; j < 3; ++j) z[i * 3 + j]= min({x[i * 3] + y[j], x[i * 3 + 1] + y[3 + j], x[i * 3 + 2] + y[6 + j], CM_INF});
  return z;
 }
 int sz= 1;
 vector<M3> t;
 explicit Solver(const string& s) {
  const int n= s.size();
  while(sz < n) sz<<= 1;
  constexpr int I= CM_INF;
  t.assign(2 * sz, M3{0, I, I, I, 0, I, I, I, 0});
  for(int i= 0; i < n; ++i) t[sz + i]= char_mat(s[i]);
  for(int i= sz - 1; i >= 1; --i) t[i]= mul(t[2 * i], t[2 * i + 1]);
 }
 int update(int x, char c) {
  int i= x + sz;
  t[i]= char_mat(c);
  for(i>>= 1; i; i>>= 1) t[i]= mul(t[2 * i], t[2 * i + 1]);
  return min({t[1][0], t[1][1], t[1][2]});
 }
};
