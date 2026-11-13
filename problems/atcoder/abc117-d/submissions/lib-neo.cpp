// lib.cpp と同じ DP を NeoLibrary の DFA (neo/counting/DFA.hpp) で回す。K 以下の DFA は、下の桁から読む leq_digits_lsb にする。
#include <iostream>
#include <vector>
#include <algorithm>
#include "neo/counting/DFA.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int64_t N, K;
 cin >> N >> K;
 vector<int64_t> A(N);
 for(int i= 0; i < N; i++) cin >> A[i];
 auto dfa= leq_digits_lsb(K, 2);
 auto op= [](int64_t l, int64_t r) { return max(l, r); };
 auto f= [&](int64_t x, int c, int k) {
  for(int i= N; i--;) x+= (((A[i] >> k) & 1) ^ c) << k;
  return x;
 };
 cout << dfa.dp<int64_t>(60, op, int64_t(0), f, int64_t(0)) << '\n';
 return 0;
}
