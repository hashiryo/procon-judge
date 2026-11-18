#pragma once
// 愚直解: 基底ごとに Π (x - j)/(k - j) を 1 つずつ求める O(K^2) (crosscheck 用)。
inline u32 run(const vector<u32>& y, u64 x0) {
 constexpr long long M= 998244353;
 const int K= int(y.size());
 const long long x= (long long)(x0 % M);
 auto inv= [](long long a) {
  long long r= 1, e= M - 2;
  a%= M;
  if(a < 0) a+= M;
  for(; e; e>>= 1, a= a * a % M)
   if(e & 1) r= r * a % M;
  return r;
 };
 long long res= 0;
 for(int k= 0; k < K; ++k) {
  long long num= 1, den= 1;
  for(int j= 0; j < K; ++j)
   if(j != k) num= num * (((x - j) % M + M) % M) % M, den= den * (((k - j) % M + M) % M) % M;
  res= (res + y[k] * num % M * inv(den)) % M;
 }
 return u32(res);
}
