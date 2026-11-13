// lib.cpp と同じ 3 つの DFA の積を NeoLibrary の DFA (neo/counting/DFA.hpp) で数える。y ≤ R と x ≥ L は、下の桁から読む
// leq_digits_lsb と geq_digits_lsb を、記号 (x の桁, y の桁) の片方を読む map_symbols で写して表す。
#include <iostream>
#include <vector>
#include <array>
#include "neo/counting/DFA.hpp"
#include "neo/algebra/ZMod.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using Mint= ZMod<int(1e9 + 7)>;
 int64_t L, R;
 cin >> L >> R;
 using symbol_t= array<bool, 2>;
 vector<symbol_t> alp= {{0, 0}, {0, 1}, {1, 1}};
 auto dfa_le= leq_digits_lsb(R, 2).map_symbols(alp, [](symbol_t c) { return int(c[1]); });
 auto dfa_ge= geq_digits_lsb(L, 2).map_symbols(alp, [](symbol_t c) { return int(c[0]); });
 auto tr_len= [](bool s, symbol_t c) {
  if(c[0] == 1 && c[1] == 1) s= 1;
  if(c[0] != c[1]) s= 0;
  return s;
 };
 auto ac_len= [](bool s) { return s; };
 DFA dfa_len(alp, true, tr_len, ac_len);
 auto dfa= dfa_le & dfa_ge & dfa_len;
 cout << dfa.count<Mint>(60) << '\n';
 return 0;
}
