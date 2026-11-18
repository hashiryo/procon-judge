#pragma once
// 愚直解: 商の次数の高い方から、a - q b の上の項が消えるように係数を 1 つずつ決め直す (掛け算を毎回全部やり直す、crosscheck 用)。
#include <utility>
inline pair<vector<u32>, vector<u32>> run(const vector<u32>& a, const vector<u32>& b) {
 constexpr long long M= 998244353;
 auto inv= [](long long x) {
  long long r= 1, e= M - 2;
  for(x%= M; e; e>>= 1, x= x * x % M)
   if(e & 1) r= r * x % M;
  return r;
 };
 const int n= int(a.size()), m= int(b.size());
 vector<long long> q(n - m + 1, 0);
 auto rem= [&]() {  // a - q b
  vector<long long> r(a.begin(), a.end());
  for(int i= 0; i <= n - m; ++i)
   for(int j= 0; j < m; ++j) r[i + j]= ((r[i + j] - q[i] * b[j]) % M + M) % M;
  return r;
 };
 for(int i= n - m; i >= 0; --i) {
  const auto r= rem();
  q[i]= r[i + m - 1] * inv(b[m - 1]) % M;
 }
 const auto r= rem();
 return {vector<u32>(q.begin(), q.end()), vector<u32>(r.begin(), r.begin() + (m - 1))};
}
