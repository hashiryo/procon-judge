#pragma once
// zmod_wiedemann の行列とベクトルの積と Berlekamp–Massey を AVX2 で 4 個ずつにした版。法は 998244353。
// 行と列を同じ並べ替え (成分の数の多い順) で付け直す (相似変換なので行列式は変わらない) と、jagged diagonal の段 j では
// 行 q < cnt[j] の成分を続けて読み、w は列の番号で 4 個ずつ集めて (gather) 掛け、和をそのまま行 q に足せる。各行の和は畳んでから
// Montgomery の還元で 2^{-32} 倍して M 未満に直す。毎回 2^{-32} 倍するのは、λ = 2^{-32} として λB の列 u^T (λB)^i v を作るのと
// 同じなので、det(λB) を求めてから λ^{-N} を掛けて戻す。Berlekamp–Massey のずれの内積と、漸化式の更新 (Shoup の方法) も 4 個ずつ。
#include <chrono>
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
#ifndef USE_SIMDE
#include <immintrin.h>
#endif
namespace wiedemann_avx2 {
using Z= ZMod<MOD>;
constexpr u64 R32= (u64(1) << 32) % MOD;
constexpr u32 NIV= [] {  // -M^{-1} mod 2^32
 u32 x= MOD;
 for(int i= 0; i < 5; ++i) x*= 2 - MOD * x;
 return -x;
}();
inline __m256i fold(__m256i x) {
 return _mm256_add_epi64(_mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_set1_epi64x(R32)), _mm256_and_si256(x, _mm256_set1_epi64x(0xffffffffll)));
}
inline u32 hsum(__m256i x) {
 alignas(32) u64 t[4];
 _mm256_store_si256((__m256i*)t, fold(x));
 return Z(t[0] % MOD + t[1] % MOD + t[2] % MOD + t[3] % MOD).val();
}
// sum_{i<n} x[i] y[i] (x, y は M 未満の u32)
inline u32 dot(const u32* x, const u32* y, int n) {
 __m256i acc= _mm256_setzero_si256();
 int i= 0;
 for(int t= 0; i + 4 <= n; i+= 4) {
  acc= _mm256_add_epi64(acc, _mm256_mul_epu32(_mm256_cvtepu32_epi64(_mm_loadu_si128((const __m128i*)(x + i))), _mm256_cvtepu32_epi64(_mm_loadu_si128((const __m128i*)(y + i)))));
  if(++t == 8) acc= fold(acc), t= 0;
 }
 u64 r= hsum(acc);
 for(; i < n; ++i) r+= u64(x[i]) * y[i] % MOD;
 return Z(r).val();
}
}  // namespace wiedemann_avx2
inline u32 run(int N, int K, const vector<int>& a, const vector<int>& b, const vector<u32>& c) {
 using namespace wiedemann_avx2;
 {  // 空の行か列があれば正則でない
  vector<char> hr(N), hc(N);
  for(int i= 0; i < K; ++i)
   if(c[i]) hr[a[i]]= hc[b[i]]= 1;
  for(int i= 0; i < N; ++i)
   if(!hr[i] || !hc[i]) return 0;
 }
 mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());
 auto rnd= [&]() { return u32(rng() % (MOD - 1) + 1); };
 vector<u32> d(N);
 for(auto& x : d) x= rnd();
 vector<int> len(N), perm(N), rank(N);
 for(int i= 0; i < K; ++i) ++len[a[i]];
 iota(perm.begin(), perm.end(), 0);
 stable_sort(perm.begin(), perm.end(), [&](int x, int y) { return len[x] > len[y]; });
 for(int q= 0; q < N; ++q) rank[perm[q]]= q;
 const int maxlen= len[perm[0]], N4= (N + 3) & ~3;
 // 段 j の行の数を 4 の倍数に切り上げ (足した分は値 0、列 0)、jst[j] から並べる
 vector<int> cnt(maxlen), jst(maxlen + 1), fill_(N);
 for(int q= 0; q < N; ++q)
  for(int j= 0; j < len[perm[q]]; ++j) ++cnt[j];
 for(int j= 0; j < maxlen; ++j) cnt[j]= (cnt[j] + 3) & ~3, jst[j + 1]= jst[j] + cnt[j];
 vector<int> col(jst[maxlen]);
 vector<u32> val(jst[maxlen]);
 for(int i= 0; i < K; ++i) {
  const int q= rank[a[i]], e= jst[fill_[q]++] + q;
  col[e]= rank[b[i]], val[e]= (Z::raw(c[i]) * Z::raw(d[b[i]])).val();
 }
 vector<u32> u(N4), w(N4), w2(N4), s(2 * N);
 for(int i= 0; i < N; ++i) u[i]= rnd(), w[i]= rnd();
 vector<u64> acc(N4);
 const __m256i Mv= _mm256_set1_epi64x(MOD), Niv= _mm256_set1_epi64x(NIV), M32= _mm256_set1_epi32(MOD);
 for(int i= 0; i < 2 * N; ++i) {
  s[i]= dot(u.data(), w.data(), N);
  if(i + 1 == 2 * N) break;
  fill(acc.begin(), acc.end(), 0);
  for(int j= 0; j < maxlen; ++j) {
   const int* cp= &col[jst[j]];
   const u32* vp= &val[jst[j]];
   for(int q= 0; q < cnt[j]; q+= 4) {
    const __m128i wv= _mm_i32gather_epi32((const int*)w.data(), _mm_loadu_si128((const __m128i*)(cp + q)), 4);
    const __m256i p= _mm256_mul_epu32(_mm256_cvtepu32_epi64(wv), _mm256_cvtepu32_epi64(_mm_loadu_si128((const __m128i*)(vp + q))));
    _mm256_storeu_si256((__m256i*)&acc[q], _mm256_add_epi64(_mm256_loadu_si256((const __m256i*)&acc[q]), p));
   }
   if((j & 7) == 7)
    for(int q= 0; q < cnt[j]; q+= 4) _mm256_storeu_si256((__m256i*)&acc[q], fold(_mm256_loadu_si256((const __m256i*)&acc[q])));
  }
  for(int q= 0; q < N4; q+= 4) {  // 畳んで 2^60.2 未満にしてから Montgomery の還元 (M 2^32 未満なら 2M 未満が返る)
   const __m256i x= fold(_mm256_loadu_si256((const __m256i*)&acc[q]));
   const __m256i t= _mm256_srli_epi64(_mm256_add_epi64(x, _mm256_mul_epu32(_mm256_mul_epu32(x, Niv), Mv)), 32);
   const __m256i r= _mm256_min_epu32(t, _mm256_sub_epi32(t, M32));
   _mm_storeu_si128((__m128i*)&w2[q], _mm256_castsi256_si128(_mm256_permutevar8x32_epi32(r, _mm256_setr_epi32(0, 2, 4, 6, 0, 0, 0, 0))));
  }
  swap(w, w2);
 }
 // Berlekamp–Massey。ずれは C と、s を逆順に並べた sr の内積。更新 C[i + m] += (M - coef) B[i] は Shoup の方法で 4 個ずつ。
 vector<u32> sr(2 * N), C(N + 4), B(N + 4), T;
 for(int i= 0; i < 2 * N; ++i) sr[i]= s[2 * N - 1 - i];
 C[0]= B[0]= 1;
 int L= 0, LB= 0, m= 1;
 Z bb= Z::raw(1);
 for(int n= 0; n < 2 * N; ++n) {
  const Z dd= Z::raw(dot(C.data(), &sr[2 * N - 1 - n], L + 1));
  if(dd == Z()) {
   ++m;
   continue;
  }
  const u32 cf= (-(dd * bb.inv())).val(), cs= u32((u64(cf) << 32) / MOD);
  const bool grow= 2 * L <= n;
  if(grow) T.assign(C.begin(), C.begin() + L + 1);
  const int top= min(LB, N - m);  // C[i + m] (i <= top) を直す
  const __m256i CF= _mm256_set1_epi64x(cf), CS= _mm256_set1_epi64x(cs);
  int i= 0;
  for(; i + 4 <= top + 1; i+= 4) {
   const __m256i bv= _mm256_cvtepu32_epi64(_mm_loadu_si128((const __m128i*)&B[i]));
   const __m256i q= _mm256_srli_epi64(_mm256_mul_epu32(bv, CS), 32);
   const __m256i t= _mm256_sub_epi64(_mm256_mul_epu32(bv, CF), _mm256_mul_epu32(q, Mv));  // [0, 2M)
   __m256i x= _mm256_add_epi64(_mm256_cvtepu32_epi64(_mm_loadu_si128((const __m128i*)&C[i + m])), t);  // [0, 3M)
   x= _mm256_min_epu32(x, _mm256_sub_epi32(x, _mm256_set1_epi32(2 * MOD)));
   x= _mm256_min_epu32(x, _mm256_sub_epi32(x, M32));
   _mm_storeu_si128((__m128i*)&C[i + m], _mm256_castsi256_si128(_mm256_permutevar8x32_epi32(x, _mm256_setr_epi32(0, 2, 4, 6, 0, 0, 0, 0))));
  }
  for(; i <= top; ++i) C[i + m]= (Z::raw(C[i + m]) + Z::raw(cf) * Z::raw(B[i])).val();
  if(grow) {
   fill(B.begin(), B.begin() + LB + 1, 0u);
   copy(T.begin(), T.end(), B.begin());
   LB= L, L= n + 1 - L, bb= dd, m= 1;
  } else ++m;
 }
 if(L != N) return 0;
 // det(λB) = (-1)^N C[N]、det(A) = det(λB) λ^{-N} / det(D)、λ^{-1} = 2^32
 Z det= Z::raw(C[N]);
 if(N & 1) det= -det;
 Z dD= Z::raw(1);
 for(u32 x : d) dD*= Z::raw(x);
 return (det * Z(R32).pow(N) * dD.inv()).val();
}
