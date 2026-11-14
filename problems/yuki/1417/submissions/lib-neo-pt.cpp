// lib-neo.cpp の DFA を、積の来られる組が少ないときに積を表にして進める写し (problems/_shared/automaton/_DFA_pt.hpp)
// に替えたもの。書き比べの記録は algo-notes の notes/automaton-dp.md にある。以下は lib-neo.cpp のまま。
// lib.cpp と同じ数え上げを NeoLibrary の DFA (neo/counting/DFA.hpp) で行う。N 以下の DFA は leq_digits にする。
#include <iostream>
#include <vector>
#include <string>
#include "_shared/automaton/_DFA_pt.hpp"
#include "neo/algebra/ZMod.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 using Mint= ZMod<int(1e9 + 7)>;
 vector alp= {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
 string N;
 cin >> N;
 int n= N.length();
 auto tr_prod= [](int s, int c) {
  if(s == -1) return c ? c : -1;
  if(!c) return -2;
  return s * c % 100;
 };
 DFA dfa_prod(alp, -1, tr_prod, [](int s) { return s == 0; }, -2);
 auto dfa= leq_digits(N) & dfa_prod;
 cout << dfa.count<Mint>(n) << '\n';
 return 0;
}
