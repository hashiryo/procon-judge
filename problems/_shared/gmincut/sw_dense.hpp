#pragma once
// 全域最小カットの書き比べの核。Stoer-Wagner の方法を隣接行列で回す O(n^3) の形 (よく知られた書き方)。頂点 n 個と無向辺
// {u, v, 重み} の列を受け、最小カットの値を返す。多重辺は行列で足し、自己ループは使わない。つながっていなければ 0。
// 段ごとに、まだ縮めていない頂点を、すでに選んだ集合 A への重みの和が最大のものから順に選び (maximum adjacency の順)、
// 最後の頂点 t のカット ({t} の重みの和、t を縮めた頂点の集合のカット) で答えを更新し、t を 1 つ前の頂点へ縮める。
#include <array>
#include <vector>
namespace gmc {
inline long long sw_dense(int n, const std::vector<std::array<long long, 3>>& edges) {
 std::vector<long long> w((size_t)n * n, 0);
 for(auto& [u, v, c]: edges)
  if(u != v) w[u * n + v]+= c, w[v * n + u]+= c;
 std::vector<int> alive(n);
 for(int i= 0; i < n; ++i) alive[i]= i;
 long long best= -1;
 std::vector<long long> a(n);
 std::vector<char> in(n);
 for(int m= n; m > 1; --m) {
  for(int i= 0; i < m; ++i) a[alive[i]]= 0, in[alive[i]]= 0;
  int prev= -1, last= -1;
  for(int k= 0; k < m; ++k) {
   int sel= -1;
   for(int i= 0; i < m; ++i)
    if(const int v= alive[i]; !in[v] && (sel < 0 || a[v] > a[sel])) sel= v;
   in[sel]= 1;
   if(k == m - 1) {
    last= sel;
    if(best < 0 || a[sel] < best) best= a[sel];
   } else {
    prev= sel;
    const long long* row= &w[(size_t)sel * n];
    for(int i= 0; i < m; ++i) a[alive[i]]+= row[alive[i]];
   }
  }
  // last を prev へ縮める。
  for(int i= 0; i < m; ++i) {
   const int x= alive[i];
   w[(size_t)prev * n + x]+= w[(size_t)last * n + x], w[(size_t)x * n + prev]= w[(size_t)prev * n + x];
  }
  w[(size_t)prev * n + prev]= 0;
  for(int i= 0; i < m; ++i)
   if(alive[i] == last) {
    alive[i]= alive[m - 1];
    break;
   }
 }
 return best < 0 ? 0 : best;
}
}  // namespace gmc
