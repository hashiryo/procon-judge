// lib.cpp と同じ DP を NeoLibrary の DFA (neo/counting/DFA.hpp) で回す。A 以上 B 以下は、上の桁から読む 40 桁の geq_digits と leq_digits にする。
#include <iostream>
#include <vector>
#include "neo/counting/DFA.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 long long A, B;
 cin >> A >> B;
 auto dfa= geq_digits(A, 2, 40) & leq_digits(B, 2, 40);
 using T= pair<bool, long long>;
 auto op= [](const T& l, const T& r) { return T{l.first ^ r.first, l.second ^ r.second}; };
 auto f= [](T x, int c, int) {
  x.second<<= 1;
  if(c) x.second|= x.first;
  return x;
 };
 cout << dfa.dp(40, op, T{0, 0}, f, T{1, 0}).second << '\n';
 return 0;
}
