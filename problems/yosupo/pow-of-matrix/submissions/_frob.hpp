#pragma once
// Frobenius 標準形で A^K を求める (Library Checker の首位の提出 401296 と同じ考え方を、ここの核で書いたもの)。
// 乱数のベクトル v から Krylov の鎖 v, Av, A^2 v, ... を作り、それまでに集めたベクトルの張る空間に入ったところで止める。
// 止まったときの関係 A^d v + sum_{i<d} f_i A^i v = w (w は前の鎖の張る空間の元) から、鎖の多項式 f が分かる。w が 0 で
// なければ、前の鎖ごとに w の成分 g_c を f で割り (乱数がうまく選べていれば割り切れる)、u = sum_c h_c(A) v_c を引いた
// v' = v - u から鎖を作り直す。f(A) v' = 0 なので、作り直した鎖は A で閉じる。全体を鎖に分けると、鎖を並べた基底 T で
// A = T F T^{-1} (F は同伴行列を並べたブロック対角) になり、A^K = T F^K T^{-1}。F^K のブロックの列 j は x^{K+j} mod f。
// 鎖のベクトルは、要の位置で正規化した掃き出しの行 [e | α] (e = sum_j α_j t_j) として持ち、新しいベクトルを入れるたびに
// 前の行で消す (和は u64 にためて 8 回ごとに畳む)。T^{-1} の列 piv_k は、e の部分を後ろから単位ベクトルに直したときの
// α の部分。最後の 2 回の積は _tile.hpp の核。割り切れなければ乱数を変えてやり直し、4 回駄目なら二分累乗にする。
#include <chrono>
#include "_tile.hpp"
#include "neo/algebra/ZMod.hpp"
#ifndef USE_SIMDE
#include <immintrin.h>
#endif
namespace frob {
using Z= ZMod<MOD>;
constexpr u32 M= MOD;
constexpr u64 R32= (u64(1) << 32) % M;
inline u64 fold(u64 x) { return (x >> 32) * R32 + u32(x); }
// y[i] += c x[i] (i < len)。x[i] < M、c < M。
inline void axpy(u64* __restrict y, const u32* __restrict x, u32 c, int len) {
 const __m256i C= _mm256_set1_epi64x(c);
 int i= 0;
 for(; i + 4 <= len; i+= 4) {
  const __m256i v= _mm256_cvtepu32_epi64(_mm_loadu_si128((const __m128i*)(x + i)));
  _mm256_storeu_si256((__m256i*)(y + i), _mm256_add_epi64(_mm256_loadu_si256((const __m256i*)(y + i)), _mm256_mul_epu32(v, C)));
 }
 for(; i < len; ++i) y[i]+= u64(c) * x[i];
}
inline void fold_all(u64* y, int len) {
 const __m256i Rv= _mm256_set1_epi64x(R32), LO= _mm256_set1_epi64x(0xffffffffll);
 int i= 0;
 for(; i + 4 <= len; i+= 4) {
  const __m256i v= _mm256_loadu_si256((const __m256i*)(y + i));
  _mm256_storeu_si256((__m256i*)(y + i), _mm256_add_epi64(_mm256_mul_epu32(_mm256_srli_epi64(v, 32), Rv), _mm256_and_si256(v, LO)));
 }
 for(; i < len; ++i) y[i]= fold(y[i]);
}
// sum_i x[i] y[i] mod M (x[i], y[i] < M)。
inline u32 dot(const u32* x, const u32* y, int n) {
 const __m256i Rv= _mm256_set1_epi64x(R32), LO= _mm256_set1_epi64x(0xffffffffll);
 __m256i acc= _mm256_setzero_si256();
 int i= 0, t= 0;
 for(; i + 4 <= n; i+= 4) {
  acc= _mm256_add_epi64(acc, _mm256_mul_epu32(_mm256_cvtepu32_epi64(_mm_loadu_si128((const __m128i*)(x + i))), _mm256_cvtepu32_epi64(_mm_loadu_si128((const __m128i*)(y + i)))));
  if(++t == 8) acc= _mm256_add_epi64(_mm256_mul_epu32(_mm256_srli_epi64(acc, 32), Rv), _mm256_and_si256(acc, LO)), t= 0;
 }
 alignas(32) u64 s[4];
 _mm256_store_si256((__m256i*)s, acc);
 u64 r= s[0] % M + s[1] % M + s[2] % M + s[3] % M;
 for(; i < n; ++i) r+= u64(x[i]) * y[i] % M;
 return u32(r % M);
}
// x^K mod χ (zmod_ps.hpp の写し)。
// t[0..len) (畳んだ u64) を、χ (monic、次数 d、χ[d] = 1) で割った余りに直して、r[0..d) (M 未満) に書く。
inline void reduce(vector<u64>& t, int len, const vector<u32>& nchi, int d, vector<u32>& r) {
 int cnt= 0;
 for(int k= len - 1; k >= d; --k) {
  const u32 c= u32(t[k] % MOD);
  if(c) {
   u64* q= &t[k - d];
   for(int j= 0; j < d; ++j) q[j]+= u64(c) * nchi[j];  // nchi[j] = M - χ[j] (mod M)
  }
  if(++cnt == 8) {
   for(int j= max(0, k - d); j < k; ++j) t[j]= fold(t[j]);
   cnt= 0;
  }
 }
 for(int j= 0; j < d; ++j) r[j]= u32(t[j] % MOD);
}
// x^K mod χ (K >= 1)。
inline vector<u32> xpow(u64 K, const vector<u32>& chi, int d) {
 vector<u32> nchi(d), r(d);
 for(int j= 0; j < d; ++j) nchi[j]= chi[j] ? MOD - chi[j] : 0;
 vector<u64> t(2 * d);
 r[0]= 1;
 for(int b= 63 - __builtin_clzll(K); b >= 0; --b) {
  // r = r^2 mod χ
  fill(t.begin(), t.end(), 0);
  int cnt= 0;
  for(int i= 0; i < d; ++i) {
   if(!r[i]) continue;
   const u64 a= r[i];
   u64* q= &t[i];
   for(int j= 0; j < d; ++j) q[j]+= a * r[j];
   if(++cnt == 8) {
    for(auto& v : t) v= fold(v);
    cnt= 0;
   }
  }
  for(auto& v : t) v= fold(v);
  reduce(t, 2 * d - 1, nchi, d, r);
  if(K >> b & 1) {  // r = x r mod χ
   const u64 c= r[d - 1];
   for(int j= d - 1; j >= 1; --j) r[j]= u32((r[j - 1] + c * nchi[j]) % MOD);
   r[0]= u32(c * nchi[0] % MOD);
  }
 }
 return r;
}
inline vector<u32> pow(int n, u64 K, const vector<u32>& a) {
 const int w= (n + 7) / 8 * 8;
 auto fallback= [&] { return tile::store(n, w, tile::pow(tile::load(n, w, a), K, w)); };
 if(K < 2) return fallback();
 const int W= 2 * n + 1;  // 行は [e (n 個) | α (n + 1 個)]。α の最後は、空間が埋まったあとの関係の分
 vector<u32> Y((size_t)n * W), T((size_t)n * n);  // T の鎖のベクトル t_j は T[j n .. j n + n)
 vector<int> piv(n), len(n);
 vector<u64> y(W);
 int r= 0;
 // x (= t_j) を入れる。新しい行になれば true。前の行の張る空間に入れば false で、y[n..n+j] に関係 (y[n+j] = 1) が残る。
 auto insert= [&](const u32* x, int j) -> bool {
  for(int i= 0; i < n; ++i) y[i]= x[i];
  fill(y.begin() + n, y.begin() + n + j + 1, 0);
  y[n + j]= 1;
  int cnt= 0;
  for(int k= 0; k < r; ++k) {
   const u32 c= u32(y[piv[k]] % M);
   if(!c) continue;
   axpy(y.data(), &Y[(size_t)k * W], M - c, len[k]);
   if(++cnt == 8) fold_all(y.data(), n + j + 1), cnt= 0;
  }
  int p= -1;
  for(int i= 0; i < n + j + 1; ++i) {
   y[i]%= M;
   if(p < 0 && i < n && y[i]) p= i;
  }
  if(p < 0) return false;
  const u64 iv= Z::raw(u32(y[p])).inv().val();
  u32* row= &Y[(size_t)r * W];
  for(int i= 0; i < n + j + 1; ++i) row[i]= u32(y[i] * iv % M);
  fill(row + n + j + 1, row + W, 0);
  piv[r]= p, len[r]= n + j + 1, ++r;
  return true;
 };
 vector<u32> cur(n), nxt(n);
 // v から鎖を作る (s 番目から)。前の空間に入ったときの鎖の長さ d を返し、y に関係を残す。
 auto gen= [&](const vector<u32>& v, int s) -> int {
  r= s;
  copy(v.begin(), v.end(), cur.begin());
  for(int j= s;; ++j) {
   if(!insert(cur.data(), j)) return j - s;
   copy(cur.begin(), cur.end(), &T[(size_t)j * n]);
   for(int i= 0; i < n; ++i) nxt[i]= dot(&a[(size_t)i * n], cur.data(), n);
   swap(cur, nxt);
  }
 };
 struct Block {
  int s, d;
  vector<u32> f;  // f[0..d]、f[d] = 1
 };
 vector<Block> blocks;
 mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());
 vector<u32> v(n);
 vector<u64> acc(n);
 while(r < n) {
  const int s= r;
  bool ok= false;
  for(int attempt= 0; attempt < 4 && !ok; ++attempt) {
   for(auto& x : v) x= u32(rng() % M);
   int d= gen(v, s);
   if(d == 0) continue;
   bool clean= true;
   for(int j= 0; j < s && clean; ++j) clean= y[n + j] == 0;
   if(!clean) {  // 前の鎖ごとに g_c = -(関係の成分) を f で割り、u = sum_c h_c(A) v_c を v から引く
    vector<u32> f(d + 1);
    for(int i= 0; i <= d; ++i) f[i]= u32(y[n + s + i]);
    fill(acc.begin(), acc.end(), 0);
    int cnt= 0;
    bool div_ok= true;
    for(const auto& B : blocks) {
     vector<u64> g(B.d);
     for(int i= 0; i < B.d; ++i) g[i]= y[n + B.s + i] ? M - y[n + B.s + i] : 0;
     for(int deg= B.d - 1; deg >= d; --deg) {  // g -= q x^{deg - d} f
      const u32 q= u32(g[deg] % M);
      g[deg]= 0;
      if(!q) continue;
      for(int i= 0; i < d; ++i) g[deg - d + i]= (g[deg - d + i] + u64(q) * (f[i] ? M - f[i] : 0)) % M;
      axpy(acc.data(), &T[(size_t)(B.s + deg - d) * n], q, n);
      if(++cnt == 8) fold_all(acc.data(), n), cnt= 0;
     }
     for(int i= 0; i < min(d, B.d) && div_ok; ++i) div_ok= g[i] % M == 0;
     if(!div_ok) break;
    }
    if(!div_ok) continue;
    for(int i= 0; i < n; ++i) v[i]= u32((v[i] + M - acc[i] % M) % M);
    d= gen(v, s);
    clean= d > 0;
    for(int j= 0; j < s && clean; ++j) clean= y[n + j] == 0;
    if(!clean) continue;
   }
   vector<u32> f(d + 1);
   for(int i= 0; i <= d; ++i) f[i]= u32(y[n + s + i]);
   blocks.push_back({s, d, f});
   ok= true;
  }
  if(!ok) return fallback();
 }
 // T^{-1}: α'_k = α_k - sum_{kk > k} e_k[piv_kk] α'_kk を k の大きい順に。T^{-1} の列 piv_k が α'_k。
 vector<u32> Ap((size_t)n * n);
 for(int k= n - 1; k >= 0; --k) {
  const u32* row= &Y[(size_t)k * W];
  for(int j= 0; j < n; ++j) acc[j]= row[n + j];
  int cnt= 0;
  for(int kk= k + 1; kk < n; ++kk) {
   const u32 c= row[piv[kk]];
   if(!c) continue;
   axpy(acc.data(), &Ap[(size_t)kk * n], M - c, n);
   if(++cnt == 8) fold_all(acc.data(), n), cnt= 0;
  }
  for(int j= 0; j < n; ++j) Ap[(size_t)k * n + j]= u32(acc[j] % M);
 }
 vector<u32> Tm((size_t)n * n), Fk((size_t)n * n), Ti((size_t)n * n);
 for(int j= 0; j < n; ++j)
  for(int i= 0; i < n; ++i) Tm[(size_t)i * n + j]= T[(size_t)j * n + i];
 for(int k= 0; k < n; ++k)
  for(int j= 0; j < n; ++j) Ti[(size_t)j * n + piv[k]]= Ap[(size_t)k * n + j];
 for(const auto& B : blocks) {
  vector<u32> col= xpow(K, B.f, B.d), nf(B.d);
  for(int i= 0; i < B.d; ++i) nf[i]= B.f[i] ? M - B.f[i] : 0;
  for(int jj= 0; jj < B.d; ++jj) {
   for(int ii= 0; ii < B.d; ++ii) Fk[(size_t)(B.s + ii) * n + B.s + jj]= col[ii];
   const u64 c= col[B.d - 1];  // col = x col mod f
   for(int ii= B.d - 1; ii >= 1; --ii) col[ii]= u32((col[ii - 1] + c * nf[ii]) % M);
   col[0]= u32(c * nf[0] % M);
  }
 }
 const vector<u32> Tw= tile::load(n, w, Tm), Fw= tile::load(n, w, Fk), Iw= tile::load(n, w, Ti);
 vector<u32> bp((size_t)w * w), P1((size_t)w * w), P2((size_t)w * w);
 tile::pack(Fw.data(), bp.data(), w), tile::mul_packed(Tw.data(), bp.data(), P1.data(), w);
 tile::pack(Iw.data(), bp.data(), w), tile::mul_packed(P1.data(), bp.data(), P2.data(), w);
 return tile::store(n, w, P2);
}
}  // namespace frob
