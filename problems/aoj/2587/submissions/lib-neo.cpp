// lib.cpp と同じ ε 遷移のある NFA を NeoLibrary の DFA (neo/counting/DFA.hpp) で組む。長さ N から 2N までの数は count_all の 1 回で求める。
#include <iostream>
#include <vector>
#include <set>
#include <array>
#include "neo/counting/DFA.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int N;
 vector<int> low, high;
 vector alp= {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
 while(cin >> N && N != 0) {
  low.resize(N), high.resize(N);
  for(int i= 0; i < N; ++i) cin >> low[i] >> high[i];
  using state_t= array<int, 2>;
  auto tr= [&](state_t s, int c) -> set<state_t> {
   auto [i, j]= s;
   if(i >= N) return {};
   auto [l1, l0]= div(low[i], 10);
   auto [h1, h0]= div(high[i], 10);
   if(j == 1) {
    if(l0 <= c) return {{i + 1, 0}};
   } else if(j == 2) return {{i + 1, 0}};
   else if(j == 3) {
    if(c <= h0) return {{i + 1, 0}};
   } else if(j == 4) {
    if(l0 <= c && c <= h0) return {{i + 1, 0}};
   } else {
    if(c == 0) return {};
    if(l1 == h1) {
     if(c == l1) return {{i, 4}};
    } else {
     if(c == l1) return {{i, 1}};
     if(l1 < c && c < h1) return {{i, 2}};
     if(c == h1) return {{i, 3}};
    }
   }
   return {};
  };
  auto eps= [&](state_t s) -> set<state_t> {
   auto [i, j]= s;
   if(i >= N) return {};
   if(j == 0 && (low[i] / 10) == 0) {
    if((high[i] / 10) == 0) return {{i, 4}};
    else return {{i, 1}};
   }
   return {};
  };
  auto ac= [&](state_t s) { return s[0] == N && s[1] == 0; };
  DFA nfa(alp, state_t{0, 0}, tr, ac, eps);
  auto cnt= nfa.count_all<int64_t>(2 * N);
  int64_t ans= 0;
  for(int i= N; i <= 2 * N; ++i) ans+= cnt[i];
  cout << ans << '\n';
 }
 return 0;
}
