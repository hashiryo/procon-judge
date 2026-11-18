#pragma once
// 愚直解: x^i を 1 つずつ作って Σ f_i x^i を足す (crosscheck 用)。
inline vector<u32> run(const vector<u32>& f, const vector<u32>& x) {
 constexpr unsigned long long M= 998244353;
 vector<u32> r;
 for(u32 p: x) {
  unsigned long long s= 0, w= 1;
  for(u32 a: f) s= (s + a * w) % M, w= w * p % M;
  r.push_back(u32(s));
 }
 return r;
}
