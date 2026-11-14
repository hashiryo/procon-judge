// lib-neo.cpp の DFA を、状態に番号を振る表を密な配列にした写し (problems/_shared/automaton/_DFA_dn.hpp) に替えたもの。
// 書き比べの記録は algo-notes の notes/automaton-dp.md にある。以下は lib-neo.cpp のまま。
// lib.cpp と同じ 2 つの DFA の積を NeoLibrary の DFA (neo/counting/DFA.hpp) で数える。S より辞書順で小さい文字列の DFA は
// 手で書いたまま。
#include <iostream>
#include <vector>
#include <string>
#include "_shared/automaton/_DFA_dn.hpp"
#include "neo/algebra/ZMod.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 using Mint= ZMod<998244353>;
 vector<char> alp(26);
 for(int i= 0; i < 26; ++i) alp[i]= 'a' + i;
 int N;
 cin >> N;
 string S;
 cin >> S;
 auto tr_le= [&](int s, char c) {
  if(s >= N) return s;
  if(c < S[s]) return N + 1;
  if(c > S[s]) return N;
  return s + 1;
 };
 DFA dfa_le(alp, 0, tr_le, [&](int) { return true; }, N);
 auto tr_a= [&](int s, char c) {
  if(c == 'a') ++s;
  return s;
 };
 DFA dfa_1(alp, 0, tr_a, [](int s) { return s == 1; }, 2);
 auto dfa= dfa_le & dfa_1;
 cout << dfa.count<Mint>(N) << '\n';
 return 0;
}
