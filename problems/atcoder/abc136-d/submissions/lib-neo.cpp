// lib.cpp を NeoLibrary の FunctionalGraph で書いたもの。K = 10^100 は閉路までの距離 d より大きいので、f^K(i) は f^{d + (K - d) mod L}(i)
// (L は閉路の長さ)。10^100 mod L は累乗で求める。
#include <iostream>
#include <string>
#include <vector>
#include "neo/base/ds/tree/FunctionalGraph.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 string S;
 cin >> S;
 int N= S.length();
 vector<int> to(N);
 for(int i= 0; i < N; ++i) to[i]= S[i] == 'L' ? i - 1 : i + 1;
 FunctionalGraph g(to);
 auto pow10_mod= [](long long L) {  // 10^100 mod L
  long long r= 1 % L, b= 10 % L;
  for(int e= 100; e; e>>= 1, b= b * b % L)
   if(e & 1) r= r * b % L;
  return r;
 };
 vector cnt(N, 0);
 for(int i= 0; i < N; ++i) {
  const long long d= g.dist_to_cycle(i), L= g.cycle_length(i);
  ++cnt[g.jump(i, d + ((pow10_mod(L) - d % L) % L + L) % L)];
 }
 for(int i= 0; i < N; ++i) cout << cnt[i] << " \n"[i == N - 1];
 return 0;
}
