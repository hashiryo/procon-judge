// lib.cpp と同じ DP を NeoLibrary の DFA (neo/counting/DFA.hpp) で回す。N 以下の DFA は、下の桁から読む leq_digits_lsb に
// し、長さ 1 から 60 の答えは dp_all の 1 回でまとめて求める。
#include <iostream>
#include <vector>
#include <array>
#include "neo/counting/DFA.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int64_t N;
 cin >> N;
 auto dfa= leq_digits_lsb(N, 10);
 using T= array<int64_t, 2>;
 auto op= [](const T& l, const T& r) { return T{l[0] + r[0], l[1] + r[1]}; };
 auto f= [](T x, int c, int) {
  if(c == 1) x[1]+= x[0];
  else x[1]= 0;
  return x;
 };
 auto res= dfa.dp_all(60, op, T{0, 0}, f, T{1, 0});
 int64_t ans= 0;
 for(int i= 1; i <= 60; ++i) ans+= res[i][1];
 cout << ans << '\n';
 return 0;
}
