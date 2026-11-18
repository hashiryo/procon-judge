// lib.cpp を NeoLibrary の Rerooting (木は辺の列で渡す) で書いたもの。値は bool。
#include <array>
#include <iostream>
#include <vector>
#include "neo/graph/Rerooting.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N;
 cin >> N;
 vector<array<int, 2>> es(N - 1);
 for(auto& [a, b]: es) cin >> a >> b, --a, --b;
 auto put_edge= [&](int, int, bool d) { return d; };
 auto op= [&](bool l, bool r) { return l | r; };
 auto put_vertex= [&](int, bool d) { return !d; };
 Rerooting<bool> dp(N, es, put_edge, op, false, put_vertex);
 vector<int> ans;
 for(int i= 0; i < N; ++i)
  if(dp[i]) ans.push_back(i + 1);
 cout << ans.size() << '\n';
 for(int x: ans) cout << x << '\n';
 return 0;
}
