#pragma once
// 一般グラフの重み最大マッチング。wm_key.hpp の偶の頂点の行の走査を AVX2 で 4 列ずつ回す。
//
// 走査では、花に入っていない元の頂点 v について、縮約費用が 0 なら木を伸ばすか花を縮め、そうでなければ key[v] を小さく
// する。0 の列を含む 4 列はスカラーで回し、そうでなければ比べて書き換える列を選ぶ。花の中の頂点は列ごとには見ず、
// 走査の終わりに、頂点の花ごとに代表の辺 e(u, x) で同じことをする。花の中の頂点の双対は一緒に動くので、u から花への
// 辺のうち縮約費用が最小のものは、花を作ったときに選んだ代表のまま変わらない。
//
// 以下は wm_key.hpp の説明。
// 一般グラフの重み最大マッチング。wm_std.hpp と同じ O(n^3) の主双対の花の方法で、元の頂点の slack を値で持つ。
//
// wm_std では、頂点 x の slack を「縮約費用が最小の偶の頂点 slack[x]」で持ち、比べるたびに行列の列 e(slack[x], x) を
// 引くので、キャッシュを外す。ここでは元の頂点 x (花でない) について、その縮約費用を key[x] に値で持つ。その段で動かした
// 双対の和を T とすると、偶の頂点から印の無い頂点への辺の縮約費用は T が増えるだけ減り、偶どうしの辺は 2 倍減るので、
// key には印の無い頂点なら縮約費用 + T、偶なら縮約費用 + 2T を入れておけば、段の中で値が変わらない。印が変わるときは
// どれも slack を付け直すので、そのときに key も付け直す。花の slack は wm_std と同じく行列で持つ。
// 元の頂点どうしの重みは (n + 1)^2 の行列 wv に別に持ち、走査はその行を読む。
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#endif
#include <algorithm>
#include <array>
#include <vector>
namespace wm_avx {
struct Solver {
 using i64= long long;
 static constexpr i64 INF= (i64)1 << 62;
 struct E {
  int u, v;
  i64 w;
 };
 int n, nx, N2, n1;
 std::vector<E> g;  // (2n + 1)^2。花の行と列だけを使う
 std::vector<i64> wv, lab, key;
 std::vector<int> match, slack, st, pa, ffrom, S, vis, q;
 std::vector<std::vector<int>> flo;
 int qh= 0, tvis= 0;
 i64 T= 0;
 E& e(int u, int v) { return g[u * N2 + v]; }
 int& ff(int b, int x) { return ffrom[b * n1 + x]; }
 i64 w0(int u, int v) const { return wv[u * n1 + v]; }
 i64 dlt(const E& x) const { return lab[x.u] + lab[x.v] - w0(x.u, x.v) * 2; }
 i64 cf(int x) const { return S[x] == 0 ? 2 : 1; }
 explicit Solver(int n): n(n), nx(n), N2(2 * n + 1), n1(n + 1), g((size_t)N2 * N2), wv((size_t)n1 * n1), lab(N2), key(n1, INF), match(N2), slack(N2), st(N2), pa(N2), ffrom((size_t)N2 * n1), S(N2), vis(N2), flo(N2) {
  for(int u= 1; u <= n; ++u)
   for(int v= 1; v <= n; ++v) e(u, v)= E{u, v, 0};
 }
 void add_edge(int u, int v, i64 w) {  // 0 始まり
  ++u, ++v;
  if(w > w0(u, v)) e(u, v).w= e(v, u).w= w, wv[u * n1 + v]= wv[v * n1 + u]= w;
 }
 void reset_slack(int x) {
  slack[x]= 0;
  if(x <= n) key[x]= INF;
 }
 void update_slack(int u, int x) {
  if(x <= n) {
   if(const i64 c= lab[u] + lab[x] - 2 * w0(u, x) + cf(x) * T; c < key[x]) key[x]= c, slack[x]= u;
  } else if(!slack[x] || dlt(e(u, x)) < dlt(e(slack[x], x))) slack[x]= u;
 }
 void set_slack(int x) {
  reset_slack(x);
  if(x <= n) {
   for(int u= 1; u <= n; ++u)
    if(w0(x, u) > 0 && st[u] != x && S[st[u]] == 0) update_slack(u, x);
  } else
   for(int u= 1; u <= n; ++u)
    if(e(u, x).w > 0 && st[u] != x && S[st[u]] == 0) update_slack(u, x);
 }
 // 頂点 x の slack の今の縮約費用
 i64 slack_value(int x) const { return x <= n ? key[x] - cf(x) * T : dlt(g[slack[x] * N2 + x]); }
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
    if(e(xs, x).w > 0 && (e(b, x).w == 0 || dlt(e(xs, x)) < dlt(e(b, x)))) e(b, x)= e(xs, x), e(x, b)= e(x, xs);
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
   reset_slack(xs), set_slack(xns);
   q_push(xns);
  }
  S[xr]= 1, pa[xr]= pa[b];
  reset_slack(xr);
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
   reset_slack(v), reset_slack(nu);
   S[nu]= 0, q_push(nu);
  } else if(S[v] == 0) {
   const int lca= get_lca(u, v);
   if(!lca) return augment(u, v), augment(v, u), true;
   add_blossom(u, lca, v);
  }
  return false;
 }
 bool matching() {
  std::fill(S.begin() + 1, S.begin() + nx + 1, -1);
  std::fill(slack.begin() + 1, slack.begin() + nx + 1, 0);
  std::fill(key.begin(), key.end(), INF);
  T= 0;
  q.clear(), qh= 0;
  for(int x= 1; x <= nx; ++x)
   if(st[x] == x && !match[x]) pa[x]= 0, S[x]= 0, q_push(x);
  if(q.empty()) return false;
  for(;;) {
   while(qh < (int)q.size()) {
    const int u= q[qh++];
    if(S[st[u]] == 1) continue;
    const i64* wr= &wv[u * n1];
    const i64 lu= lab[u];
    // 1 列をスカラーで見る。花の中の頂点は飛ばす (後でまとめて見る)。
    auto one= [&](int v) {
     if(wr[v] > 0 && st[v] == v && st[u] != v) {
      if(lu + lab[v] - 2 * wr[v] == 0) return on_found_edge(E{u, v, wr[v]});
      update_slack(u, v);
     }
     return false;
    };
    const __m256i vlu= _mm256_set1_epi64x(lu), vT= _mm256_set1_epi64x(T), zero= _mm256_setzero_si256();
    int v= 1;
    for(; v + 3 <= n; v+= 4) {
     const __m256i w= _mm256_loadu_si256((const __m256i*)(wr + v));
     const __m256i lv= _mm256_loadu_si256((const __m256i*)(lab.data() + v));
     const __m128i id4= _mm_add_epi32(_mm_set1_epi32(v), _mm_setr_epi32(0, 1, 2, 3));
     const __m128i st4= _mm_loadu_si128((const __m128i*)(st.data() + v));
     const __m128i s04= _mm_cmpeq_epi32(_mm_loadu_si128((const __m128i*)(S.data() + v)), _mm_setzero_si128());
     const __m256i act= _mm256_and_si256(_mm256_cmpgt_epi64(w, zero), _mm256_cvtepi32_epi64(_mm_cmpeq_epi32(st4, id4)));
     const __m256i dd= _mm256_sub_epi64(_mm256_add_epi64(vlu, lv), _mm256_add_epi64(w, w));
     if(!_mm256_testz_si256(act, _mm256_cmpeq_epi64(dd, zero))) {
      for(int j= 0; j < 4; ++j)
       if(one(v + j)) return true;
      continue;
     }
     const __m256i cand= _mm256_add_epi64(_mm256_add_epi64(dd, vT), _mm256_and_si256(vT, _mm256_cvtepi32_epi64(s04)));
     const __m256i kv= _mm256_loadu_si256((const __m256i*)(key.data() + v));
     const __m256i upd= _mm256_and_si256(act, _mm256_cmpgt_epi64(kv, cand));
     if(_mm256_testz_si256(upd, upd)) continue;
     _mm256_storeu_si256((__m256i*)(key.data() + v), _mm256_blendv_epi8(kv, cand, upd));
     for(int m= _mm256_movemask_pd(_mm256_castsi256_pd(upd)); m; m&= m - 1) slack[v + __builtin_ctz(m)]= u;
    }
    for(; v <= n; ++v)
     if(one(v)) return true;
    // 頂点の花。内側の花への辺は使わない。
    for(int b= n + 1; b <= nx; ++b)
     if(st[b] == b && S[b] != 1 && st[u] != b && e(u, b).w > 0) {
      if(dlt(e(u, b)) == 0) {
       if(on_found_edge(e(u, b))) return true;
      } else update_slack(u, b);
     }
   }
   i64 d= INF;
   for(int b= n + 1; b <= nx; ++b)
    if(st[b] == b && S[b] == 1) d= std::min(d, lab[b] / 2);
   for(int x= 1; x <= nx; ++x)
    if(st[x] == x && slack[x]) {
     if(S[x] == -1) d= std::min(d, slack_value(x));
     else if(S[x] == 0) d= std::min(d, slack_value(x) / 2);
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
   T+= d;
   q.clear(), qh= 0;
   for(int x= 1; x <= nx; ++x)
    if(st[x] == x && slack[x] && st[slack[x]] != x && S[x] != 1 && slack_value(x) == 0) {
     const E ed= x <= n ? E{slack[x], x, w0(slack[x], x)} : e(slack[x], x);
     if(on_found_edge(ed)) return true;
    }
   for(int b= n + 1; b <= nx; ++b)
    if(st[b] == b && S[b] == 1 && lab[b] == 0) expand_blossom(b);
  }
 }
 std::vector<int> solve() {
  nx= n;
  for(int u= 0; u <= n; ++u) st[u]= u, flo[u].clear();
  i64 wmax= 0;
  for(int u= 1; u <= n; ++u)
   for(int v= 1; v <= n; ++v) ff(u, v)= (u == v ? u : 0), wmax= std::max(wmax, w0(u, v));
  for(int u= 1; u <= n; ++u) lab[u]= wmax;
  while(matching());
  std::vector<int> mt(n, -1);
  for(int u= 1; u <= n; ++u)
   if(match[u]) mt[u - 1]= match[u] - 1;
  return mt;
 }
};
}
