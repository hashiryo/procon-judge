// 門松数の DFA と「先頭が 0 でない」DFA の積を上の桁から読み、NeoLibrary の DFA (neo/counting/DFA.hpp) の ranker を 1 回作る。
// クエリごとに 3 桁から長さごとの数を引いて長さを決め、kth で K 番目を作る (lib-neo の二分探索と count の代わり)。
#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include "neo/counting/DFA.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 vector alp= {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
 using state_t= array<int, 2>;
 auto tr_kado= [&](state_t s, int c) -> state_t {
  if(s[0] == -1) {
   if(s[1] == c) return {-2, -2};
   else return {s[1], c};
  }
  if(s[0] == c || s[1] == c) return {-2, -2};
  auto [mi, mx]= minmax({s[0], s[1], c});
  if(s[1] == mi || s[1] == mx) return {s[1], c};
  return {-2, -2};
 };
 DFA dfa_kado(alp, state_t{-1, -1}, tr_kado, [](state_t) { return true; }, state_t{-2, -2});
 DFA dfa_lead(alp, 0, [](int s, int c) { return s == 0 && c == 0 ? -1 : 1; }, [](int s) { return s == 1; }, -1);
 auto R= (dfa_kado & dfa_lead).ranker<long long>(15);
 int T;
 cin >> T;
 while(T--) {
  long long K;
  cin >> K;
  --K;
  int len= 3;
  while(K >= R.count(len)) K-= R.count(len), ++len;
  for(int x: R.kth(K, len)) cout << x;
  cout << '\n';
 }
 return 0;
}
