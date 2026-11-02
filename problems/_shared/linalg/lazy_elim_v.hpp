#pragma once
// lazy_elim.hpp の前進消去 (要を 8 本ずつ貯めて残りの各行を 1 回だけ読み書きする形) を書き換えて試す版。法は奇数。
// yosupo の matrix-det と matrix-rank の提出が読む。記録は algo-notes の notes/linear-algebra.md。
//
// ElimH は lazy_elim::Elim と同じ手順で、行列を 2 MB 境界の mmap に置いて huge page を頼み (Linux)、要の行は組の 8 本分だけ
// 持つ (後退代入をしない行列式と階数では、組を足し終えた要の行を読まない)。n = 500 では行列が 2 MB あり、4 KB のページを
// 初めて触る分が 0.9 ms ほどになるため。
//
// ElimV は ElimH の、要を探す列、要の行、行ごとの掛ける数を AVX2 で 4 行 (要の行は 4 列) ずつ求める版。
//   - 組の初めに、残りの行の次の 8 列を列ごとの配列 (残りの行の並びの順) に写し (4 × 4 の転置)、要を探す列はそこから読む。
//     行ごとの掛ける数も、残りの行の並びの順に列ごとの配列に持つ。要の行を抜くときは、末尾の行の分をその位置へ移す。
//   - 値を M 未満に直すのは、2^32 の位を 2^32 mod M に掛けて畳んでから Montgomery の還元 (2^{-32} 倍) で行う。要の行 U' は
//     本当の値の 2^{-32} 倍、掛ける数 F' は 2^32 倍で持つので、積 F' U' は本当の値の積 F U に等しく、残りの行は本当の値の
//     まま足される。要を探す列の値 v' も 2^{-32} 倍で、F'_i = -v'_i (v'_p)^{-1} 2^32 になる。要の値は v'_p 2^32。
//   - 要は ElimH と同じく、残りの行の並びの順で値が 0 でない最初の行なので、選ぶ要は ElimH と同じになる。
#include <algorithm>
#include <cstdint>
#include <new>
#include <vector>
#ifdef USE_SIMDE
#include <simde/x86/avx2.h>
#else
#include <immintrin.h>
#endif
#ifdef __linux__
#include <sys/mman.h>
#endif
#include "_shared/linalg/lazy_elim.hpp"
namespace lazy_elim_v {
using u32= unsigned;
using u64= unsigned long long;
constexpr int K= 8;
// n 個の T の領域。0 では埋めない。Linux で 2^19 byte 以上なら、2 MB 境界の mmap に置いて huge page を頼み、先にページを
// 用意させる (MADV_POPULATE_WRITE、Linux 5.14 から)。
template <class T> struct HugeBuf {
 T* p= nullptr;
 std::size_t mapped= 0;
 explicit HugeBuf(std::size_t n) {
#ifdef __linux__
  constexpr std::size_t H= std::size_t(1) << 21;
  if(n * sizeof(T) >= (std::size_t(1) << 19)) {
   const std::size_t bytes= (n * sizeof(T) + H - 1) & ~(H - 1);
   void* m= mmap(nullptr, bytes + H, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   if(m != MAP_FAILED) {
    char *q= static_cast<char*>(m), *a= reinterpret_cast<char*>((reinterpret_cast<std::uintptr_t>(q) + H - 1) & ~(H - 1));
    if(a != q) munmap(q, a - q);
    munmap(a + bytes, q + H - a);
    madvise(a, bytes, MADV_HUGEPAGE);
    madvise(a, bytes, 23);
    p= reinterpret_cast<T*>(a), mapped= bytes;
    return;
   }
  }
#endif
  p= static_cast<T*>(::operator new(n * sizeof(T)));
 }
 ~HugeBuf() {
#ifdef __linux__
  if(mapped) {
   munmap(p, mapped);
   return;
  }
#endif
  ::operator delete(p);
 }
 HugeBuf(const HugeBuf&)= delete;
 HugeBuf& operator=(const HugeBuf&)= delete;
};
// n 行 w 列 (w は 4 の倍数) の行列の前進消去 (lazy_elim::Elim と同じ手順)。行列の後ろに 8 個の余りを置く。
template <class Z> struct ElimH {
 int n, w;
 HugeBuf<u64> buf;
 std::vector<int> rem, prow;
 std::vector<u32> pval;
 ElimH(int n_, int w_): n(n_), w(w_), buf((std::size_t)n_ * w_ + 8), rem(n_) {
  for(int i= 0; i < n; ++i) rem[i]= i;
 }
 u64* row(int r) { return buf.p + (std::size_t)r * w; }
 // 列 [0, lim) で要を探す。要のない列があれば、stop なら false を返して止め、そうでなければ飛ばす。
 bool run(int lim, bool stop) {
  const u32 M= Z::mod();
  const u64 R= (u64(1) << 32) % M;
  u64* a= buf.p;
  std::vector<u64> U((std::size_t)std::min(n, K) * w), zero(w);
  std::vector<u32> F((std::size_t)n * K), v(n);
  const u64* Ub[K];
  int col= 0;
  while(col < lim && !rem.empty()) {
   int bt= 0;
   for(; bt < K && col < lim && !rem.empty(); ++col) {
    const int nr= rem.size();
    int pi= -1;
    for(int i= 0; i < nr; ++i) {
     const int r= rem[i];
     const u32* fr= &F[(std::size_t)r * K];
     u64 x= a[(std::size_t)r * w + col];
     for(int s= 0; s < bt; ++s) x+= u64(fr[s]) * Ub[s][col];
     v[i]= Z(x).val();
     if(pi < 0 && v[i]) pi= i;
    }
    if(pi < 0) {
     if(stop) return false;
     continue;
    }
    const int p= rem[pi];
    u64* up= &U[(std::size_t)bt * w];
    const u32* fp= &F[(std::size_t)p * K];
    const u64* ap= row(p);
    for(int j= col & ~3; j < w; ++j) {
     u64 x= ap[j];
     for(int s= 0; s < bt; ++s) x+= u64(fp[s]) * Ub[s][j];
     up[j]= Z(x).val();
    }
    const Z iv= Z::raw(v[pi]).inv();
    for(int i= 0; i < nr; ++i) F[(std::size_t)rem[i] * K + bt]= (-(Z::raw(v[i]) * iv)).val();
    prow.push_back(p), pval.push_back(v[pi]);
    Ub[bt++]= up;
    rem[pi]= rem.back(), rem.pop_back();
   }
   if(!bt || col >= lim || rem.empty()) break;  // 残りの行をもう読まないなら、最後の組は足さない
   for(int s= bt; s < K; ++s) Ub[s]= zero.data();
   const int jb= col & ~3;
   for(int r: rem) {
    u32* fr= &F[(std::size_t)r * K];
    for(int s= bt; s < K; ++s) fr[s]= 0;
    u32 any= 0;
    for(int s= 0; s < K; ++s) any|= fr[s];
    if(any) lazy_elim::update_row(row(r), Ub, fr, jb, w, R);
   }
  }
  return true;
 }
};
// 4 個の u64 (2^63 未満) を、2^32 の位を R = 2^32 mod M に掛けて畳んでから Montgomery の還元で 2^{-32} 倍し、M 未満に直す。
// 畳んだ値は 2^31 M + 2^32 未満 (2^32 M 未満) なので、還元の結果は 2M 未満。
struct Mont {
 __m256i R, LO, NIV, M, M32;
 explicit Mont(u32 m) {
  u32 x= m;
  for(int i= 0; i < 5; ++i) x*= 2 - m * x;
  R= _mm256_set1_epi64x((u64(1) << 32) % m), LO= _mm256_set1_epi64x(0xffffffffll), NIV= _mm256_set1_epi64x(u32(-x));
  M= _mm256_set1_epi64x(m), M32= _mm256_set1_epi32(m);
 }
 [[gnu::always_inline]] __m256i redc(__m256i x) const {
  x= _mm256_add_epi64(_mm256_mul_epu32(_mm256_srli_epi64(x, 32), R), _mm256_and_si256(x, LO));
  const __m256i t= _mm256_srli_epi64(_mm256_add_epi64(x, _mm256_mul_epu32(_mm256_mul_epu32(x, NIV), M)), 32);
  return _mm256_min_epu32(t, _mm256_sub_epi32(t, M32));  // 上の 32 bit は 0 のまま (0 - M は大きいので min で 0)
 }
};
// u64 の 4 個の下の 32 bit を u32 の 4 個に詰める。
[[gnu::always_inline]] inline __m128i pack_lo(__m256i x) { return _mm256_castsi256_si128(_mm256_permutevar8x32_epi32(x, _mm256_setr_epi32(0, 2, 4, 6, 0, 0, 0, 0))); }
template <class Z> struct ElimV {
 int n, w;
 HugeBuf<u64> buf;
 std::vector<int> rem, prow;
 std::vector<u32> pvalm;  // 要の値の 2^{-32} 倍
 ElimV(int n_, int w_): n(n_), w(w_), buf((std::size_t)n_ * w_ + 8), rem(n_) {
  for(int i= 0; i < n; ++i) rem[i]= i;
 }
 u64* row(int r) { return buf.p + (std::size_t)r * w; }
 bool run(int lim, bool stop) {
  const u32 M= Z::mod();
  const u64 R= (u64(1) << 32) % M;
  const Mont mt(M);
  u64* a= buf.p;
  const int n4= (n + 3) & ~3;
  // C[c][i]: 残りの行の並びで i 番目の行の、列 c0 + c の値。F[s][i]: その行の s 番目の要の掛ける数 (2^32 倍)。V[i]: 今の列の値。
  std::vector<u64> C((std::size_t)K * n4), U((std::size_t)std::min(n, K) * w + 8), zero(w + 8);
  std::vector<u32> F((std::size_t)K * n4), V(n4);
  const u64* Ub[K];
  int col= 0;
  auto fill_cache= [&](int c0) {  // 残りの行の列 [c0, c0 + 8) を C に写す。行の後ろの余りを読んでも、使わない列に入るだけ
   const int nr= rem.size();
   for(int i= 0; i < nr; i+= 4) {
    const u64* r0= row(rem[i]) + c0;
    const u64* r1= i + 1 < nr ? row(rem[i + 1]) + c0 : zero.data();
    const u64* r2= i + 2 < nr ? row(rem[i + 2]) + c0 : zero.data();
    const u64* r3= i + 3 < nr ? row(rem[i + 3]) + c0 : zero.data();
    for(int h= 0; h < 8; h+= 4) {
     const __m256i x0= _mm256_loadu_si256((const __m256i*)(r0 + h)), x1= _mm256_loadu_si256((const __m256i*)(r1 + h));
     const __m256i x2= _mm256_loadu_si256((const __m256i*)(r2 + h)), x3= _mm256_loadu_si256((const __m256i*)(r3 + h));
     const __m256i t0= _mm256_unpacklo_epi64(x0, x1), t1= _mm256_unpackhi_epi64(x0, x1);
     const __m256i t2= _mm256_unpacklo_epi64(x2, x3), t3= _mm256_unpackhi_epi64(x2, x3);
     _mm256_storeu_si256((__m256i*)&C[(std::size_t)(h + 0) * n4 + i], _mm256_permute2x128_si256(t0, t2, 0x20));
     _mm256_storeu_si256((__m256i*)&C[(std::size_t)(h + 1) * n4 + i], _mm256_permute2x128_si256(t1, t3, 0x20));
     _mm256_storeu_si256((__m256i*)&C[(std::size_t)(h + 2) * n4 + i], _mm256_permute2x128_si256(t0, t2, 0x31));
     _mm256_storeu_si256((__m256i*)&C[(std::size_t)(h + 3) * n4 + i], _mm256_permute2x128_si256(t1, t3, 0x31));
    }
   }
  };
  while(col < lim && !rem.empty()) {
   int bt= 0, c0= col;
   fill_cache(c0);
   for(; bt < K && col < lim && !rem.empty(); ++col) {
    if(col == c0 + 8) fill_cache(c0= col);
    const int nr= rem.size(), nr4= (nr + 3) & ~3;
    // 今の列の値 V'[i] = (C + sum_s F'[s] U'[s][col]) 2^{-32} mod M と、0 でない最初の i。
    const u64* cc= &C[(std::size_t)(col - c0) * n4];
    __m256i us[K];
    for(int s= 0; s < bt; ++s) us[s]= _mm256_set1_epi64x(Ub[s][col]);
    int pi= -1;
    for(int i= 0; i < nr4; i+= 4) {
     __m256i x= _mm256_loadu_si256((const __m256i*)(cc + i));
     for(int s= 0; s < bt; ++s) x= _mm256_add_epi64(x, _mm256_mul_epu32(_mm256_cvtepu32_epi64(_mm_loadu_si128((const __m128i*)&F[(std::size_t)s * n4 + i])), us[s]));
     const __m256i y= mt.redc(x);
     _mm_storeu_si128((__m128i*)&V[i], pack_lo(y));
     if(pi < 0) {
      const int nz= ~_mm256_movemask_pd(_mm256_castsi256_pd(_mm256_cmpeq_epi64(y, _mm256_setzero_si256()))) & 15;
      if(nz) pi= i + __builtin_ctz(nz);
     }
    }
    if(pi >= nr) pi= -1;  // 余りの行 (値は 0) は選ばない
    if(pi < 0) {
     if(stop) return false;
     continue;
    }
    const int p= rem[pi];
    u64* up= &U[(std::size_t)bt * w];
    {  // 要の行 U'[j] = (a[p][j] + sum_s F'[s][pi] U'[s][j]) 2^{-32} mod M
     __m256i fs[K];
     for(int s= 0; s < bt; ++s) fs[s]= _mm256_set1_epi64x(F[(std::size_t)s * n4 + pi]);
     const u64* ap= row(p);
     for(int j= col & ~3; j < w; j+= 4) {
      __m256i x= _mm256_loadu_si256((const __m256i*)(ap + j));
      for(int s= 0; s < bt; ++s) x= _mm256_add_epi64(x, _mm256_mul_epu32(_mm256_loadu_si256((const __m256i*)(Ub[s] + j)), fs[s]));
      _mm256_storeu_si256((__m256i*)(up + j), mt.redc(x));
     }
    }
    {  // F'[bt][i] = -V'[i] c mod M (c = (V'[pi])^{-1} 2^32)。Shoup の方法で 4 個ずつ。
     const u32 c= (Z::raw(V[pi]).inv() * Z::raw(u32(R))).val(), cs= u32((u64(c) << 32) / M);
     const __m256i Cv= _mm256_set1_epi64x(c), CS= _mm256_set1_epi64x(cs);
     u32* fb= &F[(std::size_t)bt * n4];
     for(int i= 0; i < nr4; i+= 4) {
      const __m256i vv= _mm256_cvtepu32_epi64(_mm_loadu_si128((const __m128i*)&V[i]));
      const __m256i q= _mm256_srli_epi64(_mm256_mul_epu32(vv, CS), 32);
      __m256i t= _mm256_sub_epi64(_mm256_mul_epu32(vv, Cv), _mm256_mul_epu32(q, mt.M));  // [0, 2M)
      t= _mm256_min_epu32(t, _mm256_sub_epi32(t, mt.M32));  // [0, M)
      t= _mm256_sub_epi32(mt.M32, t);  // (0, M]
      t= _mm256_min_epu32(t, _mm256_sub_epi32(t, mt.M32));  // [0, M)
      _mm_storeu_si128((__m128i*)(fb + i), pack_lo(t));
     }
    }
    prow.push_back(p), pvalm.push_back(V[pi]);
    Ub[bt++]= up;
    // 要の行を抜き、末尾の行の分を位置 pi へ移して、末尾を 0 にする
    const int last= nr - 1;
    rem[pi]= rem[last], rem.pop_back();
    for(int s= 0; s < bt; ++s) F[(std::size_t)s * n4 + pi]= F[(std::size_t)s * n4 + last], F[(std::size_t)s * n4 + last]= 0;
    for(int cix= 0; cix < K; ++cix) C[(std::size_t)cix * n4 + pi]= C[(std::size_t)cix * n4 + last], C[(std::size_t)cix * n4 + last]= 0;
   }
   if(!bt || col >= lim || rem.empty()) break;
   for(int s= bt; s < K; ++s) Ub[s]= zero.data();
   const int jb= col & ~3, nr= rem.size();
   for(int i= 0; i < nr; ++i) {
    u32 f[K], any= 0;
    for(int s= 0; s < K; ++s) any|= (f[s]= s < bt ? F[(std::size_t)s * n4 + i] : 0);
    if(any) lazy_elim::update_row(row(rem[i]), Ub, f, jb, w, R);
   }
   for(int s= 0; s < bt; ++s) std::fill(&F[(std::size_t)s * n4], &F[(std::size_t)s * n4] + n4, 0u);
  }
  return true;
 }
};
}  // namespace lazy_elim_v
