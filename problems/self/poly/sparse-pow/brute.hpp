#pragma once
// 愚直解: 2 乗の繰り返しで f^m mod x^n を求める (掛け算は素朴、crosscheck 用)。
#include <utility>
inline vector<u32> run(int n, u64 m, const vector<pair<int, u32>>& f) {
 constexpr unsigned long long M= 998244353;
 auto mul= [&](const vector<unsigned long long>& a, const vector<unsigned long long>& b) {
  vector<unsigned long long> c(n, 0);
  for(int i= 0; i < n; ++i)
   for(int j= 0; i + j < n; ++j) c[i + j]= (c[i + j] + a[i] * b[j]) % M;
  return c;
 };
 vector<unsigned long long> r(n, 0), x(n, 0);
 if(n == 0) return {};
 r[0]= 1;
 for(auto [i, a]: f)
  if(i < n) x[i]= a;
 for(; m; m>>= 1) {
  if(m & 1) r= mul(r, x);
  if(m > 1) x= mul(x, x);
 }
 return vector<u32>(r.begin(), r.end());
}
