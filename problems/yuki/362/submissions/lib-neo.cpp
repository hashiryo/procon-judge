// lib.cpp と同じ二分探索と数え上げを NeoLibrary の DFA (neo/counting/DFA.hpp) で行う。X 以下の DFA は、下の桁から読む
// leq_digits_lsb にする。桁の数が X より少ない門松数の数は、二分探索の前に count_all の 1 回で求めておく。
#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include "neo/counting/DFA.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 vector alp= {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
 using state_t= array<int, 2>;
 auto tr_kado= [&](state_t s, int c) -> state_t {
  if(s[0] == -1) {
   if(s[1] == c) return {-2, -2};
   else return {s[1], c};
  }
  if(s[0] == c || s[1] == c) return {-2, -2};
  auto [mi, mx]= minmax({s[0], s[1], c});
  if(s[1] == mi || s[1] == mx) return {s[1], c};
  return {-2, -2};
 };
 auto ac_kado= [&](state_t) { return true; };
 DFA dfa_kado(alp, state_t{-1, -1}, tr_kado, ac_kado, state_t{-2, -2});
 auto tr_0= [&](bool, int c) -> bool { return c; };
 DFA dfa_0(alp, false, tr_0, [](bool s) { return s; });
 auto dfa1= dfa_kado & dfa_0;
 auto cnt= dfa1.count_all<int64_t>(15);
 int T;
 cin >> T;
 while(T--) {
  int64_t K;
  cin >> K;
  int64_t l= 101, h= 37294859064823;
  while(h - l > 1) {
   int64_t X= (h + l) / 2;
   int n= 0;
   for(auto m= X; m; m/= 10) ++n;
   auto dfa= dfa1 & leq_digits_lsb(X, 10);
   int64_t sum= dfa.count<int64_t>(n);
   for(int i= 3; i < n; ++i) sum+= cnt[i];
   if(sum < K) l= X;
   else h= X;
  }
  cout << h << '\n';
 }
 return 0;
}
