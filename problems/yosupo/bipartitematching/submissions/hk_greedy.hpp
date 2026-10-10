#pragma once
// Hopcroft-Karp。段ごとに、空いている左の頂点すべてから BFS で層を作り、空いている右の頂点に最初に届いた層で止める。
// そのあと、層を 1 つずつ下る辺だけを辿る DFS で、頂点を共有しない最短の増加路を流す。DFS は再帰せず、左の頂点ごとに
// 次に見る辺の位置を持ち、行き止まりになった頂点は層から外す。段の数は O(√V) で、全体は O(E√V)。
// hk との違いは、最初の段の前に、左の頂点を順に見て空いている右の隣と組ませる貪欲なマッチングを作ることだけ。
#include <array>
#include <climits>
#include <vector>
namespace bm_hk_greedy {
// 左の頂点ごとの相手 (右の頂点、いなければ -1) を返す。
inline std::vector<int> bipartite_matching(int L, int R, const std::vector<std::array<int, 2>>& e) {
 const int M= e.size(), INF= INT_MAX;
 std::vector<int> st(L + 1), adj(M);
 for(auto& x: e) ++st[x[0]];
 for(int i= 0; i < L; ++i) st[i + 1]+= st[i];
 for(int i= M; i--;) adj[--st[e[i][0]]]= e[i][1];
 std::vector<int> ml(L, -1), mr(R, -1), dist(L), it(L), q(L), stk(L);
 for(int l= 0; l < L; ++l)
  for(int k= st[l]; k < st[l + 1]; ++k)
   if(const int r= adj[k]; mr[r] < 0) {
    ml[l]= r, mr[r]= l;
    break;
   }
 for(;;) {
  int qt= 0;
  for(int l= 0; l < L; ++l)
   if(ml[l] < 0) dist[l]= 0, q[qt++]= l;
   else dist[l]= INF;
  int lim= INF;
  for(int qi= 0; qi < qt; ++qi) {
   const int l= q[qi], d= dist[l];
   if(d >= lim) break;
   for(int k= st[l]; k < st[l + 1]; ++k) {
    const int l2= mr[adj[k]];
    if(l2 < 0) {
     if(lim == INF) lim= d + 1;
    } else if(dist[l2] == INF) dist[l2]= d + 1, q[qt++]= l2;
   }
  }
  if(lim == INF) break;
  for(int l= 0; l < L; ++l) it[l]= st[l];
  for(int l0= 0; l0 < L; ++l0) {
   if(dist[l0] != 0) continue;
   int sp= 0;
   stk[sp++]= l0;
   while(sp) {
    const int l= stk[sp - 1];
    if(it[l] == st[l + 1]) {
     dist[l]= INF;
     if(--sp) ++it[stk[sp - 1]];
     continue;
    }
    const int l2= mr[adj[it[l]]];
    if(l2 < 0) {
     if(dist[l] + 1 == lim) {
      for(int k= 0; k < sp; ++k) {
       const int a= stk[k], b= adj[it[a]];
       ml[a]= b, mr[b]= a;
      }
      break;
     }
     ++it[l];
    } else if(dist[l2] == dist[l] + 1) stk[sp++]= l2;
    else ++it[l];
   }
  }
 }
 return ml;
}
}
struct Solver {
 int L, R;
 const vector<array<int, 2>>& e;
 vector<int> ml;
 Solver(int l, int r, const vector<array<int, 2>>& edges): L(l), R(r), e(edges) {}
 void run() { ml= bm_hk_greedy::bipartite_matching(L, R, e); }
 vector<array<int, 2>> answer() const {
  vector<array<int, 2>> ret;
  for(int l= 0; l < L; ++l)
   if(ml[l] >= 0) ret.push_back({l, ml[l]});
  return ret;
 }
};
