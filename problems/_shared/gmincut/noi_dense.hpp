#pragma once
// 全域最小カットの書き比べの核。Nagamochi-Ono-Ibaraki の方法 (noi.hpp と同じ手順) を、隣接行列で回す形。最大の r を持つ頂点を
// 毎回なめて選ぶので、1 回のなめは O(k^2) (k は今の頂点の数)。行列は n × n のまま持ち、まとめた頂点は代表の行と列に足して、
// 生きている頂点の列から外す (sw_dense と同じ)。Stoer-Wagner の行列の形と 1 回の手間は同じで、1 回に縮める組がそれ以上に
// なる。PR なら、なめる前に Padberg-Rinaldi の 2 つの条件 (重みが λ̂ 以上、2 c(u, v) > min(deg u, deg v)) で縮める。
#include <algorithm>
#include <array>
#include <vector>
namespace gmc {
template <bool PR> long long noi_dense(int n, const std::vector<std::array<long long, 3>>& edges) {
 if(n <= 1) return 0;
 std::vector<long long> w((size_t)n * n, 0), deg(n, 0);
 for(auto& [u, v, c]: edges)
  if(u != v) w[u * n + v]+= c, w[v * n + u]+= c, deg[u]+= c, deg[v]+= c;
 long long lam= *std::min_element(deg.begin(), deg.end());
 std::vector<int> par(n), alive(n);
 for(int v= 0; v < n; ++v) par[v]= v, alive[v]= v;
 auto find= [&](int x) {
  while(par[x] != x) x= par[x]= par[par[x]];
  return x;
 };
 // par でまとめた頂点を代表へ縮める。まとめられた頂点 x の行を代表の行に足し、代表の行の中で、まとめられた頂点の列を代表の
 // 列へ寄せる。代表 A の次数は、A の頂点の次数の和から、A の中の重みの 2 倍 (寄せたあとの自分の列) を引いたもの。手間は
 // まとめられた頂点の数 × k。
 auto contract= [&]() {
  std::vector<int> keep, gone;
  for(int v: alive) (find(v) == v ? keep : gone).push_back(v);
  for(int x: gone) {
   const int a= find(x);
   long long* ra= &w[(size_t)a * n];
   const long long* rx= &w[(size_t)x * n];
   for(int y: alive) ra[y]+= rx[y];
   deg[a]+= deg[x];
  }
  for(int a: keep) {
   long long* ra= &w[(size_t)a * n];
   for(int y: gone) ra[find(y)]+= ra[y], ra[y]= 0;
   deg[a]-= ra[a], ra[a]= 0;
  }
  alive.swap(keep);
  if(alive.size() > 1)
   for(int v: alive) lam= std::min(lam, deg[v]);
 };
 std::vector<long long> r(n);
 std::vector<char> done(n);
 while(alive.size() > 1 && lam > 0) {
  const int k= alive.size();
  if constexpr(PR) {
   bool any= false;
   for(int i= 0; i < k; ++i)
    for(int j= i + 1; j < k; ++j) {
     const int u= alive[i], v= alive[j];
     if(const long long c= w[(size_t)u * n + v]; c > 0 && (c >= lam || 2 * c > std::min(deg[u], deg[v]))) {
      const int a= find(u), b= find(v);
      if(a != b) par[a]= b, any= true;
     }
    }
   if(any) {
    contract();
    continue;
   }
  }
  for(int v: alive) r[v]= 0, done[v]= 0;
  long long cut= 0;
  int prev= -1, last= -1;
  bool any= false;
  for(int step= 0; step < k; ++step) {
   int v= -1;
   for(int x: alive)
    if(!done[x] && (v < 0 || r[x] > r[v])) v= x;
   if(v < 0) break;  // 起きない (未訪問の頂点は残っている)。gcc の -Wstringop-overflow の誤検知を消す
   done[v]= 1, prev= last, last= v;
   cut+= deg[v] - 2 * r[v];
   if(step + 1 < k && cut < lam) lam= cut;
   const long long* row= &w[(size_t)v * n];
   for(int x: alive) {
    if(done[x] || row[x] == 0) continue;
    if(r[x] < lam && r[x] + row[x] >= lam) {
     const int a= find(v), b= find(x);
     if(a != b) par[a]= b, any= true;
    }
    r[x]+= row[x];
   }
  }
  if(lam == 0) break;
  if(!any) par[find(last)]= find(prev);
  contract();
 }
 return lam;
}
}  // namespace gmc
