#pragma once
// push-relabel (Cherkassky, Goldberg, Martin, Setubal, Stolfi の double push)。右の頂点に、空いている右の頂点までの
// 残余グラフでの距離の下界 psi を持つ。空いている左の頂点 u を FIFO で取り出し、psi が最小の隣 r と組ませ、r の元の
// 相手を空けて FIFO に戻す。r の psi は、u の隣のうち r を除いた psi の最小に 2 を足した値に上げる (u から r 以外へ
// 戻る道の長さ)。psi が上限 (2R) に達した右の頂点からは空いている右の頂点に届かないので、隣がすべて上限の左の頂点は
// 捨てる。L + R 回組ませるごとに、空いている右の頂点から逆向きの BFS で psi を正確な距離に直す (global relabel)。
// 最初に、左の頂点を順に見て空いている右の隣と組ませる貪欲なマッチングを作る。
#include <algorithm>
#include <array>
#include <vector>
namespace bm_pr {
// 左の頂点ごとの相手 (右の頂点、いなければ -1) を返す。
inline std::vector<int> bipartite_matching(int L, int R, const std::vector<std::array<int, 2>>& e) {
 const int M= e.size(), INF= 2 * R;
 std::vector<int> sl(L + 1), al(M), sr(R + 1), ar(M);
 for(auto& x: e) ++sl[x[0]], ++sr[x[1]];
 for(int i= 0; i < L; ++i) sl[i + 1]+= sl[i];
 for(int i= 0; i < R; ++i) sr[i + 1]+= sr[i];
 for(int i= M; i--;) al[--sl[e[i][0]]]= e[i][1], ar[--sr[e[i][1]]]= e[i][0];
 std::vector<int> ml(L, -1), mr(R, -1), psi(R), q(L), bq(R);
 for(int l= 0; l < L; ++l)
  for(int k= sl[l]; k < sl[l + 1]; ++k)
   if(const int r= al[k]; mr[r] < 0) {
    ml[l]= r, mr[r]= l;
    break;
   }
 auto global_relabel= [&] {
  int h= 0, t= 0;
  for(int r= 0; r < R; ++r)
   if(mr[r] < 0) psi[r]= 0, bq[t++]= r;
   else psi[r]= INF;
  while(h < t) {
   const int r= bq[h++], d= psi[r] + 2;
   for(int k= sr[r]; k < sr[r + 1]; ++k)
    if(const int r2= ml[ar[k]]; r2 >= 0 && psi[r2] == INF) psi[r2]= d, bq[t++]= r2;
  }
 };
 global_relabel();
 // 空いている左の頂点は同時に L 個を超えないので、長さ L の輪で FIFO を持つ。
 int qh= 0, qn= 0;
 for(int l= 0; l < L; ++l)
  if(ml[l] < 0) q[qn++]= l;
 const int freq= L + R;
 for(int cnt= 0; qn;) {
  const int u= q[qh];
  if(++qh == L) qh= 0;
  --qn;
  int m1= INF, m2= INF, r1= -1;
  for(int k= sl[u]; k < sl[u + 1]; ++k) {
   const int r= al[k], p= psi[r];
   if(p < m1) m2= m1, m1= p, r1= r;
   else if(p < m2) m2= p;
  }
  if(m1 >= INF) continue;
  const int w= mr[r1];
  ml[u]= r1, mr[r1]= u, psi[r1]= std::min(m2 + 2, INF);
  if(w >= 0) {
   ml[w]= -1;
   int t= qh + qn;
   if(t >= L) t-= L;
   q[t]= w, ++qn;
  }
  if(++cnt == freq) cnt= 0, global_relabel();
 }
 return ml;
}
}
struct Solver {
 int L, R;
 const vector<array<int, 2>>& e;
 vector<int> ml;
 Solver(int l, int r, const vector<array<int, 2>>& edges): L(l), R(r), e(edges) {}
 void run() { ml= bm_pr::bipartite_matching(L, R, e); }
 vector<array<int, 2>> answer() const {
  vector<array<int, 2>> ret;
  for(int l= 0; l < L; ++l)
   if(ml[l] >= 0) ret.push_back({l, ml[l]});
  return ret;
 }
};
