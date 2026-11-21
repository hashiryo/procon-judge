// lib.cpp の Period を NeoLibrary の FunctionalGraph に替えたもの。to を作る線形漸化式 (法 10000) は今の Library の
// linear_recurrence と ModInt のまま。
#include <iostream>
#include <string>
#include "mylib/algebra/ModInt.hpp"
#include "mylib/algebra/bostan_mori.hpp"
#include "neo/base/ds/tree/FunctionalGraph.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 constexpr int N= 10000;
 using Mint= ModInt<N>;
 string s;
 cin >> s;
 int a= stoi(s);
 long long M, L;
 cin >> M >> L;
 vector<int> to(N);
 for(int n= 0; n < N; ++n) {
  auto x= linear_recurrence<Mint>({n, 1}, {0, 1}, M);
  if(M & 1) x-= 1;
  to[n]= x.val();
 }
 string ans= to_string(FunctionalGraph(to).jump(a, L));
 ans= string(4 - ans.length(), '0') + ans;
 cout << ans << '\n';
 return 0;
}
