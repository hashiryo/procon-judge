#pragma once
// 一般グラフの最大マッチング。空いている頂点すべてを根にした交互森を 1 本の BFS の待ち行列で育て、森を作り直さずに
// 最後まで保つ (二部マッチングの MS-BFS-Graft (Azad, Buluç, Pothen) の木の付け替えを、花のある一般グラフで行う)。
//
// 花の縮め方と増加路の反転は gm_gabow.hpp と同じ。違う木の偶どうしが隣り合えば流し、その 2 本の木だけを壊して印を
// 消す。壊した頂点の隣にいる、生きている木の偶の頂点を待ち行列に積み直すので、壊れた所は隣の木が育ち直して拾う。
// 増加路の反転で組が変わるのは壊した 2 本の木の中だけなので、ほかの木はそのまま正しい交互木として残る。待ち行列が
// 空になったとき、偶の頂点から出る辺はどれも、端点の印が最後に変わったあとに見てあるので、森は完成していて最大。
#include "_shared/flow/gm_gabow.hpp"
namespace gm_graft {
template <int INIT> std::vector<int> general_matching(int n, const std::vector<std::array<int, 2>>& e) {
 std::vector<int> off, adj;
 gm_gabow::build_csr(n, e, off, adj);
 std::vector<int> mate= gm_gabow::init_matching<INIT>(n, off, adj);
 // z[v] が 1 なら偶、zo[v] が 1 なら奇 (花で偶になった頂点は両方)。どちらも 0 なら印が無い。tr[v] は木の根で、
 // 木の頂点は根ごとの連結リスト (hd, nx) に並べる。
 std::vector<int> z(n), zo(n), p(n), tr(n), hd(n, -1), nx(n), q, stk, dead;
 std::vector<std::array<int, 2>> fs(n);
 q.reserve(n);
 constexpr int t= 1;
 auto find= [&](int x) {
  int r= x;
  while(z[r] == t && p[r] >= 0) r= p[r];
  while(x != r) {
   const int ny= p[x];
   p[x]= r, x= ny;
  }
  return r;
 };
 auto rematch= [&](int u, int v) {
  stk.clear();
  for(;;) {
   for(;;) {
    const int w= mate[u];
    mate[u]= v;
    if(w < 0 || mate[w] != u) break;
    if(auto [x, y]= fs[u]; y < 0) mate[w]= x, u= x, v= w;
    else stk.push_back(y), stk.push_back(x), u= x, v= y;
   }
   if(stk.empty()) break;
   v= stk.back(), stk.pop_back(), u= stk.back(), stk.pop_back();
  }
 };
 auto join= [&](int v, int r) { tr[v]= r, nx[v]= hd[r], hd[r]= v; };
 for(int v= 0; v < n; ++v)
  if(mate[v] < 0 && off[v] < off[v + 1]) z[v]= t, p[v]= -1, fs[v]= {-1, -1}, join(v, v), q.push_back(v);
 for(size_t i= 0; i < q.size(); ++i) {
  const int x= q[i];
  if(z[x] != t) continue;
  const int r= tr[x];
  for(int k= off[x]; k < off[x + 1]; ++k) {
   const int y= adj[k];
   if(z[y] == t) {
    if(const int ry= tr[y]; ry != r) {
     rematch(x, y), rematch(y, x);
     // 2 本の木を壊す。印を消してから、隣の生きている偶の頂点を積み直す。
     dead.clear();
     for(int c: {r, ry}) {
      for(int v= hd[c]; v >= 0; v= nx[v]) z[v]= zo[v]= 0, dead.push_back(v);
      hd[c]= -1;
     }
     for(int v: dead)
      for(int l= off[v]; l < off[v + 1]; ++l)
       if(const int w= adj[l]; z[w] == t) q.push_back(w);
     break;
    }
    int u= find(x), v= find(y), w= r;
    if(u == v) continue;
    for(; u != r || v != r; fs[u]= {x, y}, u= find(fs[mate[u]][0])) {
     if(v != r) std::swap(u, v);
     if(fs[u][0] == x && fs[u][1] == y) {
      w= u;
      break;
     }
    }
    for(int a: {find(x), find(y)})
     for(; a != w; a= find(fs[mate[a]][0])) z[a]= t, p[a]= w, q.push_back(a);
   } else if(!zo[y]) {
    const int my= mate[y];
    if(my < 0) continue;  // 起きない (空いている頂点はどれも木の根)
    zo[y]= t, fs[y]= {-1, -1}, join(y, r);
    z[my]= t, p[my]= y, fs[my]= {x, -1}, join(my, r), q.push_back(my);
   }
  }
 }
 return mate;
}
}
