#pragma once
// 一般グラフの重み最大マッチング。よく知られた O(n^3) の主双対の花の方法 (頂点 1..n、花 n + 1..2n、2n × 2n の辺の行列) を
// 素直に書いたもの。今の Library の WeightedMatching と同じ形で、比べる基準にする。
//
// lab は頂点の双対の 2 倍 (花は z の 2 倍)、辺の縮約費用は lab[u] + lab[v] - 2w。重みは 1 以上を仮定し、0 は辺が無い印。
// 段ごとに、空いている頂点を根にした交互森を BFS で育て、きつい辺で木を伸ばすか、花を縮めるか、増加路を流す。
// 伸ばせなくなったら双対を動かす。空いた頂点の双対が 0 に達したら終わる (最大重みで、完全である必要はない)。
#include <algorithm>
#include <array>
#include <vector>
namespace wm_std {
struct Solver {
 using i64= long long;
 struct E {
  int u, v;
  i64 w;
 };
 int n, nx, N2;
 std::vector<E> g;  // (2n + 1)^2
 std::vector<i64> lab;
 std::vector<int> match, slack, st, pa, ffrom, S, vis, q;
 std::vector<std::vector<int>> flo;
 int qh= 0, tvis= 0;
 E& e(int u, int v) { return g[u * N2 + v]; }
 int& ff(int b, int x) { return ffrom[b * (n + 1) + x]; }
 i64 dlt(const E& x) const { return lab[x.u] + lab[x.v] - g[x.u * N2 + x.v].w * 2; }
 explicit Solver(int n): n(n), nx(n), N2(2 * n + 1), g((size_t)N2 * N2), lab(N2), match(N2), slack(N2), st(N2), pa(N2), ffrom((size_t)N2 * (n + 1)), S(N2), vis(N2), flo(N2) {
  for(int u= 1; u <= n; ++u)
   for(int v= 1; v <= n; ++v) e(u, v)= E{u, v, 0};
 }
 void add_edge(int u, int v, i64 w) {  // 0 始まり
  ++u, ++v;
  if(w > e(u, v).w) e(u, v).w= e(v, u).w= w;
 }
 void update_slack(int u, int x) {
  if(!slack[x] || dlt(e(u, x)) < dlt(e(slack[x], x))) slack[x]= u;
 }
 void set_slack(int x) {
  slack[x]= 0;
  for(int u= 1; u <= n; ++u)
   if(e(u, x).w > 0 && st[u] != x && S[st[u]] == 0) update_slack(u, x);
 }
 void q_push(int x) {
  if(x <= n) q.push_back(x);
  else
   for(int y: flo[x]) q_push(y);
 }
 void set_st(int x, int b) {
  st[x]= b;
  if(x > n)
   for(int y: flo[x]) set_st(y, b);
 }
 int get_pr(int b, int xr) {
  int pr= std::find(flo[b].begin(), flo[b].end(), xr) - flo[b].begin();
  if(pr % 2 == 1) {
   std::reverse(flo[b].begin() + 1, flo[b].end());
   return (int)flo[b].size() - pr;
  }
  return pr;
 }
 void set_match(int u, int v) {
  match[u]= e(u, v).v;
  if(u <= n) return;
  const int xr= ff(u, e(u, v).u), pr= get_pr(u, xr);
  for(int i= 0; i < pr; ++i) set_match(flo[u][i], flo[u][i ^ 1]);
  set_match(xr, v);
  std::rotate(flo[u].begin(), flo[u].begin() + pr, flo[u].end());
 }
 void augment(int u, int v) {
  for(;;) {
   const int xnv= st[match[u]];
   set_match(u, v);
   if(!xnv) return;
   set_match(xnv, st[pa[xnv]]);
   u= st[pa[xnv]], v= xnv;
  }
 }
 int get_lca(int u, int v) {
  for(++tvis; u || v; std::swap(u, v)) {
   if(u == 0) continue;
   if(vis[u] == tvis) return u;
   vis[u]= tvis;
   u= st[match[u]];
   if(u) u= st[pa[u]];
  }
  return 0;
 }
 void add_blossom(int u, int lca, int v) {
  int b= n + 1;
  while(b <= nx && st[b]) ++b;
  if(b > nx) ++nx;
  lab[b]= 0, S[b]= 0;
  match[b]= match[lca];
  flo[b].clear();
  flo[b].push_back(lca);
  for(int x= u, y; x != lca; x= st[pa[y]]) flo[b].push_back(x), flo[b].push_back(y= st[match[x]]), q_push(y);
  std::reverse(flo[b].begin() + 1, flo[b].end());
  for(int x= v, y; x != lca; x= st[pa[y]]) flo[b].push_back(x), flo[b].push_back(y= st[match[x]]), q_push(y);
  set_st(b, b);
  for(int x= 1; x <= nx; ++x) e(b, x).w= e(x, b).w= 0;
  for(int x= 1; x <= n; ++x) ff(b, x)= 0;
  for(int xs: flo[b]) {
   for(int x= 1; x <= nx; ++x)
    if(e(b, x).w == 0 || dlt(e(xs, x)) < dlt(e(b, x))) e(b, x)= e(xs, x), e(x, b)= e(x, xs);
   for(int x= 1; x <= n; ++x)
    if(ff(xs, x)) ff(b, x)= xs;
  }
  set_slack(b);
 }
 void expand_blossom(int b) {
  for(int y: flo[b]) set_st(y, y);
  const int xr= ff(b, e(b, pa[b]).u), pr= get_pr(b, xr);
  for(int i= 0; i < pr; i+= 2) {
   const int xs= flo[b][i], xns= flo[b][i + 1];
   pa[xs]= e(xns, xs).u;
   S[xs]= 1, S[xns]= 0;
   slack[xs]= 0, set_slack(xns);
   q_push(xns);
  }
  S[xr]= 1, pa[xr]= pa[b];
  for(size_t i= pr + 1; i < flo[b].size(); ++i) {
   const int xs= flo[b][i];
   S[xs]= -1, set_slack(xs);
  }
  st[b]= 0;
 }
 bool on_found_edge(const E& x) {
  const int u= st[x.u], v= st[x.v];
  if(S[v] == -1) {
   pa[v]= x.u, S[v]= 1;
   const int nu= st[match[v]];
   slack[v]= slack[nu]= 0;
   S[nu]= 0, q_push(nu);
  } else if(S[v] == 0) {
   const int lca= get_lca(u, v);
   if(!lca) return augment(u, v), augment(v, u), true;
   add_blossom(u, lca, v);
  }
  return false;
 }
 // 1 段。増加路を流せたら true、双対が尽きたら false。
 bool matching() {
  std::fill(S.begin() + 1, S.begin() + nx + 1, -1);
  std::fill(slack.begin() + 1, slack.begin() + nx + 1, 0);
  q.clear(), qh= 0;
  for(int x= 1; x <= nx; ++x)
   if(st[x] == x && !match[x]) pa[x]= 0, S[x]= 0, q_push(x);
  if(q.empty()) return false;
  for(;;) {
   while(qh < (int)q.size()) {
    const int u= q[qh++];
    if(S[st[u]] == 1) continue;
    for(int v= 1; v <= n; ++v)
     if(e(u, v).w > 0 && st[u] != st[v]) {
      if(dlt(e(u, v)) == 0) {
       if(on_found_edge(e(u, v))) return true;
      } else update_slack(u, st[v]);
     }
   }
   i64 d= (i64)1 << 62;
   for(int b= n + 1; b <= nx; ++b)
    if(st[b] == b && S[b] == 1) d= std::min(d, lab[b] / 2);
   for(int x= 1; x <= nx; ++x)
    if(st[x] == x && slack[x]) {
     if(S[x] == -1) d= std::min(d, dlt(e(slack[x], x)));
     else if(S[x] == 0) d= std::min(d, dlt(e(slack[x], x)) / 2);
    }
   for(int u= 1; u <= n; ++u) {
    if(S[st[u]] == 0) {
     if(lab[u] <= d) return false;
     lab[u]-= d;
    } else if(S[st[u]] == 1) lab[u]+= d;
   }
   for(int b= n + 1; b <= nx; ++b)
    if(st[b] == b) {
     if(S[st[b]] == 0) lab[b]+= d * 2;
     else if(S[st[b]] == 1) lab[b]-= d * 2;
    }
   q.clear(), qh= 0;
   for(int x= 1; x <= nx; ++x)
    if(st[x] == x && slack[x] && st[slack[x]] != x && dlt(e(slack[x], x)) == 0)
     if(on_found_edge(e(slack[x], x))) return true;
   for(int b= n + 1; b <= nx; ++b)
    if(st[b] == b && S[b] == 1 && lab[b] == 0) expand_blossom(b);
  }
 }
 // 頂点ごとの相手 (0 始まり、いなければ -1) を返す。
 std::vector<int> solve() {
  nx= n;
  for(int u= 0; u <= n; ++u) st[u]= u, flo[u].clear();
  i64 wmax= 0;
  for(int u= 1; u <= n; ++u)
   for(int v= 1; v <= n; ++v) ff(u, v)= (u == v ? u : 0), wmax= std::max(wmax, e(u, v).w);
  for(int u= 1; u <= n; ++u) lab[u]= wmax;
  while(matching());
  std::vector<int> mt(n, -1);
  for(int u= 1; u <= n; ++u)
   if(match[u]) mt[u - 1]= match[u] - 1;
  return mt;
 }
};
}
