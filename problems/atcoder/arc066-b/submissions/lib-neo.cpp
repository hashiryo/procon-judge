// lib.cpp と同じ NFA と 2 つの DFA の積を NeoLibrary の DFA (neo/counting/DFA.hpp) で数える。u ≤ N と v ≤ N は、下の桁から
// 読む leq_digits_lsb を、記号 (u の桁, v の桁) の片方を読む map_symbols で写して表す。
// https://atcoder.jp/contests/arc066/tasks/arc066_b
#include <iostream>
#include <vector>
#include <array>
#include <set>
#include "neo/counting/DFA.hpp"
#include "neo/algebra/ZMod.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using Mint= ZMod<int(1e9 + 7)>;
 long long N;
 cin >> N;
 using symbol_t= array<bool, 2>;
 vector<symbol_t> alp= {{0, 0}, {0, 1}, {1, 0}, {1, 1}};
 auto tr= [&](int s, symbol_t c) {
  auto [u, v]= c;
  set<int> ret;
  for(int a= 2; a--;)
   for(int b= 2; b--;)
    if((a ^ b) == u && (a + b + s) % 2 == v) ret.insert((a + b + s) / 2);
  return ret;
 };
 auto ac= [&](int s) { return s == 0; };
 DFA nfa(alp, 0, tr, ac);
 auto dfa_le0= leq_digits_lsb(N, 2).map_symbols(alp, [](symbol_t c) { return int(c[0]); });
 auto dfa_le1= leq_digits_lsb(N, 2).map_symbols(alp, [](symbol_t c) { return int(c[1]); });
 cout << (nfa & dfa_le0 & dfa_le1).count<Mint>(60) << '\n';
 return 0;
}
