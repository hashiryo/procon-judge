// 記号を服、状態を (日, 前の日の服) にした NeoLibrary の DFA (neo/counting/DFA.hpp) で、派手さの差の和の最大を求める。
// 足す量が前の服で決まるので、dp の f に遷移の前後の状態 (Step) を受けさせ、移る前の状態から前の服を引く。
#include <iostream>
#include <vector>
#include <utility>
#include <climits>
#include <cstdlib>
#include "neo/counting/DFA.hpp"
using namespace std;
signed main() {
 int D, N;
 cin >> D >> N;
 vector<int> T(D), A(N), B(N), C(N);
 for(auto& t: T) cin >> t;
 for(int j= 0; j < N; ++j) cin >> A[j] >> B[j] >> C[j];
 vector<int> alp(N);
 for(int j= 0; j < N; ++j) alp[j]= j;
 using state_t= pair<int, int>;
 auto tr= [&](state_t s, int c) -> state_t {
  if(s.first >= D || T[s.first] < A[c] || B[c] < T[s.first]) return {-1, -1};
  return {s.first + 1, c};
 };
 DFA<int> d(alp, state_t{0, -1}, tr, [&](state_t s) { return s.first == D; }, state_t{-1, -1});
 auto f= [&](int v, int c, int, const DFA<int>::Step& t) {
  const int prev= t.from.get<state_t>().second;
  return prev < 0 ? v : v + abs(C[prev] - C[c]);
 };
 cout << d.dp(D, [](int a, int b) { return max(a, b); }, INT_MIN, f, 0) << '\n';
 return 0;
}
