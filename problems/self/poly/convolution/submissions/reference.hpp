#pragma once
// 参照実装 (ライブラリを読まない)。短い側の 1 項ずつを行にして u64 にため、16 行ごとに剰余を取る。
#include <vector>
inline vector<u32> run(const vector<u32>& a0, const vector<u32>& b0) {
 constexpr u32 M= 998244353;
 const bool sw= a0.size() > b0.size();
 const vector<u32>&a= sw ? b0 : a0, &b= sw ? a0 : b0;
 const int n= int(a.size()), m= int(b.size());
 vector<u64> acc(n + m - 1);
 for(int i= 0; i < n; ++i) {
  const u64 x= a[i];
  for(int j= 0; j < m; ++j) acc[i + j]+= x * b[j];
  if((i & 15) == 15)
   for(auto& v: acc) v%= M;
 }
 vector<u32> c(n + m - 1);
 for(int k= 0; k < n + m - 1; ++k) c[k]= u32(acc[k] % M);
 return c;
}
