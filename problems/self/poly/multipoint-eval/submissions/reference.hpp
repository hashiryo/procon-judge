#pragma once
// 参照実装 (ライブラリを読まない、intrinsics を使わない)。点ごとに Horner 法で、1 回ごとに剰余を取る。
#include <vector>
inline vector<u32> run(const vector<u32>& f, const vector<u32>& x) {
 constexpr u64 M= 998244353;
 const int n= int(f.size()), m= int(x.size());
 vector<u32> r(m);
 for(int j= 0; j < m; ++j) {
  u64 a= 0;
  for(int i= n - 1; i >= 0; --i) a= (a * x[j] + f[i]) % M;
  r[j]= u32(a);
 }
 return r;
}
