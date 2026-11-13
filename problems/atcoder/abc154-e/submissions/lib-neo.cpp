// lib.cpp と同じ数え上げを NeoLibrary の DFA (neo/counting/DFA.hpp) で行う。N 以下の DFA は leq_digits にする。
#include <iostream>
#include <vector>
#include <string>
#include "neo/counting/DFA.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 string N;
 int K;
 cin >> N >> K;
 int n= N.length();
 vector alp= {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
 auto tr_0= [&](int s, int c) { return s + bool(c); };
 auto ac_0= [&](int s) { return s == K; };
 DFA dfa_0(alp, 0, tr_0, ac_0, K + 1);
 auto dfa= leq_digits(N) & dfa_0;
 cout << dfa.count<int>(n) << '\n';
 return 0;
}
