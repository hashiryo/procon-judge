// lib-neo.cpp の DFA を、積の来られる組が少ないときに積を表にして進める写し (problems/_shared/automaton/_DFA_pt.hpp)
// に替えたもの。書き比べの記録は algo-notes の notes/automaton-dp.md にある。以下は lib-neo.cpp のまま。
// lib.cpp と同じ DP を NeoLibrary の DFA (neo/counting/DFA.hpp) で回す。N 以下の DFA は leq_digits にする。
#include <iostream>
#include <vector>
#include <string>
#include <array>
#include "_shared/automaton/_DFA_pt.hpp"
#include "neo/algebra/ZMod.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 using Mint= ZMod<998244353>;
 string N;
 int M;
 cin >> N >> M;
 int n= N.length();
 int c= 0;
 for(int i= 0; i < M; i++) {
  int C;
  cin >> C, c|= 1 << C;
 }
 std::vector<int> alp= {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
 auto tr1= [&](int s, int q) { return s | ((q || s) << q); };
 auto ac1= [&](int s) { return (s & c) == c; };
 DFA dfa_variety(alp, 0, tr1, ac1);
 auto dfa= leq_digits(N) & dfa_variety;
 using T= array<Mint, 2>;
 auto op= [](const T& l, const T& r) { return T{l[0] + r[0], l[1] + r[1]}; };
 auto f= [](const T& v, int a, int) { return T{v[0], v[1] * 10 + v[0] * a}; };
 cout << dfa.dp(n, op, T{0, 0}, f, T{1, 0})[1] << '\n';
 return 0;
}
