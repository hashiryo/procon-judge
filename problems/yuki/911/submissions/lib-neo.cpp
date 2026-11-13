// lib.cpp と同じ 3 つの DFA の積を NeoLibrary の DFA (neo/counting/DFA.hpp) で数える。L 以上 R 以下は、上の桁から読む
// 61 桁の geq_digits と leq_digits にする。
#include <iostream>
#include <vector>
#include <algorithm>
#include "neo/counting/DFA.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N;
 cin >> N;
 int64_t L, R;
 cin >> L >> R;
 vector<int64_t> A(N);
 for(int i= 0; i < N; ++i) cin >> A[i];
 vector alp= {0, 1};
 auto dfa_l= geq_digits(L, 2, 61), dfa_r= leq_digits(R, 2, 61);
 using state_t= pair<vector<int>, int>;
 state_t rej= make_pair(vector<int>(), -1);
 state_t init= make_pair(vector<int>{0, N}, 60);
 auto tr_x= [&](const state_t& s, int c) -> state_t {
  const auto& [sep, i]= s;
  if(i <= -1) return s;
  const int n= sep.size();
  vector<int> nsep{0};
  for(int j= 0; j + 1 < n; ++j) {
   int bg= sep[j], ed= sep[j + 1];
   for(int k= bg; k + 1 < ed; ++k) {
    int l= ((A[k] >> i) & 1) ^ c, r= ((A[k + 1] >> i) & 1) ^ c;
    if(l > r) return rej;
    if(l < r) nsep.push_back(k + 1);
   }
   nsep.push_back(ed);
  }
  return {nsep, i - 1};
 };
 auto ac_x= [&](const state_t& s) {
  const auto& [sep, i]= s;
  return sep.size() == size_t(N + 1);
 };
 DFA dfa_x(alp, init, tr_x, ac_x, rej);
 auto dfa= dfa_l & dfa_r & dfa_x;
 cout << dfa.count<int64_t>(61) << '\n';
 return 0;
}
