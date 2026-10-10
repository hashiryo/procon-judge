#pragma once
// 一般グラフの最大マッチング。空いている頂点すべてを根にした交互森を、1 本の BFS の待ち行列で同時に育てる。
//
// 花の縮め方と増加路の反転は gm_gabow.hpp と同じ。頂点ごとに木の根 tr を持ち、偶どうしを結ぶ辺が同じ木の中なら花を
// 縮め、違う木をつなげば増加路なので、両方の木を反転してその回の残りでは使わない (死んだ木)。待ち行列が空になった
// とき、その回に 1 本でも流していれば印を新しくして次の回を始め、1 本も流せなければ森が完成しているので終わる。
//
// RELABEL が true なら、死んだ木の頂点をその回のうちに別の木が拾い直してよい (印を付け直す)。false なら、死んだ木の
// 頂点はその回の残りで通らない。どちらでも、最後の回は 1 本も流さない完全な森なので、最大になっている。
#include "_shared/flow/gm_gabow.hpp"
namespace gm_forest {
template <int INIT, bool RELABEL> std::vector<int> general_matching(int n, const std::vector<std::array<int, 2>>& e) {
 std::vector<int> off, adj;
 gm_gabow::build_csr(n, e, off, adj);
 std::vector<int> mate= gm_gabow::init_matching<INIT>(n, off, adj);
 // z[v] == t なら v はこの回の偶、zo[v] == t なら奇 (花で偶になった頂点は両方が t)。tr[v] は木の根。
 std::vector<int> z(n), zo(n), p(n), tr(n), q, stk;
 std::vector<char> alive(n);
 std::vector<std::array<int, 2>> fs(n);
 q.reserve(n);
 int t= 0;
 auto find= [&](int x) {
  int r= x;
  while(z[r] == t && p[r] >= 0) r= p[r];
  while(x != r) {
   const int nx= p[x];
   p[x]= r, x= nx;
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
 for(;;) {
  ++t, q.clear();
  for(int v= 0; v < n; ++v)
   if(mate[v] < 0 && off[v] < off[v + 1]) z[v]= t, p[v]= -1, tr[v]= v, alive[v]= 1, fs[v]= {-1, -1}, q.push_back(v);
  int aug= 0;
  for(size_t i= 0; i < q.size(); ++i) {
   const int x= q[i];
   if(z[x] != t || !alive[tr[x]]) continue;
   const int r= tr[x];
   for(int k= off[x]; k < off[x + 1]; ++k) {
    const int y= adj[k];
    const bool labeled= (z[y] == t || zo[y] == t);
    if(labeled && alive[tr[y]]) {
     if(z[y] != t) continue;  // 奇
     if(tr[y] != r) {
      rematch(x, y), rematch(y, x), alive[r]= alive[tr[y]]= 0, ++aug;
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
    } else {
     if(!RELABEL && labeled) continue;  // 死んだ木
     const int my= mate[y];
     if(my < 0) continue;  // 起きない (空いている頂点はどれも生きた木の根か、死んだ木で組まれた)
     if constexpr(RELABEL) z[y]= 0;
     zo[y]= t, tr[y]= r, fs[y]= {-1, -1};
     z[my]= t, p[my]= y, tr[my]= r, fs[my]= {x, -1}, q.push_back(my);
    }
   }
  }
  if(!aug) break;
 }
 return mate;
}
}
