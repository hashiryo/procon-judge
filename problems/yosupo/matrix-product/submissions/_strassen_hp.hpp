#pragma once
// zmod_strassen (_strassen.hpp) に 3 つの書き換えを足して試す版。法は 998244353。
//   1. 作業の配列 (Morton 順の A、B、C と、象限の和を置く領域) を 1 つの領域にまとめ、Linux では 2 MB 境界の mmap に置いて
//      MADV_HUGEPAGE を頼み、MADV_POPULATE_WRITE で先にページを用意させる。答えの vector も reserve してから、2 MB 境界の
//      内側に huge page を頼む。1024 次では作業の配列が 16 MB あり、4 KB のページを初めて触る分 (1 枚 1.7 us ほど) が 7 ms ほど
//      になるため。
//   2. (ODD) 核で B の行の奇数番目の列を、32 bit 右にずらす (vpsrlq) 代わりに、4 byte ずらした番地から読み直す。vpmuludq は
//      64 bit の各々の下の 32 bit しか見ないので、上の 32 bit に隣の値が入っても結果は同じ。核の 1 段 (32 回の積和) の
//      ベクトルの演算が 17 個から 16 個に減る (読み込みは 1 個増える)。
//   3. (WINO) Strassen の 7 回の積を Winograd の形にして、1 段の足し算と引き算を 18 回から 15 回に減らす。並べ方は
//      Boyer、Dumas、Pernet、Zhou (2009) の表 1 (作業の領域 X、Y の 2 つで C = A B を求める順) に従う。
// BLK は核のブロックの 1 辺 (64 か 32)。記録は algo-notes の notes/linear-algebra.md。
#include "../common.hpp"
#ifndef USE_SIMDE
#include <immintrin.h>
#endif
#ifdef __linux__
#include <sys/mman.h>
#endif
namespace strassen_hp {
constexpr u32 M= MOD;
constexpr u32 NIV= [] {  // -M^{-1} mod 2^32
 u32 x= M;
 for(int i= 0; i < 5; ++i) x*= 2 - M * x;
 return -x;
}();
constexpr u32 R2= u32((u128(1) << 64) % M);
inline __m256i shrk(__m256i x, __m256i m) { return _mm256_min_epu32(x, _mm256_sub_epi32(x, m)); }
inline __m256i dilt(__m256i x, __m256i m) { return _mm256_min_epu32(x, _mm256_add_epi32(x, m)); }
// 偶数番目の和 e と奇数番目の和 o (どちらも 2^63 未満) を Montgomery の還元で 2^32 で割り、8 個の u32 (4M 未満) に並べる。
inline __m256i redc(__m256i e, __m256i o) {
 const __m256i niv= _mm256_set1_epi32(NIV), m= _mm256_set1_epi32(M);
 const __m256i ke= _mm256_mul_epu32(_mm256_mul_epu32(e, niv), m), ko= _mm256_mul_epu32(_mm256_mul_epu32(o, niv), m);
 return _mm256_blend_epi32(_mm256_srli_epi64(_mm256_add_epi64(e, ke), 32), _mm256_add_epi64(o, ko), 0xaa);
}
// c = a b 2^{-32} mod M (BLK × BLK のブロック、値は M 未満)。ODD なら奇数番目の列を 4 byte ずらして読む (b の行の 8 個の
// 後ろの 1 個 (4 byte) まで読むので、b の後ろに 4 byte 以上の領域がいる)。
template <int BLK, bool ODD> [[gnu::noinline]] void kernel(const u32* __restrict a, const u32* __restrict b, u32* __restrict c) {
 const __m256i Mv= _mm256_set1_epi32(M), M2= _mm256_set1_epi32(2 * M), TOP= _mm256_set1_epi64x(1ll << 63);
 auto keep= [&](__m256i x) { return _mm256_sub_epi64(x, _mm256_min_epu32(M2, _mm256_and_si256(TOP, x))); };
 for(int i= 0; i < BLK; i+= 4) {
  const u32 *a0= a + i * BLK, *a1= a0 + BLK, *a2= a1 + BLK, *a3= a2 + BLK;
  for(int j= 0; j < BLK; j+= 8) {
   __m256i e0= _mm256_setzero_si256(), o0= e0, e1= e0, o1= e0, e2= e0, o2= e0, e3= e0, o3= e0;
   for(int k= 0; k < BLK; k+= 8) {
    for(int w= k; w < k + 8; ++w) {
     const u32* bw= b + w * BLK + j;
     const __m256i z= _mm256_loadu_si256((const __m256i*)bw);
     const __m256i zo= ODD ? _mm256_loadu_si256((const __m256i*)(bw + 1)) : _mm256_srli_epi64(z, 32);
     __m256i x= _mm256_set1_epi32(a0[w]);
     e0= _mm256_add_epi64(e0, _mm256_mul_epu32(x, z)), o0= _mm256_add_epi64(o0, _mm256_mul_epu32(x, zo));
     x= _mm256_set1_epi32(a1[w]);
     e1= _mm256_add_epi64(e1, _mm256_mul_epu32(x, z)), o1= _mm256_add_epi64(o1, _mm256_mul_epu32(x, zo));
     x= _mm256_set1_epi32(a2[w]);
     e2= _mm256_add_epi64(e2, _mm256_mul_epu32(x, z)), o2= _mm256_add_epi64(o2, _mm256_mul_epu32(x, zo));
     x= _mm256_set1_epi32(a3[w]);
     e3= _mm256_add_epi64(e3, _mm256_mul_epu32(x, z)), o3= _mm256_add_epi64(o3, _mm256_mul_epu32(x, zo));
    }
    e0= keep(e0), o0= keep(o0), e1= keep(e1), o1= keep(o1), e2= keep(e2), o2= keep(o2), e3= keep(e3), o3= keep(o3);
   }
   _mm256_storeu_si256((__m256i*)(c + i * BLK + j), shrk(shrk(redc(e0, o0), M2), Mv));
   _mm256_storeu_si256((__m256i*)(c + (i + 1) * BLK + j), shrk(shrk(redc(e1, o1), M2), Mv));
   _mm256_storeu_si256((__m256i*)(c + (i + 2) * BLK + j), shrk(shrk(redc(e2, o2), M2), Mv));
   _mm256_storeu_si256((__m256i*)(c + (i + 3) * BLK + j), shrk(shrk(redc(e3, o3), M2), Mv));
  }
 }
}
inline void add(u32* x, const u32* y, const u32* z, size_t d) {
 const __m256i m= _mm256_set1_epi32(M);
 for(size_t i= 0; i < d; i+= 8)
  _mm256_storeu_si256((__m256i*)(x + i), shrk(_mm256_add_epi32(_mm256_loadu_si256((const __m256i*)(y + i)), _mm256_loadu_si256((const __m256i*)(z + i))), m));
}
inline void sub(u32* x, const u32* y, const u32* z, size_t d) {
 const __m256i m= _mm256_set1_epi32(M);
 for(size_t i= 0; i < d; i+= 8)
  _mm256_storeu_si256((__m256i*)(x + i), dilt(_mm256_sub_epi32(_mm256_loadu_si256((const __m256i*)(y + i)), _mm256_loadu_si256((const __m256i*)(z + i))), m));
}
// Strassen の形 (_strassen.hpp の mul と同じ並べ方)。c の後ろの n^2 / 3 ほどと、aa、bb を作業に使う。
template <int BLK, bool ODD> void mul_s(const u32* a, const u32* b, u32* c, size_t n, u32* aa, u32* bb) {
 if(n == BLK) return kernel<BLK, ODD>(a, b, c);
 const size_t h= n / 2, d= h * h;
 const u32 *a00= a, *a01= a + d, *a10= a + 2 * d, *a11= a + 3 * d;
 const u32 *b00= b, *b01= b + d, *b10= b + 2 * d, *b11= b + 3 * d;
 u32 *c00= c, *c01= c + d, *c10= c + 2 * d, *c11= c + 3 * d, *cc= c + n * n;
 sub(aa, a01, a11, d), add(bb, b10, b11, d), mul_s<BLK, ODD>(aa, bb, c00, h, aa + d, bb + d);
 add(aa, a00, a01, d), mul_s<BLK, ODD>(aa, b11, c01, h, aa + d, bb + d), sub(c00, c00, c01, d);
 sub(bb, b10, b00, d), mul_s<BLK, ODD>(a11, bb, c10, h, aa + d, bb + d), add(c00, c00, c10, d);
 sub(aa, a10, a00, d), add(bb, b00, b01, d), mul_s<BLK, ODD>(aa, bb, c11, h, aa + d, bb + d);
 add(aa, a10, a11, d), mul_s<BLK, ODD>(aa, b00, cc, h, aa + d, bb + d), add(c10, c10, cc, d), sub(c11, c11, cc, d);
 add(aa, a00, a11, d), add(bb, b00, b11, d), mul_s<BLK, ODD>(aa, bb, cc, h, aa + d, bb + d), add(c00, c00, cc, d), add(c11, c11, cc, d);
 sub(bb, b01, b11, d), mul_s<BLK, ODD>(a00, bb, cc, h, aa + d, bb + d), add(c01, c01, cc, d), add(c11, c11, cc, d);
}
// Winograd の形。S1 = A21 + A22、S2 = S1 - A11、S3 = A11 - A21、S4 = A12 - S2、T1 = B12 - B11、T2 = B22 - T1、
// T3 = B22 - B12、T4 = T2 - B21 として、P1 = A11 B11、P2 = A12 B21、P3 = S4 B22、P4 = A22 T4、P5 = S1 T1、P6 = S2 T2、
// P7 = S3 T3、U2 = P1 + P6、U3 = U2 + P7、U4 = U2 + P5 から C11 = P1 + P2、C12 = U4 + P3、C21 = U3 - P4、C22 = U3 + P5。
// X、Y は 1 辺 n / 2 の作業の領域で、深い段はその後ろを使う。c は作業に使わない。
template <int BLK, bool ODD> void mul_w(const u32* a, const u32* b, u32* c, size_t n, u32* X, u32* Y) {
 if(n == BLK) return kernel<BLK, ODD>(a, b, c);
 const size_t h= n / 2, d= h * h;
 const u32 *a11= a, *a12= a + d, *a21= a + 2 * d, *a22= a + 3 * d;
 const u32 *b11= b, *b12= b + d, *b21= b + 2 * d, *b22= b + 3 * d;
 u32 *c11= c, *c12= c + d, *c21= c + 2 * d, *c22= c + 3 * d, *X2= X + d, *Y2= Y + d;
 sub(X, a11, a21, d), sub(Y, b22, b12, d), mul_w<BLK, ODD>(X, Y, c21, h, X2, Y2);  // C21 = P7
 add(X, a21, a22, d), sub(Y, b12, b11, d), mul_w<BLK, ODD>(X, Y, c22, h, X2, Y2);  // C22 = P5
 sub(X, X, a11, d), sub(Y, b22, Y, d), mul_w<BLK, ODD>(X, Y, c12, h, X2, Y2);  // X = S2、Y = T2、C12 = P6
 sub(X, a12, X, d), mul_w<BLK, ODD>(X, b22, c11, h, X2, Y2);  // X = S4、C11 = P3
 mul_w<BLK, ODD>(a11, b11, X, h, X2, Y2);  // X = P1
 add(c12, X, c12, d), add(c21, c12, c21, d), add(c12, c12, c22, d), add(c22, c21, c22, d), add(c12, c12, c11, d);  // U2、U3、U4、U7、U5
 sub(Y, Y, b21, d), mul_w<BLK, ODD>(a22, Y, c11, h, X2, Y2), sub(c21, c21, c11, d);  // Y = T4、C11 = P4、C21 = U6
 mul_w<BLK, ODD>(a12, b21, c11, h, X2, Y2), add(c11, X, c11, d);  // C11 = P2、U1
}
template <int BLK> void place(const u32* src, int rows, int cols, size_t ld, size_t n, u32* dst) {
 if(n == BLK) {
  for(int i= 0; i < BLK; ++i) {
   u32* d= dst + i * BLK;
   if(i < rows) {
    const int w= min(cols, BLK);
    if(w > 0) memcpy(d, src + i * ld, w * sizeof(u32));
    if(w < BLK) memset(d + max(w, 0), 0, (BLK - max(w, 0)) * sizeof(u32));
   } else memset(d, 0, BLK * sizeof(u32));
  }
  return;
 }
 const size_t h= n / 2, d= h * h;
 place<BLK>(src, rows, cols, ld, h, dst);
 place<BLK>(src + h, rows, (int)(cols - h), ld, h, dst + d);
 place<BLK>(src + h * ld, (int)(rows - h), cols, ld, h, dst + 2 * d);
 place<BLK>(src + h * ld + h, (int)(rows - h), (int)(cols - h), ld, h, dst + 3 * d);
}
template <int BLK> void unplace(const u32* src, int rows, int cols, size_t ld, size_t n, u32* dst) {
 if(rows <= 0 || cols <= 0) return;
 if(n == BLK) {
  const __m256i r2= _mm256_set1_epi32(R2), m= _mm256_set1_epi32(M);
  alignas(32) u32 t[8];
  for(int i= 0; i < min(rows, BLK); ++i)
   for(int j= 0; j < min(cols, BLK); j+= 8) {
    const __m256i x= _mm256_loadu_si256((const __m256i*)(src + i * BLK + j));
    const __m256i y= shrk(redc(_mm256_mul_epu32(x, r2), _mm256_mul_epu32(_mm256_srli_epi64(x, 32), r2)), m);
    if(j + 8 <= cols) _mm256_storeu_si256((__m256i*)(dst + i * ld + j), y);
    else {
     _mm256_store_si256((__m256i*)t, y);
     for(int q= 0; j + q < cols; ++q) dst[i * ld + j + q]= t[q];
    }
   }
  return;
 }
 const size_t h= n / 2, d= h * h;
 unplace<BLK>(src, rows, cols, ld, h, dst);
 unplace<BLK>(src + d, rows, (int)(cols - h), ld, h, dst + h);
 unplace<BLK>(src + 2 * d, (int)(rows - h), cols, ld, h, dst + h * ld);
 unplace<BLK>(src + 3 * d, (int)(rows - h), (int)(cols - h), ld, h, dst + h * ld + h);
}
// n 個の u32 の作業の領域。0 で埋めない。Linux で 2^19 byte 以上なら、2 MB 境界の mmap に置いて huge page を頼み、先に
// ページを用意させる。
struct Region {
 u32* p= nullptr;
 size_t mapped= 0;
 explicit Region(size_t n) {
#ifdef __linux__
  constexpr size_t H= size_t(1) << 21;
  if(n * sizeof(u32) >= (size_t(1) << 19)) {
   const size_t bytes= (n * sizeof(u32) + H - 1) & ~(H - 1);
   void* m= mmap(nullptr, bytes + H, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   if(m != MAP_FAILED) {
    char *q= static_cast<char*>(m), *a= reinterpret_cast<char*>((reinterpret_cast<uintptr_t>(q) + H - 1) & ~(H - 1));
    if(a != q) munmap(q, a - q);
    munmap(a + bytes, q + H - a);
    madvise(a, bytes, MADV_HUGEPAGE);
    madvise(a, bytes, 23);  // MADV_POPULATE_WRITE (Linux 5.14 から。古い kernel では何もしない)
    p= reinterpret_cast<u32*>(a), mapped= bytes;
    return;
   }
  }
#endif
  p= static_cast<u32*>(::operator new(n * sizeof(u32)));
 }
 ~Region() {
#ifdef __linux__
  if(mapped) {
   munmap(p, mapped);
   return;
  }
#endif
  ::operator delete(p);
 }
 Region(const Region&)= delete;
 Region& operator=(const Region&)= delete;
};
// 長さ n の答えの vector。reserve した領域のうち 2 MB 境界の内側に huge page を頼み、先にページを用意させてから 0 で埋める。
inline vector<u32> out_vector(size_t n) {
 vector<u32> r;
 r.reserve(n);
#ifdef __linux__
 constexpr uintptr_t H= uintptr_t(1) << 21, P= 4096;
 const uintptr_t b= reinterpret_cast<uintptr_t>(r.data()), e= b + n * sizeof(u32);
 const uintptr_t hb= (b + H - 1) & ~(H - 1), he= e & ~(H - 1);
 if(hb < he) madvise(reinterpret_cast<void*>(hb), he - hb, MADV_HUGEPAGE);
 if(n * sizeof(u32) >= (size_t(1) << 19)) {
  const uintptr_t pb= b & ~(P - 1), pe= (e + P - 1) & ~(P - 1);
  madvise(reinterpret_cast<void*>(pb), pe - pb, 23);  // MADV_POPULATE_WRITE
 }
#endif
 r.resize(n);
 return r;
}
template <int BLK, bool WINO, bool ODD> vector<u32> matmul(int n, int m, int p, const vector<u32>& a, const vector<u32>& b) {
 size_t N= BLK;
 while(N < (size_t)max({n, m, p})) N*= 2;
 const size_t NN= N * N, W= NN / 3 + BLK * BLK;  // 作業の領域は n^2 / 4 + n^2 / 16 + ... < n^2 / 3
 // WINO: A、B (後ろに 4 byte 読む分の余り)、C、X、Y。そうでなければ A、B、C の後ろに W ずつ。
 const size_t la= NN + 64, sa= WINO ? la : NN + W;
 Region R(WINO ? 2 * la + NN + 2 * W : 3 * sa);
 u32 *A= R.p, *B= A + sa, *C= B + sa;
 place<BLK>(a.data(), n, m, m, N, A);
 place<BLK>(b.data(), m, p, p, N, B);
 if constexpr(WINO) {
  u32 *X= C + NN, *Y= X + W;
  mul_w<BLK, ODD>(A, B, C, N, X, Y);
 } else mul_s<BLK, ODD>(A, B, C, N, A + NN, B + NN);
 vector<u32> res= out_vector((size_t)n * p);
 unplace<BLK>(C, n, p, p, N, res.data());
 return res;
}
}  // namespace strassen_hp
