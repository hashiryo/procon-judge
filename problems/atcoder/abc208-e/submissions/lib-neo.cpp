// lib.cpp と同じ数え上げを NeoLibrary の DFA (neo/counting/DFA.hpp) で行う。N 以下の DFA は leq_digits にし、桁の数が
// N より少ない数は count_all の 1 回でまとめて数える。
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include "neo/counting/DFA.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 string N;
 int K;
 cin >> N >> K;
 int n= N.length();
 std::vector<int> alp= {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
 auto tr_k= [&](int s, int q) -> int {
  auto p= (int64_t)s * q;
  if(p > K) return K + 1;
  return p;
 };
 auto ac_k= [&](int s) { return s <= K; };
 DFA dfa_k(alp, 1, tr_k, ac_k);
 auto cnt= dfa_k.count_all<int64_t>(n - 1);
 int64_t ans= 0, pw= 1;
 for(int i= 1; i < n; ++i, pw*= 10) ans+= cnt[i] - pw;
 ans+= (leq_digits(N) & dfa_k).count<int64_t>(n) - pw;
 cout << ans << '\n';
 return 0;
}
