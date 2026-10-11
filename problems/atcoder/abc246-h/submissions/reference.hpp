#pragma once
// 期待出力を作る参照実装。3 × 3 の行列を u32 の配列で持ち、成分ごとに u64 で 3 個の積を足して M で割った余りに直す。
// セグメント木も手で書く。brute.hpp と小さい入力で突き合わせてある (pj testdata crosscheck)。
#include "../common.hpp"
#include "_gen_mat.hpp"
struct Solver {
 using M3= array<u32, 9>;
 static M3 mul(const M3& x, const M3& y) {
  M3 z;
  for(int i= 0; i < 3; ++i)
   for(int j= 0; j < 3; ++j) z[i * 3 + j]= u32((u64(x[i * 3]) * y[j] + u64(x[i * 3 + 1]) * y[3 + j] + u64(x[i * 3 + 2]) * y[6 + j]) % MOD);
  return z;
 }
 int sz= 1;
 vector<M3> t;
 explicit Solver(const string& s) {
  const int n= s.size();
  while(sz < n) sz<<= 1;
  t.assign(2 * sz, M3{1, 0, 0, 0, 1, 0, 0, 0, 1});
  for(int i= 0; i < n; ++i) t[sz + i]= char_mat(s[i]);
  for(int i= sz - 1; i >= 1; --i) t[i]= mul(t[2 * i], t[2 * i + 1]);
 }
 u32 update(int x, char c) {
  int i= x + sz;
  t[i]= char_mat(c);
  for(i>>= 1; i; i>>= 1) t[i]= mul(t[2 * i], t[2 * i + 1]);
  return (t[1][6] + t[1][7]) % MOD;
 }
};
