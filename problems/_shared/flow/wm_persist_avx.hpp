#pragma once
// 一般グラフの重み最大マッチング。wm_persist.hpp の、元の頂点についての 4 つの走査を AVX2 で 4 列ずつ回したもの。
// 偶の頂点の行の走査、元の頂点の slack の付け直し、双対の動かし方を決める最小、動かしたあとのきつい slack の検出。
// 花についての部分はスカラーのまま。
//
// 以下は wm_persist.hpp の説明。
// 一般グラフの重み最大マッチング。wm_key.hpp と同じ主双対の花の方法で、交互森を増加路ごとに作り直さずに保つ。
//
// wm_std / wm_key は、増加路を流すたびに印をすべて消し、空いている頂点を根に森を作り直して、偶の頂点の行を全部走査し
// 直す。ここでは、流した 2 本の木 (根 A と B) の印だけを消し、ほかの木はそのまま残す。流して組が変わるのは A と B の
// 中だけなので、ほかの木は正しい交互木のままで、双対も変わらない。A と B の頂点は印の無い頂点になり、生きている木の
// 偶の頂点からの slack を付け直す (縮約費用が 0 ならすぐ木を伸ばす)。
//
// 双対は、全体で動かした量の和 T を使って遅延で持つ。頂点 v の双対 (の 2 倍) は lb[v] + sv[v] T で、sv は偶が -1、
// 奇が +1、印が無ければ 0。頂点の花 b は lb[b] + sv[b] T で、sv は偶が +2、奇が -2、それ以外は 0。印が変わるときに
// lb を付け直すので、双対を動かす手間は T を足すだけになる。元の頂点 x の slack は wm_key と同じく key[x] に、印が無ければ
// 縮約費用 + T、偶なら縮約費用 + 2T で持つ。木を壊すと、壊した木の偶の頂点を指す slack が古くなる。古い slack は本当の値
// 以下なので、双対の動かし方を決めるときに最小を与えたものだけを確かめ、古ければ付け直す。
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#endif
#include <algorithm>
#include <array>
#include <vector>
namespace wm_persist_avx {
struct Solver {
 using i64= long long;
 static constexpr i64 INF= (i64)1 << 62;
 struct E {
  int u, v;
 };
 int n, nx, N2, n1;
 std::vector<E> g;  // (2n + 1)^2。花の代表の辺 (両端は元の頂点)
 std::vector<i64> wv, lb, key;
 std::vector<int> sv, match, slack, st, pa, ffrom, S, vis, q, rt, cand, dead, osince, kst;
 std::vector<std::vector<int>> flo;
 int qh= 0, tvis= 0, clk= 0;
 i64 T= 0;
 E& e(int u, int v) { return g[u * N2 + v]; }
 int& ff(int b, int x) { return ffrom[b * n1 + x]; }
 i64 w0(int u, int v) const { return wv[u * n1 + v]; }
 i64 lab(int x) const { return lb[x] + sv[x] * T; }
 i64 dlt(const E& x) const { return lab(x.u) + lab(x.v) - w0(x.u, x.v) * 2; }
 i64 cf(int x) const { return S[x] == 0 ? 2 : 1; }
 explicit Solver(int n): n(n), nx(n), N2(2 * n + 1), n1(n + 1), g((size_t)N2 * N2), wv((size_t)n1 * n1), lb(N2), key(N2, INF), sv(N2), match(N2), slack(N2), st(N2), pa(N2), ffrom((size_t)N2 * n1), S(N2, -1), vis(N2), rt(N2), osince(n1), kst(N2), flo(N2) {
  for(int u= 1; u <= n; ++u)
   for(int v= 1; v <= n; ++v) e(u, v)= E{u, v};
 }
 void add_edge(int u, int v, i64 w) {  // 0 始まり
  ++u, ++v;
  if(w > w0(u, v)) wv[u * n1 + v]= wv[v * n1 + u]= w;
 }
 // 頂点の印を変えるときに、中の頂点の双対の持ち方を付け直す。c は頂点の係数 (-1, 0, +1)。偶になった時刻を osince に残す。
 void set_vertex_coef(int x, int c) {
  if(x <= n) {
   if(c == -1 && sv[x] != -1) osince[x]= ++clk;
   lb[x]= lab(x) - c * T, sv[x]= c;
   return;
  }
  for(int y: flo[x]) set_vertex_coef(y, c);
 }
 // 頂点の花 b の双対の係数を付け直す (c は -2, 0, +2)。
 void set_blossom_coef(int b, int c) {
  if(b > n) lb[b]= lab(b) - c * T, sv[b]= c;
 }
 // 頂点 (または花) x の印を s にする。双対の係数も付け直す。
 void set_label(int x, int s) {
  S[x]= s;
  set_vertex_coef(x, s == 0 ? -1 : s == 1 ? 1 : 0);
  set_blossom_coef(x, s == 0 ? 2 : s == 1 ? -2 : 0);
 }
 // slack[x] が、記録したときからずっと偶のままの頂点を指しているか。途中で偶でなくなった頂点は、縮約費用が想定どおりに
 // 減っていないので、偶に戻っていても古い。
 bool slack_valid(int x) const { return S[st[slack[x]]] == 0 && st[slack[x]] != x && osince[slack[x]] <= kst[x]; }
 // slack は元の頂点と花のどちらも、縮約費用に cf(x) T を足した値 key[x] で持つ。偶のままの頂点からの値は変わらない。
 void reset_slack(int x) { slack[x]= 0, key[x]= INF; }
 i64 edge_d(int u, int x) const { return x <= n ? lab(u) + lab(x) - 2 * w0(u, x) : dlt(g[u * N2 + x]); }
 // 古い slack と比べて小さければ書き換えてよい。古い slack の値は、それに負けた偶のままの頂点のどれの値よりも小さいので、
 // それより小さい値は本当の最小になる。大きければ捨てる (古い slack は、双対の動かし方を決めるときに付け直す)。
 void update_slack(int u, int x) {
  if(const i64 c= edge_d(u, x) + cf(x) * T; c < key[x]) key[x]= c, slack[x]= u, kst[x]= clk;
 }
 static __m256i ld(const i64* p) { return _mm256_loadu_si256((const __m256i*)p); }
 static __m256i ld32(const int* p) { return _mm256_cvtepi32_epi64(_mm_loadu_si128((const __m128i*)p)); }
 // 4 列の最小とその列を、スカラーの値と合わせて返す。
 static void reduce_min(__m256i bv, __m256i bi, i64& b, int& bx) {
  alignas(32) i64 v[4], id[4];
  _mm256_store_si256((__m256i*)v, bv), _mm256_store_si256((__m256i*)id, bi);
  for(int j= 0; j < 4; ++j)
   if(v[j] < b) b= v[j], bx= (int)id[j];
 }
 void set_slack(int x) {
  reset_slack(x);
  if(x <= n) {
   // 偶の頂点 u は sv[u] == -1 で、lab(u) = lb[u] - T。x と同じ頂点に入る u は x 自身だけで、自己ループは無い。
   const i64* wr= &wv[x * n1];
   const i64 base= lab(x) - T + cf(x) * T;
   const __m256i vb= _mm256_set1_epi64x(base), zero= _mm256_setzero_si256(), m1= _mm256_set1_epi64x(-1);
   __m256i bv= _mm256_set1_epi64x(INF), bi= zero, id= _mm256_setr_epi64x(1, 2, 3, 4);
   const __m256i four= _mm256_set1_epi64x(4);
   int u= 1;
   for(; u + 3 <= n; u+= 4, id= _mm256_add_epi64(id, four)) {
    const __m256i w= ld(wr + u);
    const __m256i act= _mm256_and_si256(_mm256_cmpgt_epi64(w, zero), _mm256_cmpeq_epi64(ld32(sv.data() + u), m1));
    const __m256i val= _mm256_sub_epi64(_mm256_add_epi64(ld(lb.data() + u), vb), _mm256_add_epi64(w, w));
    const __m256i better= _mm256_and_si256(act, _mm256_cmpgt_epi64(bv, val));
    bv= _mm256_blendv_epi8(bv, val, better), bi= _mm256_blendv_epi8(bi, id, better);
   }
   i64 b= INF;
   int bx= 0;
   reduce_min(bv, bi, b, bx);
   for(; u <= n; ++u)
    if(wr[u] > 0 && sv[u] == -1)
     if(const i64 val= lb[u] + base - 2 * wr[u]; val < b) b= val, bx= u;
   if(bx) key[x]= b, slack[x]= bx, kst[x]= clk;
  } else
   for(int u= 1; u <= n; ++u)
    if(const E& ex= e(u, x); w0(ex.u, ex.v) > 0 && st[u] != x && S[st[u]] == 0) update_slack(u, x);
 }
 i64 slack_value(int x) const { return key[x] - cf(x) * T; }
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
  match[b]= match[lca];
  flo[b].clear();
  flo[b].push_back(lca);
  for(int x= u, y; x != lca; x= st[pa[y]]) flo[b].push_back(x), flo[b].push_back(y= st[match[x]]), q_push(y);
  std::reverse(flo[b].begin() + 1, flo[b].end());
  for(int x= v, y; x != lca; x= st[pa[y]]) flo[b].push_back(x), flo[b].push_back(y= st[match[x]]), q_push(y);
  // 中に入る花の双対は止め、中の頂点はすべて偶にする。新しい花の双対は 0 から始める。
  for(int x: flo[b]) set_blossom_coef(x, 0), set_vertex_coef(x, -1), S[x]= -1;
  rt[b]= rt[lca], S[b]= 0, sv[b]= 2, lb[b]= -2 * T;
  set_st(b, b);
  for(int x= 1; x <= nx; ++x) e(b, x)= e(x, b)= E{0, 0};
  for(int x= 1; x <= n; ++x) ff(b, x)= 0;
  for(int xs: flo[b]) {
   for(int x= 1; x <= nx; ++x) {
    const E& c= e(xs, x);
    if(!c.u || w0(c.u, c.v) == 0) continue;
    if(const E& d= e(b, x); !d.u || dlt(c) < dlt(d)) e(b, x)= c, e(x, b)= e(x, xs);
   }
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
   set_label(xs, 1), set_label(xns, 0), rt[xs]= rt[xns]= rt[b];
   reset_slack(xs), set_slack(xns);
   q_push(xns);
  }
  set_label(xr, 1), rt[xr]= rt[b], pa[xr]= pa[b];
  reset_slack(xr);
  for(size_t i= pr + 1; i < flo[b].size(); ++i) {
   const int xs= flo[b][i];
   set_label(xs, -1), set_slack(xs);
  }
  st[b]= 0, S[b]= -1;
 }
 // 増加路を流したら、両端の木の根を dead に入れて true を返す。
 bool on_found_edge(const E& x) {
  const int u= st[x.u], v= st[x.v];
  if(S[v] == -1) {
   pa[v]= x.u, set_label(v, 1), rt[v]= rt[u];
   const int nu= st[match[v]];
   reset_slack(v), reset_slack(nu);
   set_label(nu, 0), rt[nu]= rt[u], q_push(nu);
  } else if(S[v] == 0) {
   const int lca= get_lca(u, v);
   if(!lca) {
    dead.push_back(rt[u]), dead.push_back(rt[v]);
    return augment(u, v), augment(v, u), true;
   }
   add_blossom(u, lca, v);
  }
  return false;
 }
 // 流した 2 本の木を壊す。印を消してから、生きている木の偶の頂点からの slack を付け直し、きつい辺があれば伸ばす。
 void dissolve() {
  const int ra= dead[0], rb= dead[1];
  dead.clear();
  cand.clear();
  for(int x= 1; x <= nx; ++x)
   if(st[x] == x && S[x] != -1 && (rt[x] == ra || rt[x] == rb)) set_label(x, -1), reset_slack(x), cand.push_back(x);
  for(int x: cand) {
   if(S[x] != -1) continue;  // 先の頂点から伸びた木に拾われた
   set_slack(x);
   if(slack[x] && slack_value(x) == 0) on_found_edge(x <= n ? E{slack[x], x} : e(slack[x], x));
  }
 }
 // 偶の頂点 u の行を走査する。増加路を流したら true。
 bool scan(int u) {
  const i64* wr= &wv[u * n1];
  const i64 lu= lab(u);
  auto one= [&](int v) {
   if(wr[v] > 0 && st[v] == v && st[u] != v && S[v] != 1) {
    if(lu + lab(v) - 2 * wr[v] == 0) return on_found_edge(E{u, v});
    update_slack(u, v);
   }
   return false;
  };
  const __m256i vlu= _mm256_set1_epi64x(lu), vT= _mm256_set1_epi64x(T), zero= _mm256_setzero_si256();
  const __m256i one64= _mm256_set1_epi64x(1), m1= _mm256_set1_epi64x(-1);
  int v= 1;
  for(; v + 3 <= n; v+= 4) {
   const __m256i w= ld(wr + v), s4= ld32(sv.data() + v), S4= ld32(S.data() + v);
   const __m256i id= _mm256_add_epi64(_mm256_set1_epi64x(v), _mm256_setr_epi64x(0, 1, 2, 3));
   const __m256i labv= _mm256_sub_epi64(_mm256_add_epi64(ld(lb.data() + v), _mm256_and_si256(_mm256_cmpeq_epi64(s4, one64), vT)), _mm256_and_si256(_mm256_cmpeq_epi64(s4, m1), vT));
   const __m256i dd= _mm256_sub_epi64(_mm256_add_epi64(vlu, labv), _mm256_add_epi64(w, w));
   const __m256i act= _mm256_andnot_si256(_mm256_cmpeq_epi64(S4, one64), _mm256_and_si256(_mm256_cmpgt_epi64(w, zero), _mm256_cmpeq_epi64(ld32(st.data() + v), id)));
   if(!_mm256_testz_si256(act, _mm256_cmpeq_epi64(dd, zero))) {
    for(int j= 0; j < 4; ++j)
     if(one(v + j)) return true;
    continue;
   }
   const __m256i cand= _mm256_add_epi64(_mm256_add_epi64(dd, vT), _mm256_and_si256(_mm256_cmpeq_epi64(S4, zero), vT));
   const __m256i kv= ld(key.data() + v);
   const __m256i upd= _mm256_and_si256(act, _mm256_cmpgt_epi64(kv, cand));
   if(_mm256_testz_si256(upd, upd)) continue;
   _mm256_storeu_si256((__m256i*)(key.data() + v), _mm256_blendv_epi8(kv, cand, upd));
   for(int m= _mm256_movemask_pd(_mm256_castsi256_pd(upd)); m; m&= m - 1) slack[v + __builtin_ctz(m)]= u, kst[v + __builtin_ctz(m)]= clk;
  }
  for(; v <= n; ++v)
   if(one(v)) return true;
  for(int b= n + 1; b <= nx; ++b)
   if(st[b] == b && S[b] != 1 && st[u] != b && e(u, b).u) {
    if(dlt(e(u, b)) == 0) {
     if(on_found_edge(e(u, b))) return true;
    } else update_slack(u, b);
   }
  return false;
 }
 std::vector<int> solve() {
  nx= n;
  for(int u= 0; u <= n; ++u) st[u]= u, flo[u].clear();
  i64 wmax= 0;
  for(int u= 1; u <= n; ++u)
   for(int v= 1; v <= n; ++v) ff(u, v)= (u == v ? u : 0), wmax= std::max(wmax, w0(u, v));
  for(int u= 1; u <= n; ++u) lb[u]= wmax, rt[u]= u, pa[u]= 0, set_label(u, 0), q.push_back(u);
  for(;;) {
   while(qh < (int)q.size()) {
    const int u= q[qh++];
    if(S[st[u]] != 0) continue;
    if(scan(u)) dissolve();
   }
   // 双対の動かし方。slack は 2 倍の尺度で比べる (印の無い頂点への辺は縮約費用の 2 倍、偶どうしは縮約費用、奇の花は双対)。
   i64 best= INF;
   // 元の頂点の最小。最小を与えたものが古ければ付け直してやり直す (古い slack は本当の値以下)。
   for(;;) {
    const __m256i vT= _mm256_set1_epi64x(T), zero= _mm256_setzero_si256(), one64= _mm256_set1_epi64x(1), m1= _mm256_set1_epi64x(-1);
    const __m256i vinf= _mm256_set1_epi64x(INF), four= _mm256_set1_epi64x(4);
    __m256i bv= vinf, bi= zero, id= _mm256_setr_epi64x(1, 2, 3, 4);
    int x= 1;
    for(; x + 3 <= n; x+= 4, id= _mm256_add_epi64(id, four)) {
     const __m256i S4= ld32(S.data() + x), kv= ld(key.data() + x);
     const __m256i act= _mm256_andnot_si256(_mm256_cmpeq_epi64(S4, one64), _mm256_and_si256(_mm256_cmpeq_epi64(ld32(st.data() + x), id), _mm256_cmpgt_epi64(vinf, kv)));
     const __m256i val= _mm256_sub_epi64(_mm256_sub_epi64(kv, vT), _mm256_and_si256(_mm256_cmpeq_epi64(S4, zero), vT));
     const __m256i sc= _mm256_add_epi64(val, _mm256_and_si256(_mm256_cmpeq_epi64(S4, m1), val));
     const __m256i better= _mm256_and_si256(act, _mm256_cmpgt_epi64(bv, sc));
     bv= _mm256_blendv_epi8(bv, sc, better), bi= _mm256_blendv_epi8(bi, id, better);
    }
    i64 b= INF;
    int bx= 0;
    reduce_min(bv, bi, b, bx);
    for(; x <= n; ++x)
     if(st[x] == x && S[x] != 1 && slack[x])
      if(const i64 val= slack_value(x) * (S[x] == -1 ? 2 : 1); val < b) b= val, bx= x;
    if(bx && !slack_valid(bx)) {
     set_slack(bx);
     continue;
    }
    best= b;
    break;
   }
   for(int x= n + 1; x <= nx; ++x) {
    if(st[x] != x || S[x] == 1 || !slack[x]) continue;
    i64 val= slack_value(x) * (S[x] == -1 ? 2 : 1);
    if(val >= best) continue;
    if(!slack_valid(x)) {
     set_slack(x);
     if(!slack[x]) continue;
     val= slack_value(x) * (S[x] == -1 ? 2 : 1);
     if(val >= best) continue;
    }
    best= val;
   }
   for(int b= n + 1; b <= nx; ++b)
    if(st[b] == b && S[b] == 1) best= std::min(best, lab(b));
   const i64 d= best / 2;
   // 偶の頂点の双対の最小 (lb - T)。これが d 以下なら終わり。偶の頂点が無ければ空いている頂点も無い。
   {
    const __m256i m1= _mm256_set1_epi64x(-1);
    __m256i mv= _mm256_set1_epi64x(INF);
    int u= 1;
    for(; u + 3 <= n; u+= 4) {
     const __m256i l= ld(lb.data() + u);
     const __m256i act= _mm256_and_si256(_mm256_cmpeq_epi64(ld32(sv.data() + u), m1), _mm256_cmpgt_epi64(mv, l));
     mv= _mm256_blendv_epi8(mv, l, act);
    }
    i64 mlb= INF;
    int dummy= 0;
    reduce_min(mv, _mm256_setzero_si256(), mlb, dummy);
    for(; u <= n; ++u)
     if(sv[u] == -1) mlb= std::min(mlb, lb[u]);
    if(mlb >= INF || best >= INF || mlb - T <= d) break;
   }
   T+= d;
   {
    const __m256i vT= _mm256_set1_epi64x(T), zero= _mm256_setzero_si256(), one64= _mm256_set1_epi64x(1);
    int x= 1;
    auto one= [&](int y) {
     if(st[y] == y && S[y] != 1 && slack[y] && slack_value(y) == 0 && slack_valid(y))
      if(on_found_edge(y <= n ? E{slack[y], y} : e(slack[y], y))) dissolve();
    };
    for(; x + 3 <= n; x+= 4) {
     const __m256i S4= ld32(S.data() + x);
     const __m256i val= _mm256_sub_epi64(_mm256_sub_epi64(ld(key.data() + x), vT), _mm256_and_si256(_mm256_cmpeq_epi64(S4, zero), vT));
     const __m256i hit= _mm256_andnot_si256(_mm256_cmpeq_epi64(S4, one64), _mm256_cmpeq_epi64(val, zero));
     for(int m= _mm256_movemask_pd(_mm256_castsi256_pd(hit)); m; m&= m - 1) one(x + __builtin_ctz(m));
    }
    for(; x <= nx; ++x) one(x);
   }
   for(int b= n + 1; b <= nx; ++b)
    if(st[b] == b && S[b] == 1 && lab(b) == 0) expand_blossom(b);
  }
  std::vector<int> mt(n, -1);
  for(int u= 1; u <= n; ++u)
   if(match[u]) mt[u - 1]= match[u] - 1;
  return mt;
 }
};
}
