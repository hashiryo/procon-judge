#pragma once
// Pothen-Fan の DFS に lookahead と fairness を足したもの (Duff, Kaya, Ucar の PF+)。段ごとに、空いている左の頂点から順に
// DFS で増加路を探し、見つけたらすぐ流す。右の頂点は 1 つの段で一度しか訪ねないので、1 段は O(E)。左の頂点に入るたびに、
// まず空いている右の隣が残っていないかを lookahead の位置から先へ探す。右の頂点は一度埋まると空かないので、lookahead の
// 位置は段をまたいで進めるだけでよい。fairness として、隣接を見る向きを段ごとに入れ替える。増加路が 1 本も見つからない
// 段で終わる。最初に、左の頂点を順に見て空いている右の隣と組ませる貪欲なマッチングを作る。DFS は再帰しない。
#include <array>
#include <vector>
namespace bm_pf {
// 左の頂点ごとの相手 (右の頂点、いなければ -1) を返す。
inline std::vector<int> bipartite_matching(int L, int R, const std::vector<std::array<int, 2>>& e) {
 const int M= e.size();
 std::vector<int> st(L + 1), adj(M);
 for(auto& x: e) ++st[x[0]];
 for(int i= 0; i < L; ++i) st[i + 1]+= st[i];
 for(int i= M; i--;) adj[--st[e[i][0]]]= e[i][1];
 std::vector<int> ml(L, -1), mr(R, -1), la(st.begin(), st.end() - 1), it(L), vis(R, 0), stk(L), via(L);
 for(int l= 0; l < L; ++l)
  for(int k= st[l]; k < st[l + 1]; ++k)
   if(const int r= adj[k]; mr[r] < 0) {
    ml[l]= r, mr[r]= l;
    break;
   }
 for(int ph= 1;; ++ph) {
  bool aug= false;
  const bool fwd= ph & 1;
  for(int l0= 0; l0 < L; ++l0) {
   if(ml[l0] >= 0) continue;
   int sp= 0;
   stk[sp++]= l0, it[l0]= fwd ? st[l0] : st[l0 + 1] - 1;
   while(sp) {
    const int l= stk[sp - 1];
    int fr= -1;
    for(int& p= la[l]; p < st[l + 1];)
     if(const int r= adj[p++]; mr[r] < 0) {
      fr= r;
      break;
     }
    if(fr >= 0) {
     for(int k= 0; k + 1 < sp; ++k) ml[stk[k]]= via[k], mr[via[k]]= stk[k];
     ml[l]= fr, mr[fr]= l, vis[fr]= ph;
     aug= true;
     break;
    }
    int r= -1;
    if(fwd) {
     for(int& p= it[l]; p < st[l + 1];)
      if(const int c= adj[p++]; vis[c] != ph) {
       r= c;
       break;
      }
    } else {
     for(int& p= it[l]; p >= st[l];)
      if(const int c= adj[p--]; vis[c] != ph) {
       r= c;
       break;
      }
    }
    if(r < 0) {
     --sp;
     continue;
    }
    vis[r]= ph, via[sp - 1]= r;
    const int l2= mr[r];
    stk[sp++]= l2, it[l2]= fwd ? st[l2] : st[l2 + 1] - 1;
   }
  }
  if(!aug) break;
 }
 return ml;
}
}
struct Solver {
 int L, R;
 const vector<array<int, 2>>& e;
 vector<int> ml;
 Solver(int l, int r, const vector<array<int, 2>>& edges): L(l), R(r), e(edges) {}
 void run() { ml= bm_pf::bipartite_matching(L, R, e); }
 vector<array<int, 2>> answer() const {
  vector<array<int, 2>> ret;
  for(int l= 0; l < L; ++l)
   if(ml[l] >= 0) ret.push_back({l, ml[l]});
  return ret;
 }
};
