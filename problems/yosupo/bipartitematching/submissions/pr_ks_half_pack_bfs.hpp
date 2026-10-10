#pragma once
// pr_ks_half_pack の前に、頂点の番号を BFS の順に振り直したもの。左の頂点を番号の順に根にして、左から右、右から
// 左へ広げ、見つけた順に新しい番号を付けて隣接を組み直す。入力の番号は乱択で振られていることが多く、そのままだと
// 隣の頂点の位置が配列の中でばらばらになる。組み直しの手間は global relabel 2 回ほど。
// 以下は pr_ks_half_pack と同じ。pr の最初のマッチングを Karp-Sipser にし、global relabel を (L + R) / 2 回ごとに
// したもの。右の頂点の psi と相手を 1 つの配列に並べ、double push で psi が最小の隣の相手を読むときに同じキャッシュ
// ラインに当たるようにする。
// push-relabel (Cherkassky, Goldberg, Martin, Setubal, Stolfi の double push)。右の頂点に、空いている右の頂点までの
// 残余グラフでの距離の下界 psi を持つ。空いている左の頂点 u を FIFO で取り出し、psi が最小の隣 r と組ませ、r の元の
// 相手を空けて FIFO に戻す。r の psi は、u の隣のうち r を除いた psi の最小に 2 を足した値に上げる (u から r 以外へ
// 戻る道の長さ)。psi が上限 (2R) に達した右の頂点からは空いている右の頂点に届かないので、隣がすべて上限の左の頂点は
// 捨てる。(L + R) / 2 回組ませるごとに、空いている右の頂点から逆向きの BFS で psi を正確な距離に直す (global relabel)。
// 最初のマッチングは Karp-Sipser で作る。空いている隣が 1 つだけになった頂点 (左右どちらでも) を先にその隣と組ませ、
// そういう頂点が無くなったら、空いている隣を持つ左の頂点のうち番号の小さいものを最初の空いている隣と組ませる。
// 次数 1 の頂点を先に組ませても最大マッチングの大きさは変わらないので、鎖や木に近い部分はこれだけで解ける。
#include <algorithm>
#include <array>
#include <vector>
namespace bm_pr_ks_half_pack_bfs {
// 左の頂点ごとの相手 (右の頂点、いなければ -1) を返す。
inline std::vector<int> bipartite_matching(int L, int R, const std::vector<std::array<int, 2>>& e) {
 const int M= e.size(), INF= 2 * R;
 std::vector<int> sl(L + 1), al(M), sr(R + 1), ar(M);
 for(auto& x: e) ++sl[x[0]], ++sr[x[1]];
 for(int i= 0; i < L; ++i) sl[i + 1]+= sl[i];
 for(int i= 0; i < R; ++i) sr[i + 1]+= sr[i];
 for(int i= M; i--;) al[--sl[e[i][0]]]= e[i][1], ar[--sr[e[i][1]]]= e[i][0];
 // 頂点の番号を BFS の順に振り直す。ol と orr は新しい番号から元の番号へ。
 std::vector<int> ol(L), orr(R);
 {
  std::vector<int> nl(L, -1), nr(R, -1);
  int cl= 0, cr= 0, head= 0;
  for(int s0= 0; s0 < L; ++s0) {
   if(nl[s0] >= 0) continue;
   nl[s0]= cl, ol[cl++]= s0;
   while(head < cl) {
    const int u= ol[head++];
    for(int k= sl[u]; k < sl[u + 1]; ++k)
     if(const int r= al[k]; nr[r] < 0) {
      nr[r]= cr, orr[cr++]= r;
      for(int j= sr[r]; j < sr[r + 1]; ++j)
       if(const int w= ar[j]; nl[w] < 0) nl[w]= cl, ol[cl++]= w;
     }
   }
  }
  for(int r= 0; r < R; ++r)
   if(nr[r] < 0) nr[r]= cr, orr[cr++]= r;
  std::vector<int> sl2(L + 1), al2(M), sr2(R + 1), ar2(M);
  for(int i= 0, p= 0; i < L; ++i) {
   const int u= ol[i];
   for(int k= sl[u]; k < sl[u + 1]; ++k) al2[p++]= nr[al[k]];
   sl2[i + 1]= p;
  }
  for(int j= 0, p= 0; j < R; ++j) {
   const int r= orr[j];
   for(int k= sr[r]; k < sr[r + 1]; ++k) ar2[p++]= nl[ar[k]];
   sr2[j + 1]= p;
  }
  sl.swap(sl2), al.swap(al2), sr.swap(sr2), ar.swap(ar2);
 }
 std::vector<int> ml(L, -1), mr(R, -1), q(L), bq(R);
 {
  // Karp-Sipser。dl, dr は空いている隣の数。次数が 1 に落ちた頂点を stk に積む (右の頂点 r は L + r で積む)。
  // 次数は減るだけで 1 になるのは各頂点で一度なので、stk は L + R で足りる。
  std::vector<int> dl(L), dr(R), stk(L + R);
  int sp= 0;
  for(int l= 0; l < L; ++l)
   if((dl[l]= sl[l + 1] - sl[l]) == 1) stk[sp++]= l;
  for(int r= 0; r < R; ++r)
   if((dr[r]= sr[r + 1] - sr[r]) == 1) stk[sp++]= L + r;
  auto take= [&](int l, int r) {
   ml[l]= r, mr[r]= l;
   for(int k= sl[l]; k < sl[l + 1]; ++k)
    if(const int r2= al[k]; mr[r2] < 0 && --dr[r2] == 1) stk[sp++]= L + r2;
   for(int k= sr[r]; k < sr[r + 1]; ++k)
    if(const int l2= ar[k]; ml[l2] < 0 && --dl[l2] == 1) stk[sp++]= l2;
  };
  for(int cur= 0;;) {
   while(sp) {
    const int x= stk[--sp];
    if(x < L) {
     if(ml[x] >= 0) continue;
     for(int k= sl[x]; k < sl[x + 1]; ++k)
      if(const int r= al[k]; mr[r] < 0) {
       take(x, r);
       break;
      }
    } else {
     const int r= x - L;
     if(mr[r] >= 0) continue;
     for(int k= sr[r]; k < sr[r + 1]; ++k)
      if(const int l= ar[k]; ml[l] < 0) {
       take(l, r);
       break;
      }
    }
   }
   while(cur < L && (ml[cur] >= 0 || dl[cur] == 0)) ++cur;
   if(cur == L) break;
   for(int k= sl[cur]; k < sl[cur + 1]; ++k)
    if(const int r= al[k]; mr[r] < 0) {
     take(cur, r);
     break;
    }
  }
 }
 // ここからは右の頂点の psi と相手を rv に並べて持つ。
 struct Rv {
  int psi, mate;
 };
 std::vector<Rv> rv(R);
 for(int r= 0; r < R; ++r) rv[r].mate= mr[r];
 auto global_relabel= [&] {
  int h= 0, t= 0;
  for(int r= 0; r < R; ++r)
   if(rv[r].mate < 0) rv[r].psi= 0, bq[t++]= r;
   else rv[r].psi= INF;
  while(h < t) {
   const int r= bq[h++], d= rv[r].psi + 2;
   for(int k= sr[r]; k < sr[r + 1]; ++k)
    if(const int r2= ml[ar[k]]; r2 >= 0 && rv[r2].psi == INF) rv[r2].psi= d, bq[t++]= r2;
  }
 };
 global_relabel();
 // 空いている左の頂点は同時に L 個を超えないので、長さ L の輪で FIFO を持つ。
 int qh= 0, qn= 0;
 for(int l= 0; l < L; ++l)
  if(ml[l] < 0) q[qn++]= l;
 const int freq= std::max(1, (L + R) / 2);
 for(int cnt= 0; qn;) {
  const int u= q[qh];
  if(++qh == L) qh= 0;
  --qn;
  int m1= INF, m2= INF, r1= -1;
  for(int k= sl[u]; k < sl[u + 1]; ++k) {
   const int r= al[k], p= rv[r].psi;
   if(p < m1) m2= m1, m1= p, r1= r;
   else if(p < m2) m2= p;
  }
  if(m1 >= INF) continue;
  const int w= rv[r1].mate;
  ml[u]= r1, rv[r1]= {std::min(m2 + 2, INF), u};
  if(w >= 0) {
   ml[w]= -1;
   int t= qh + qn;
   if(t >= L) t-= L;
   q[t]= w, ++qn;
  }
  if(++cnt == freq) cnt= 0, global_relabel();
 }
 std::vector<int> res(L, -1);
 for(int i= 0; i < L; ++i)
  if(ml[i] >= 0) res[ol[i]]= orr[ml[i]];
 return res;
}
}
struct Solver {
 int L, R;
 const vector<array<int, 2>>& e;
 vector<int> ml;
 Solver(int l, int r, const vector<array<int, 2>>& edges): L(l), R(r), e(edges) {}
 void run() { ml= bm_pr_ks_half_pack_bfs::bipartite_matching(L, R, e); }
 vector<array<int, 2>> answer() const {
  vector<array<int, 2>> ret;
  for(int l= 0; l < L; ++l)
   if(ml[l] >= 0) ret.push_back({l, ml[l]});
  return ret;
 }
};
