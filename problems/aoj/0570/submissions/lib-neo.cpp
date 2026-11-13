// lib.cpp と同じオートマトンを NeoLibrary の DFA (neo/counting/DFA.hpp) で組む。A 以上と B 以下は geq_digits と leq_digits にする。
#include <iostream>
#include <vector>
#include <string>
#include <array>
#include "neo/counting/DFA.hpp"
#include "neo/algebra/ZMod.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using Mint= ZMod<10000>;
 string A, B;
 cin >> A >> B;
 int M;
 cin >> M;
 int n= B.length();
 A= string(n - A.length(), '0') + A;
 vector alp= {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
 auto tr_m= [&](int s, int c) { return (s * 10 + c) % M; };
 DFA dfa_m(alp, 0, tr_m, [](int s) { return s == 0; });
 using state_t= array<int, 2>;
 auto tr_zz= [&](state_t s, int c) -> state_t {
  auto [d, t]= s;
  if(d == -2) {
   if(c == 0) return {-2, -2};
   else return {c, -2};
  }
  if(d == c) return {-1, -1};
  if(t == -2) return {c, d < c};
  if(d < c) {
   if(t == 1) return {-1, -1};
   else return {c, 1};
  } else {
   if(t == 0) return {-1, -1};
   else return {c, 0};
  }
  return {-1, -1};
 };
 DFA dfa_zz(alp, state_t{-2, -2}, tr_zz, [&](state_t) { return true; }, state_t{-1, -1});
 auto dfa= geq_digits(A) & leq_digits(B) & dfa_m & dfa_zz;
 cout << dfa.count<Mint>(n) << '\n';
 return 0;
}
