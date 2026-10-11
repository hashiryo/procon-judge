#pragma once
// 三辺連結成分の基準。Library の実装は無いので、Library Checker の参照解 (Tsin の吸収の形の 1 回の DFS) を反復に書き直したもの。
// 隣接は std::vector の CSR で、自己ループは入れない。親への木の辺は 1 本だけ飛ばす (2 本目からの平行な辺は後ろ向きの辺)。
//
// 頂点 v から帰るとき、v の部分木から出る辺の数の見積もり deg[v] と lowlink の low[v]、v から下へ伸びる「道」path[v] (まだ v に
// 吸収していない頂点の列) を持つ。子 w から帰ったとき、木の辺 (v, w) が 2 辺の切断に入る (w の道が空で deg[w] <= 1) なら w の道は
// そこで切れる。そうでなければ、w の道と v の道のうち low の小さい方を v の道にし、もう片方を v に吸収する。子孫 w から v への
// 後ろ向きの辺を見たら、v の道のうち w の祖先にあたる頂点を v に吸収する。吸収した頂点は、吸収した祖先を親に持ち、最後に
// 行きがけ順に親の名前を写して成分の名前にする。
#include "pj.hpp"
struct Solver {
 int n;
 const vector<array<int, 2>>& es;
 vector<int> lab;
 Solver(int n, const vector<array<int, 2>>& es): n(n), es(es) {}
 void run() {
  const int N= n, NIL= n;
  vector<int> off(N + 2, 0);
  for(auto& e: es)
   if(e[0] != e[1]) ++off[e[0] + 2], ++off[e[1] + 2];
  for(int x= 2; x <= N + 1; ++x) off[x]+= off[x - 1];
  vector<int> adj(off[N + 1]);
  for(auto& e: es)
   if(e[0] != e[1]) adj[off[e[0] + 1]++]= e[1], adj[off[e[1] + 1]++]= e[0];
  vector<int> in(N), out(N), low(N), deg(N), path(N + 1), par(N), order;
  vector<char> vis(N, 0);
  order.reserve(N);
  struct F {
   int v, p, i;
  };
  vector<F> st;
  int cnt= 0;
  auto enter= [&](int v) { vis[v]= 1, in[v]= cnt++, low[v]= N, deg[v]= 0, path[v]= NIL, par[v]= v, order.push_back(v); };
  auto absorb= [&](int v, int u) { par[u]= v, deg[v]+= deg[u]; };
  for(int r= 0; r < N; ++r) {
   if(vis[r]) continue;
   enter(r);
   int v= r, p= NIL, i= off[r];
   for(;;) {
    if(i < off[v + 1]) {
     int w= adj[i++];
     if(w == p) {
      p= NIL;
      continue;
     }
     if(vis[w]) {
      if(in[w] < in[v]) ++deg[v], low[v]= min(low[v], in[w]);
      else {
       --deg[v];
       int u= path[v];
       while(u != NIL && in[u] <= in[w] && in[w] < out[u]) absorb(v, u), u= path[u];
       path[v]= u;
      }
      continue;
     }
     st.push_back({v, p, i}), p= v, v= w, i= off[w], enter(w);
    } else {
     out[v]= cnt;
     if(st.empty()) break;
     int w= v;
     v= st.back().v, p= st.back().p, i= st.back().i, st.pop_back();
     if(path[w] == NIL && deg[w] <= 1) {
      deg[v]+= deg[w], low[v]= min(low[v], low[w]);
      continue;
     }
     if(deg[w] == 0) w= path[w];
     if(low[w] < low[v]) low[v]= low[w], swap(w, path[v]);
     while(w != NIL) absorb(v, w), w= path[w];
    }
   }
  }
  lab.resize(N);
  for(int x: order) lab[x]= par[x] == x ? x : lab[par[x]];
 }
 int comp(int v) const { return lab[v]; }
};
