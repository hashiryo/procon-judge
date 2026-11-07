#pragma once
// cf-147-b の愚直解 (提出ではない)。ちょうど k 歩の歩みの重みの最大 D_k を k = 1, 2, ..., n と伸ばし (D_k = D_{k-1} W)、対角に
// 正の値が初めて出た k を答える。対角に 0 を足した行列の 2 の冪も、上の bit から決める手順も使わないので、提出とは別の考え方に
// なる。正の閉じた歩みがあれば、それを単純な閉路に分けたうちの 1 つが正で、長さは n 以下なので、n までで足りる。
// n ≤ 9 程度の小さい入力でだけ使う (pj testdata crosscheck --brute brute.hpp)。
#include "common.hpp"
inline int solve(int n, const vector<array<int, 4>>& doors) {
 constexpr long long NEG= LLONG_MIN / 4;
 vector<vector<long long>> w(n, vector<long long>(n, NEG)), d;
 for(auto [i, j, c, e]: doors) w[i][j]= max<long long>(w[i][j], c), w[j][i]= max<long long>(w[j][i], e);
 d= w;
 for(int k= 1; k <= n; ++k) {
  if(k >= 2)
   for(int s= 0; s < n; ++s)
    if(d[s][s] != NEG && d[s][s] > 0) return k;
  vector<vector<long long>> e(n, vector<long long>(n, NEG));
  for(int s= 0; s < n; ++s)
   for(int u= 0; u < n; ++u) {
    if(d[s][u] == NEG) continue;
    for(int v= 0; v < n; ++v)
     if(w[u][v] != NEG) e[s][v]= max(e[s][v], d[s][u] + w[u][v]);
   }
  d.swap(e);
 }
 return 0;
}
