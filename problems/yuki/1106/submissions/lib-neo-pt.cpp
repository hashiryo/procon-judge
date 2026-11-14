// lib-neo.cpp の DFA を、積の来られる組が少ないときに積を表にして進める写し (problems/_shared/automaton/_DFA_pt.hpp)
// に替えたもの。書き比べの記録は algo-notes の notes/automaton-dp.md にある。以下は lib-neo.cpp のまま。
// lib.cpp と同じ 3 つの DFA の積を NeoLibrary の DFA (neo/counting/DFA.hpp) で数える。桁が -2 から 2 の 5 進なので、
// N 以下の DFA は手で書いたまま。
#include <iostream>
#include <vector>
#include <cstdlib>
#include "_shared/automaton/_DFA_pt.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 vector alp= {-2, -1, 0, 1, 2};
 int64_t N;
 cin >> N;
 auto tr_le= [&](int64_t s, int c) { return (s - c + 5) / 5 - 1; };
 DFA dfa_le(alp, N, tr_le, [](int64_t) { return true; }, int64_t(-1));
 auto tr_pos= [&](bool s, int c) {
  if(c > 0) s= 1;
  else if(c < 0) s= 0;
  return s;
 };
 DFA dfa_pos(alp, false, tr_pos, [](bool s) { return s; });
 auto tr_eq= [&](int s, int c) {
  if(abs(s) > 100) return s;
  return s + c;
 };
 DFA dfa_eq(alp, 0, tr_eq, [](int s) { return s == 0; });
 auto dfa= dfa_le & dfa_pos & dfa_eq;
 cout << dfa.count<int64_t>(60) << '\n';
 return 0;
}
