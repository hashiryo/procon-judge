// lib.cpp と同じ DP を NeoLibrary の DFA (neo/counting/DFA.hpp) で回す。N 以下の DFA は、上の桁から読む 60 桁の leq_digits にする。
#include <iostream>
#include <vector>
#include <array>
#include "neo/algebra/ZMod.hpp"
#include "neo/counting/DFA.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 using Mint= ZMod<int(1e9 + 7)>;
 int64_t N;
 cin >> N;
 auto dfa= leq_digits(N, 2, 60);
 using T= array<Mint, 4>;
 auto op= [&](const T& l, const T& r) {
  T ret;
  for(int i= 4; i--;) ret[i]= l[i] + r[i];
  return ret;
 };
 auto f= [&](T x, int c, int) {
  x[3]+= x[3], x[1]+= x[1];
  if(c) x[3]+= x[1] + x[2] + x[0], x[1]+= x[0], x[2]+= x[0];
  return x;
 };
 cout << dfa.dp(60, op, T{0, 0, 0, 0}, f, T{1, 0, 0, 0})[3] << '\n';
 return 0;
}
