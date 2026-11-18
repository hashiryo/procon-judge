#pragma once
// 全域最小カットの書き比べの核。Nagamochi-Ono-Ibaraki の方法 (CAPFOREST)。上界 λ̂ (はじめは重み付きの次数の最小) を持ち、
// maximum adjacency の順に頂点をなめる。頂点 v をなめたとき、隣の未訪問の w の r(w) (なめた頂点から w への重みの和) が
// λ̂ を越えたら、辺 (v, w) の両端の局所連結度は λ̂ 以上なので (q(e) = r(w) は局所連結度の下界)、v と w をまとめる。なめた
// 頂点の集合のカットの値も足し引きで持ち、λ̂ を下げる。1 回なめたら、まとめた頂点を縮めて多重辺を足し、頂点が 1 つになる
// まで繰り返す。λ(G) < λ̂ なら、まとめた組はどの最小カットにも分けられないので、まとめて縮めてよい。1 回のなめで 1 組も
// まとまらなければ、Stoer-Wagner と同じく最後の 2 頂点をまとめる (その組の局所連結度は最後の頂点の次数で、λ̂ 以上)。
// PR が 1 以上なら、なめる前に Padberg-Rinaldi の条件で縮める。1 は 2 つの条件で、重みが λ̂ 以上の辺と、
// 2 c(u, v) > min(deg u, deg v) の辺。後者は等号だと「分けない最小カットもある」までしか言えず、まとめて縮めると壊れうるので、
// 等号を除く。2 は 3 つ目の条件も使う。c(u, v) と、共通の隣 w を通る長さ 2 の道の min(c(u, w), c(w, v)) の和は、u と v の
// 局所連結度の下界 (道どうしは辺を共有しない) なので、λ̂ 以上なら縮める。重みがどれも 1 の完全グラフは、これで 1 回で縮む。
// 優先度付きの待ち行列は、古い鍵を残す 2 分ヒープ。HYB なら、なめる前に今のグラフが密 (辺の数 × 20 が頂点の数の 2 乗以上で、
// 頂点が 4096 以下) なら、隣接行列で回す noi_dense に移る。縮めるうちに密になったグラフも行列で仕上げる。
#include <algorithm>
#include <array>
#include <queue>
#include <utility>
#include <vector>
#include "noi_dense.hpp"
namespace gmc {
template <int PR, bool HYB= false> long long noi(int n, const std::vector<std::array<long long, 3>>& edges) {
 if(n <= 1) return 0;
 // 今のグラフ。頂点 0..k-1、辺 (u, v, c) は u < v で、同じ組はまとめてある。
 int k= n;
 std::vector<std::array<long long, 3>> es;
 for(auto& [u, v, c]: edges)
  if(u != v) es.push_back({std::min(u, v), std::max(u, v), c});
 std::vector<int> par(n), id(n);
 auto find= [&](int x) {
  while(par[x] != x) x= par[x]= par[par[x]];
  return x;
 };
 std::vector<long long> deg, r;
 std::vector<int> st, to;
 std::vector<long long> wt;
 std::vector<long long> acc;
 std::vector<int> seen;
 // es の頂点を par でまとめた番号に付け替え、同じ組を足して es を作り直す。k も更新する。
 auto rebuild= [&]() {
  int cnt= 0;
  for(int v= 0; v < k; ++v)
   if(find(v) == v) id[v]= cnt++;
  std::vector<std::array<long long, 3>> ne;
  ne.reserve(es.size());
  for(auto& [u, v, c]: es) {
   const long long a= id[find(u)], b= id[find(v)];
   if(a != b) ne.push_back({std::min(a, b), std::max(a, b), c});
  }
  k= cnt;
  // 同じ組を足す。左の頂点ごとに並べてから、右の頂点の印でまとめる。
  std::vector<int> cntv(k + 1, 0);
  for(auto& e: ne) ++cntv[e[0] + 1];
  for(int i= 0; i < k; ++i) cntv[i + 1]+= cntv[i];
  std::vector<std::array<long long, 3>> srt(ne.size());
  for(auto& e: ne) srt[cntv[e[0]]++]= e;
  es.clear();
  seen.assign(k, -1);
  for(size_t i= 0; i < srt.size();) {
   const long long a= srt[i][0];
   const size_t base= es.size();
   for(; i < srt.size() && srt[i][0] == a; ++i) {
    const int b= srt[i][1];
    if(seen[b] >= (int)base) es[seen[b]][2]+= srt[i][2];
    else seen[b]= es.size(), es.push_back(srt[i]);
   }
  }
  for(int v= 0; v < k; ++v) par[v]= v;
 };
 for(int v= 0; v < n; ++v) par[v]= v;
 rebuild();
 auto build_adj= [&]() {
  deg.assign(k, 0), st.assign(k + 1, 0);
  for(auto& [u, v, c]: es) deg[u]+= c, deg[v]+= c, ++st[u + 1], ++st[v + 1];
  for(int i= 0; i < k; ++i) st[i + 1]+= st[i];
  to.resize(2 * es.size()), wt.resize(2 * es.size());
  std::vector<int> pos(st.begin(), st.end() - 1);
  for(auto& [u, v, c]: es) to[pos[u]]= v, wt[pos[u]++]= c, to[pos[v]]= u, wt[pos[v]++]= c;
 };
 build_adj();
 long long lam= *std::min_element(deg.begin(), deg.end());
 std::vector<char> vis;
 while(k > 1 && lam > 0) {
  if constexpr(HYB)
   if(k <= 4096 && (long long)es.size() * 20 >= (long long)k * k) return std::min(lam, noi_dense<true>(k, es));
  if constexpr(PR >= 1) {
   bool any= false;
   for(auto& [u, v, c]: es)
    if(c >= lam || 2 * c > std::min(deg[u], deg[v])) {
     const int a= find(u), b= find(v);
     if(a != b) par[a]= b, any= true;
    }
   if constexpr(PR >= 2) {
    // u の隣に重みを書いてから、u < v の隣 v ごとに v の隣をなめ、共通の隣の min を足す。
    std::vector<long long> wu(k, 0);
    std::vector<int> mk(k, -1);
    for(int u= 0; u < k; ++u) {
     for(int i= st[u]; i < st[u + 1]; ++i) wu[to[i]]= wt[i], mk[to[i]]= u;
     for(int i= st[u]; i < st[u + 1]; ++i) {
      const int v= to[i];
      if(v < u) continue;
      long long sum= wt[i];
      for(int j= st[v]; j < st[v + 1] && sum < lam; ++j)
       if(mk[to[j]] == u) sum+= std::min(wu[to[j]], wt[j]);
      if(sum >= lam) {
       const int a= find(u), b= find(v);
       if(a != b) par[a]= b, any= true;
      }
     }
    }
   }
   if(any) {
    rebuild(), build_adj();
    if(k > 1) lam= std::min(lam, *std::min_element(deg.begin(), deg.end()));
    continue;
   }
  }
  r.assign(k, 0), vis.assign(k, 0);
  std::priority_queue<std::pair<long long, int>> pq;
  for(int v= 0; v < k; ++v) pq.push({0, v});
  long long cut= 0;
  int scanned= 0, prev= -1, last= -1;
  bool any= false;
  while(!pq.empty()) {
   auto [key, v]= pq.top();
   pq.pop();
   if(vis[v] || key != r[v]) continue;
   vis[v]= 1, ++scanned, prev= last, last= v;
   cut+= deg[v] - 2 * r[v];
   if(scanned < k && cut < lam) lam= cut;
   for(int i= st[v]; i < st[v + 1]; ++i) {
    const int w= to[i];
    if(vis[w]) continue;
    if(r[w] < lam && r[w] + wt[i] >= lam) {
     const int a= find(v), b= find(w);
     if(a != b) par[a]= b, any= true;
    }
    r[w]+= wt[i], pq.push({r[w], w});
   }
  }
  if(lam == 0) break;
  if(!any) par[find(last)]= find(prev);
  rebuild(), build_adj();
  if(k > 1) lam= std::min(lam, *std::min_element(deg.begin(), deg.end()));
 }
 return lam;
}
}  // namespace gmc
