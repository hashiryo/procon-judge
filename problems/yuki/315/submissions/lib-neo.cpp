// lib.cpp と同じ数え上げを NeoLibrary の DFA (neo/counting/DFA.hpp) で行う。A - 1 以下と B 以下の DFA は leq_digits にする。
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include "neo/algebra/ZMod.hpp"
#include "neo/counting/DFA.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using Mint= ZMod<int(1e9 + 7)>;
 string A, B;
 cin >> A >> B;
 int P;
 cin >> P;
 vector alp= {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
 int n_a= A.length(), n_b= B.length();
 for(int i= n_a; i--;) {  // A を A - 1 にする (桁の数は変えない)
  if(A[i] > '0') {
   --A[i];
   break;
  }
  A[i]= '9';
 }
 auto tr_3= [&](int s, int c) {
  if(s == -1 || c == 3) return -1;
  return (s + c) % 3;
 };
 DFA dfa_3(alp, 0, tr_3, [](int s) { return s <= 0; });
 Mint ans= (leq_digits(B) & dfa_3).count<Mint>(n_b) - (leq_digits(A) & dfa_3).count<Mint>(n_a);
 while(P % 10 == 0) P/= 10, --n_a, --n_b;
 auto tr_P= [&](int s, int c) { return (s * 10 + c) % P; };
 DFA dfa_P(alp, 0, tr_P, [](int s) { return s == 0; });
 auto dfa3= leq_digits(A.substr(0, max(n_a, 0))) & dfa_3 & dfa_P;
 auto dfa4= leq_digits(B.substr(0, max(n_b, 0))) & dfa_3 & dfa_P;
 ans-= dfa4.count<Mint>(n_b) - dfa3.count<Mint>(n_a);
 cout << ans << '\n';
 return 0;
}
