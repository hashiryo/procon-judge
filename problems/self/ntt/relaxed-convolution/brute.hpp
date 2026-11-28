#pragma once
// 愚直解: 積の定義どおりの二重ループ (crosscheck 用)。u128 にためてから法で割る。
#include <vector>

struct Solver {
 static constexpr unsigned M= 998244353;
 const vector<u32>& f;
 const vector<u32>& g;
 vector<u32> ans;

 Solver(const vector<u32>& f_, const vector<u32>& g_): f(f_), g(g_), ans(f_.size() + g_.size() - 1) {}

 void run() {
  const int n= int(f.size()), m= int(g.size());
  for (int k= 0; k < n + m - 1; ++k) {
   unsigned __int128 s= 0;
   for (int i= std::max(0, k - m + 1), e= std::min(n - 1, k); i <= e; ++i) s+= (unsigned __int128)f[i] * g[k - i];
   ans[k]= unsigned(s % M);
  }
 }

 const vector<u32>& answer() const { return ans; }
};
