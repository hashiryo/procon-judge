#pragma once
// 愚直解: Newton の差分商で係数を求める (crosscheck 用、点は少ない)。
inline vector<u32> run(const vector<u32>& x, const vector<u32>& y) {
 constexpr u64 M= 998244353;
 auto inv= [](u64 a) {
  u64 r= 1, e= M - 2;
  for(a%= M; e; e>>= 1, a= a * a % M)
   if(e & 1) r= r * a % M;
  return r;
 };
 const int n= int(x.size());
 vector<u64> d(y.begin(), y.end());
 for(int k= 1; k < n; ++k)
  for(int i= n - 1; i >= k; --i) d[i]= (d[i] + M - d[i - 1]) % M * inv((x[i] + M - x[i - k]) % M) % M;
 // Newton の形 Σ d_k Π_{j<k} (X - x_j) を係数に直す
 vector<u64> c(n, 0), b{1};
 for(int k= 0; k < n; ++k) {
  for(size_t t= 0; t < b.size(); ++t) c[t]= (c[t] + d[k] * b[t]) % M;
  vector<u64> nb(b.size() + 1, 0);
  for(size_t t= 0; t < b.size(); ++t) nb[t + 1]= (nb[t + 1] + b[t]) % M, nb[t]= (nb[t] + (M - x[k]) * b[t]) % M;
  b= nb;
 }
 return vector<u32>(c.begin(), c.end());
}
