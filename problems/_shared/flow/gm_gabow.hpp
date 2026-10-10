#pragma once
// 一般グラフの最大マッチング。今の Library の general_matching と同じ探索 (Gabow 風の O(VE) の Edmonds の方法) を、CSR と
// 反復で書き直し、初期のマッチングを選べるようにしたもの。
//
// 空いている頂点 r ごとに、r を根とする交互木を BFS で育てる。偶 (outer) の頂点の組は、奇の頂点を代表にして union-find
// でまとめる (根は自分が代表)。偶どうしを結ぶ辺 (x, y) を見つけたら、両側から代表をたどって最小の共通祖先を探し、
// その間の奇の頂点を偶にして花 (blossom) を縮める。偶の頂点が空いている頂点と隣り合えば、増加路を反転して止める。
// 頂点の印 z は探索の回の番号で、増加路が見つからなかった探索の印は次の探索でもそのまま残す。見つからなかった木の
// 頂点は、以後のどの増加路にも乗らないので (Edmonds)、残した印が壁になって次の探索が入らない。
//
// INIT は初期のマッチングの作り方。0 は空、1 は頂点の番号の順に最初の空いている隣と組ませる貪欲、2 は Karp-Sipser
// (空いている隣が 1 つだけになった頂点を先に組ませ、無くなったら番号の小さい頂点を最初の空いている隣と組ませる)。
#include <array>
#include <utility>
#include <vector>
namespace gm_gabow {
// 辺の列から両向きの CSR を組む。
inline void build_csr(int n, const std::vector<std::array<int, 2>>& e, std::vector<int>& off, std::vector<int>& adj) {
 const int m= e.size();
 off.assign(n + 1, 0), adj.resize(2 * m);
 for(auto& [u, v]: e) ++off[u], ++off[v];
 for(int i= 0; i < n; ++i) off[i + 1]+= off[i];
 for(int i= m; i--;) {
  auto [u, v]= e[i];
  adj[--off[u]]= v, adj[--off[v]]= u;
 }
}
// 初期のマッチングを作る。INIT は 0 が空、1 が貪欲、2 が Karp-Sipser。
template <int INIT> std::vector<int> init_matching(int n, const std::vector<int>& off, const std::vector<int>& adj) {
 std::vector<int> mate(n, -1);
 if constexpr(INIT == 1) {
  for(int v= 0; v < n; ++v)
   if(mate[v] < 0)
    for(int k= off[v]; k < off[v + 1]; ++k)
     if(const int u= adj[k]; mate[u] < 0) {
      mate[v]= u, mate[u]= v;
      break;
     }
 } else if constexpr(INIT == 2) {
  // deg は空いている隣の数 (多重辺は重ねて数える)。1 に落ちた頂点を stk に積む。各頂点が 1 になるのは一度だけ。
  std::vector<int> deg(n), stk(n);
  int sp= 0;
  for(int v= 0; v < n; ++v)
   if((deg[v]= off[v + 1] - off[v]) == 1) stk[sp++]= v;
  auto take= [&](int a, int b) {
   mate[a]= b, mate[b]= a;
   for(int x: {a, b})
    for(int k= off[x]; k < off[x + 1]; ++k)
     if(const int w= adj[k]; mate[w] < 0 && --deg[w] == 1) stk[sp++]= w;
  };
  auto pick= [&](int v) {
   for(int k= off[v]; k < off[v + 1]; ++k)
    if(const int u= adj[k]; mate[u] < 0) return take(v, u);
  };
  for(int cur= 0;;) {
   while(sp)
    if(const int v= stk[--sp]; mate[v] < 0) pick(v);
   while(cur < n && (mate[cur] >= 0 || deg[cur] == 0)) ++cur;
   if(cur == n) break;
   pick(cur);
  }
 }
 return mate;
}
template <int INIT> std::vector<int> general_matching(int n, const std::vector<std::array<int, 2>>& e) {
 std::vector<int> off, adj;
 build_csr(n, e, off, adj);
 std::vector<int> mate= init_matching<INIT>(n, off, adj);
 // fs[v] は、偶の頂点 v から根への交互路の作り方。v が奇の頂点の相手として偶になったなら {その奇の頂点を見つけた偶の
 // 頂点, -1}、奇から花で偶になったなら花を閉じた辺 {x, y} (x が v の側)。p は union-find の親。
 std::vector<int> z(n), p(n), q, stk;
 std::vector<std::array<int, 2>> fs(n);
 q.reserve(n);
 int t= 1;
 auto find= [&](int x) {
  int r= x;
  while(z[r] == t && p[r] >= 0) r= p[r];
  while(x != r) {
   const int nx= p[x];
   p[x]= r, x= nx;
  }
  return r;
 };
 // u を v と組ませ、u から根への交互路を反転する。花を通るところでは 2 本に分かれるので、後の 1 本を積んでおく。
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
 auto search= [&](int r) {
  q.clear(), q.push_back(r), fs[r]= {-1, -1}, z[r]= t, p[r]= -1;
  for(size_t i= 0; i < q.size(); ++i) {
   const int x= q[i];
   for(int k= off[x]; k < off[x + 1]; ++k) {
    const int y= adj[k];
    if(y == r) continue;
    if(mate[y] < 0) return mate[y]= x, rematch(x, y), true;
    if(z[y] == t) {
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
    } else if(const int my= mate[y]; z[my] != t) fs[y]= {-1, -1}, fs[my]= {x, -1}, z[my]= t, p[my]= y, q.push_back(my);
   }
  }
  return false;
 };
 for(int r= 0; r < n; ++r)
  if(mate[r] < 0) t+= search(r);
 return mate;
}
// 相手の配列から、マッチングに使う辺の番号を返す (多重辺はどれか 1 本)。
inline std::vector<int> edge_ids(const std::vector<int>& mate, const std::vector<std::array<int, 2>>& e) {
 std::vector<int> ids;
 std::vector<char> done(mate.size());
 for(int i= 0; i < (int)e.size(); ++i)
  if(auto [u, v]= e[i]; mate[u] == v && !done[u]) done[u]= done[v]= 1, ids.push_back(i);
 return ids;
}
}
