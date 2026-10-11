#pragma once
// 二部グラフの辺彩色 (色の数は最大次数 D)。多重辺を「辺と重複度」にまとめて持ち、D 正則の多重グラフを再帰で分ける。
//
// 頂点は、次数の和が D 以下になるように左右それぞれ先頭から詰めて組にまとめ (next fit)、組の間の辺を重複度付きの辺に
// まとめる。左右の組の数を揃え、足りない次数は組の間に重複度付きの辺を足して D 正則にする。
// 次数 d の部分を分けるとき、d が奇数なら完全マッチングを 1 つ取って色を 1 つ塗り、d を 1 減らす。d が偶数なら、重複度が
// 偶数の辺は半分ずつ両側へ、奇数の辺は 1 本を除いて半分ずつ両側へ送り、残った 1 本ずつの辺 (どの頂点でも本数が偶数) を
// Euler 閉路に沿って交互に両側へ振り分ける。両側はどちらも d / 2 正則になる。辺の種類が SMALL 以下なら、完全マッチングを
// 取ってその上の重複度の最小 δ だけ、色を δ 個まとめて塗る (Birkhoff-von Neumann の分解。1 回で辺の種類が 1 つは減る)。
// マッチングは NeoLibrary の BipartiteMatching (push-relabel)。最後に、まとめた辺ごとに塗った色を、元の辺へ順に配る。
#include <algorithm>
#include <array>
#include <vector>
#include "neo/graph/BipartiteMatching.hpp"
namespace bec_mult {
struct CE {
 int l, r, k, id;  // 左の組、右の組 (K + 番号)、重複度、まとめた辺の番号
};
template <int SMALL= 64> std::vector<int> edge_coloring(int L, int R, const std::vector<std::array<int, 2>>& e) {
 const int m= e.size();
 std::vector<int> col(m, 0);
 if(m == 0) return col;
 std::vector<int> dl(L), dr(R);
 for(auto [a, b]: e) ++dl[a], ++dr[b];
 const int D= std::max(*std::max_element(dl.begin(), dl.end()), *std::max_element(dr.begin(), dr.end()));
 // 組にまとめる (next fit)。
 std::vector<int> gl(L), gr(R);
 auto group= [&](const std::vector<int>& d, std::vector<int>& g) {
  int cnt= 0, load= D + 1;
  for(size_t v= 0; v < d.size(); ++v) {
   if(load + d[v] > D) ++cnt, load= 0;
   g[v]= cnt - 1, load+= d[v];
  }
  return cnt;
 };
 const int K= std::max(group(dl, gl), group(dr, gr));
 // まとめた辺。元の辺を組の対で並べ、同じ対を 1 本にする。
 // 右の組で数え上げて並べてから、左の組で安定に数え上げて並べる。
 std::vector<int> ord(m), tmp(m), cnt(K + 1);
 for(auto [a, b]: e) ++cnt[gr[b] + 1];
 for(int g= 0; g < K; ++g) cnt[g + 1]+= cnt[g];
 for(int i= 0; i < m; ++i) tmp[cnt[gr[e[i][1]]]++]= i;
 std::fill(cnt.begin(), cnt.end(), 0);
 for(auto [a, b]: e) ++cnt[gl[a] + 1];
 for(int g= 0; g < K; ++g) cnt[g + 1]+= cnt[g];
 for(int i: tmp) ord[cnt[gl[e[i][0]]]++]= i;
 std::vector<CE> ces;
 std::vector<int> head;  // まとめた辺 id の元の辺の ord の中の始まり (足した辺は -1)
 std::vector<int> degl(K), degr(K);
 for(int i= 0; i < m;) {
  int j= i;
  const int a= gl[e[ord[i]][0]], b= gr[e[ord[i]][1]];
  while(j < m && gl[e[ord[j]][0]] == a && gr[e[ord[j]][1]] == b) ++j;
  ces.push_back({a, K + b, j - i, (int)head.size()}), head.push_back(i);
  degl[a]+= j - i, degr[b]+= j - i;
  i= j;
 }
 const int real= head.size();
 for(int a= 0, b= 0; a < K; ++a)
  while(degl[a] < D) {
   while(degr[b] == D) ++b;
   const int x= std::min(D - degl[a], D - degr[b]);
   ces.push_back({a, K + b, x, (int)head.size()}), head.push_back(-1);
   degl[a]+= x, degr[b]+= x;
  }
 // 塗った色の記録 (まとめた辺 id、色 c、本数 cnt)。色 c から c + cnt - 1 を 1 本ずつに塗る。
 std::vector<std::array<int, 3>> rec;
 rec.reserve(ces.size() * 2);
 // 部分グラフの完全マッチング。使う辺の番号 (list の添字) を返す。
 auto perfect= [&](const std::vector<CE>& list) {
  std::vector<std::array<int, 2>> es(list.size());
  for(size_t i= 0; i < list.size(); ++i) es[i]= {list[i].l, list[i].r - K};
  BipartiteMatching bm(K, K, es);
  return bm.matching();
 };
 struct Task {
  int d, c;
  std::vector<CE> list;
 };
 std::vector<Task> stk;
 stk.push_back({D, 0, std::move(ces)});
 std::vector<int> off, adj, it, cyc, st2;
 std::vector<char> used;
 while(!stk.empty()) {
  Task t= std::move(stk.back());
  stk.pop_back();
  int d= t.d, c= t.c;
  std::vector<CE>& list= t.list;
  while(d > 0 && (int)list.size() <= SMALL) {
   // Birkhoff-von Neumann: 完全マッチングの上の重複度の最小だけ、まとめて塗る。
   const std::vector<int> mt= perfect(list);
   int delta= d;
   for(int i: mt) delta= std::min(delta, list[i].k);
   for(int i: mt) rec.push_back({list[i].id, c, delta}), list[i].k-= delta;
   c+= delta, d-= delta;
   list.erase(std::remove_if(list.begin(), list.end(), [](const CE& x) { return x.k == 0; }), list.end());
  }
  if(d == 0) continue;
  if(d == 1) {
   for(auto& x: list) rec.push_back({x.id, c, x.k});
   continue;
  }
  if(d & 1) {
   for(int i: perfect(list)) rec.push_back({list[i].id, c, 1}), --list[i].k;
   ++c, --d;
   list.erase(std::remove_if(list.begin(), list.end(), [](const CE& x) { return x.k == 0; }), list.end());
  }
  // d は偶数。重複度の半分ずつを両側へ送り、奇数の辺の 1 本ずつを Euler 閉路で振り分ける。
  std::vector<CE> A, B;
  A.reserve(list.size()), B.reserve(list.size());
  std::vector<int> odd;
  for(int i= 0; i < (int)list.size(); ++i) {
   const CE& x= list[i];
   if(x.k >= 2) A.push_back({x.l, x.r, x.k / 2, x.id}), B.push_back({x.l, x.r, x.k / 2, x.id});
   if(x.k & 1) odd.push_back(i);
  }
  if(!odd.empty()) {
   const int n2= 2 * K, no= odd.size();
   off.assign(n2 + 1, 0), adj.resize(2 * no), used.assign(no, 0);
   for(int i: odd) ++off[list[i].l + 1], ++off[list[i].r + 1];
   for(int v= 0; v < n2; ++v) off[v + 1]+= off[v];
   it.assign(off.begin(), off.end() - 1);
   for(int j= 0; j < no; ++j) adj[it[list[odd[j]].l]++]= j, adj[it[list[odd[j]].r]++]= j;
   it.assign(off.begin(), off.end() - 1);
   // Hierholzer。閉路の辺を出てくる順に並べ、交互に A と B へ。
   std::vector<char> side(no);
   for(int s= 0; s < n2; ++s) {
    if(it[s] == off[s + 1]) continue;
    cyc.clear(), st2.clear();
    st2.push_back(s), st2.push_back(-1);  // (頂点, 来た辺)
    while(!st2.empty()) {
     const int u= st2[st2.size() - 2], ein= st2.back();
     while(it[u] < off[u + 1] && used[adj[it[u]]]) ++it[u];
     if(it[u] == off[u + 1]) {
      if(ein >= 0) cyc.push_back(ein);
      st2.pop_back(), st2.pop_back();
     } else {
      const int j= adj[it[u]++];
      used[j]= 1;
      const CE& x= list[odd[j]];
      st2.push_back(x.l == u ? x.r : x.l), st2.push_back(j);
     }
    }
    for(size_t q= 0; q < cyc.size(); ++q) side[cyc[q]]= q & 1;
   }
   for(int j= 0; j < no; ++j) {
    const CE& x= list[odd[j]];
    (side[j] ? B : A).push_back({x.l, x.r, 1, x.id});
   }
  }
  std::vector<CE>().swap(list);
  stk.push_back({d / 2, c + d / 2, std::move(B)});
  stk.push_back({d / 2, c, std::move(A)});
 }
 // 塗った色を、まとめた辺ごとに元の辺へ配る (同じ辺の色の順はどうでもよい)。
 std::vector<int> fill(head.size());
 for(auto [id, c, k]: rec) {
  if(id >= real) continue;
  const int lo= head[id], hi= id + 1 < real ? head[id + 1] : m;
  for(int q= 0; q < k && lo + fill[id] < hi; ++q) col[ord[lo + fill[id]++]]= c + q;
 }
 return col;
}
}
