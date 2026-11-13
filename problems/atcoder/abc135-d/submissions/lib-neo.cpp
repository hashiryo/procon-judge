// lib.cpp と同じ 2 つの DFA の積を NeoLibrary の DFA (neo/counting/DFA.hpp) で数える。積は表にしない。
#include <iostream>
#include <vector>
#include <string>
#include "neo/counting/DFA.hpp"
#include "neo/algebra/ZMod.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using Mint= ZMod<int(1e9 + 7)>;
 string S;
 cin >> S;
 int n= S.length();
 vector alp= {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
 auto tr_s= [&](int s, int c) {
  if(S[s] == '?' || S[s] == c + '0') return s + 1;
  return -1;
 };
 auto ac_s= [](int) { return true; };
 DFA dfa_s(alp, 0, tr_s, ac_s, -1);
 auto tr_13= [](int s, int c) { return (s * 10 + c) % 13; };
 auto ac_13= [](int s) { return s == 5; };
 DFA dfa_13(alp, 0, tr_13, ac_13);
 auto dfa= dfa_s & dfa_13;
 cout << dfa.count<Mint>(n) << '\n';
 return 0;
}
