#pragma once
// 支配木の基準。Library の実装は無いので、Library Checker の参照解と同じ、経路圧縮だけの Lengauer–Tarjan を反復で書いたもの。
// 隣接は std::vector の CSR で、順向きと逆向きを組む。
//
// s から届く頂点に DFS の行きがけ順の番号を振り、以後は番号の上で計算する。番号の大きい頂点から順に、逆向きの辺の始点の eval の
// 準支配点 (sdom) の最小を sdom にし、sdom のバケットに入れてから DFS の木の親と森でつなぐ。親のバケットの頂点 v は、eval(v) の
// sdom が v の sdom より小さければ eval(v) を、そうでなければ親を仮の直接支配点にする。最後に番号の小さい順に、仮の直接支配点が
// sdom と違う頂点を、仮の直接支配点の直接支配点に直す。eval は森の根までの道を圧縮しながら、道の上で sdom の最小の頂点を返す。
#include <numeric>
#include "pj.hpp"
struct Solver {
 int n, s;
 const vector<array<int, 2>>& es;
 vector<int> ans;
 Solver(int n, int s, const vector<array<int, 2>>& es): n(n), s(s), es(es) {}
 void run() {
  const int N= n, m= es.size();
  vector<int> fo(N + 2, 0), ro(N + 2, 0), fa(m), ra(m);
  for(auto& e: es) ++fo[e[0] + 2], ++ro[e[1] + 2];
  for(int x= 2; x <= N + 1; ++x) fo[x]+= fo[x - 1], ro[x]+= ro[x - 1];
  for(auto& e: es) fa[fo[e[0] + 1]++]= e[1], ra[ro[e[1] + 1]++]= e[0];
  // DFS (反復)。num[v] は行きがけ順の番号 (届かなければ -1)、vert[i] は番号 i の頂点、par[i] は木の親の番号。
  vector<int> num(N, -1), vert, par;
  vert.reserve(N), par.reserve(N);
  vector<pair<int, int>> st;  // (頂点, 次に見る順向きの辺の位置)
  num[s]= 0, vert.push_back(s), par.push_back(0), st.push_back({s, fo[s]});
  while(!st.empty()) {
   auto& [v, i]= st.back();
   if(i == fo[v + 1]) {
    st.pop_back();
    continue;
   }
   const int w= fa[i++];
   if(num[w] >= 0) continue;
   num[w]= vert.size(), par.push_back(num[v]), vert.push_back(w), st.push_back({w, fo[w]});
  }
  const int R= vert.size();
  vector<int> sdom(R), anc(R, -1), lab(R), idom(R), bhead(R, -1), bnext(R), path;
  iota(sdom.begin(), sdom.end(), 0), iota(lab.begin(), lab.end(), 0);
  auto eval= [&](int v) {
   if(anc[v] < 0) return v;
   // 森の根の手前まで登り、上から順に lab と anc を直す。
   path.clear();
   for(int x= v; anc[anc[x]] >= 0; x= anc[x]) path.push_back(x);
   for(int k= int(path.size()); k--;) {
    const int x= path[k], a= anc[x];
    if(sdom[lab[a]] < sdom[lab[x]]) lab[x]= lab[a];
    anc[x]= anc[a];
   }
   return lab[v];
  };
  for(int w= R - 1; w >= 1; --w) {
   const int vw= vert[w];
   for(int j= ro[vw]; j < ro[vw + 1]; ++j) {
    const int u= num[ra[j]];
    if(u < 0) continue;
    sdom[w]= min(sdom[w], sdom[eval(u)]);
   }
   bnext[w]= bhead[sdom[w]], bhead[sdom[w]]= w;
   const int p= par[w];
   anc[w]= p;
   for(int v= bhead[p]; v >= 0; v= bnext[v]) {
    const int u= eval(v);
    idom[v]= sdom[u] < sdom[v] ? u : p;
   }
   bhead[p]= -1;
  }
  for(int w= 1; w < R; ++w)
   if(idom[w] != sdom[w]) idom[w]= idom[idom[w]];
  ans.assign(N, -1);
  ans[s]= s;
  for(int w= 1; w < R; ++w) ans[vert[w]]= vert[idom[w]];
 }
 int parent(int v) const { return ans[v]; }
};
