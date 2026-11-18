#pragma once
// 愚直解: 漸化式をそのまま k 項目まで回す (crosscheck 用、k ≤ 3000)。
inline u32 run(const vector<u32>& a, const vector<u32>& c, u64 k) {
 constexpr u32 M= 998244353;
 const int d= int(a.size());
 vector<u32> s(a.begin(), a.end());
 while(s.size() <= k) {
  const int n= int(s.size());
  u64 v= 0;
  for(int j= 1; j <= d; ++j) v= (v + u64(c[j - 1]) * s[n - j]) % M;
  s.push_back(u32(v));
 }
 return s[k];
}
