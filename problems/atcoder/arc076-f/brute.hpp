#pragma once
// 愚直解 (提出ではない)。人と椅子の二部グラフを作り、Kuhn の増加路で最大マッチングを求め、N から引く。小さい入力でだけ
// 使う (pj testdata crosscheck --brute brute.hpp)。
#include <vector>
struct Solver {
 int n, m;
 const vector<array<int, 2>>& lr;
 int ans= 0;
 Solver(int n, int m, const vector<array<int, 2>>& lr): n(n), m(m), lr(lr) {}
 void run() {
  vector<int> owner(m + 1, -1);
  for(int i= 0; i < n; ++i) {
   vector<char> seen(m + 1);
   auto dfs= [&](auto&& self, int p) -> bool {
    for(int c= 1; c <= m; ++c)
     if((c <= lr[p][0] || c >= lr[p][1]) && !seen[c]) {
      seen[c]= 1;
      if(owner[c] < 0 || self(self, owner[c])) return owner[c]= p, true;
     }
    return false;
   };
   if(!dfs(dfs, i)) ++ans;
  }
 }
 int answer() const { return ans; }
};
