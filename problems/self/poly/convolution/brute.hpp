#pragma once
// 愚直解: 定義どおり 1 項ずつ剰余を取って足す (crosscheck 用)。
inline vector<u32> run(const vector<u32>& a, const vector<u32>& b) {
 constexpr u64 M= 998244353;
 vector<u64> c(a.size() + b.size() - 1, 0);
 for(size_t i= 0; i < a.size(); ++i)
  for(size_t j= 0; j < b.size(); ++j) c[i + j]= (c[i + j] + u64(a[i]) * b[j]) % M;
 return vector<u32>(c.begin(), c.end());
}
