// 先頭が 0 でない数字の列の、NeoLibrary の DFA (neo/counting/DFA.hpp) の ranker を 1 回作り、テストケースごとに長さごとの
// count_leq と count_less で、辞書順で K 以下かつ値が N 以下の数を数える。
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include "neo/counting/DFA.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 DFA<int> d({0, 1, 2, 3, 4, 5, 6, 7, 8, 9}, 0, [](int s, int c) { return s == 0 && c == 0 ? -1 : 1; }, [](int s) { return s == 1; }, -1);
 auto R= d.ranker<unsigned long long>(18);
 int T;
 cin >> T;
 while(T--) {
  string Ns, Ks;
  cin >> Ns >> Ks;
  vector<int> N, K;
  for(char ch: Ns) N.push_back(ch - '0');
  for(char ch: Ks) K.push_back(ch - '0');
  const int n= N.size(), k= K.size();
  unsigned long long ans= 0;
  for(int l= 1; l <= n; ++l) {
   if(l <= k) {  // 長さ l の x が辞書順で K 以下 ⟺ x ≤ K の先頭 l 桁
    vector<int> B(K.begin(), K.begin() + l);
    if(l == n) B= min(B, N);
    ans+= R.count_leq(B);
   } else {  // x の先頭 k 桁が K より小さい ⟺ x < K の後ろに 0 を足したもの
    vector<int> B= K;
    B.resize(l, 0);
    if(l == n && N < B) ans+= R.count_leq(N);
    else ans+= R.count_less(B);
   }
  }
  cout << ans << '\n';
 }
 return 0;
}
