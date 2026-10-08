#pragma once
// lsd11_hp (11 bit + 11 bit + 10 bit の 3 回の LSD、作業用の配列は huge page) の振り分けを software write-combining にした版。桁の値
// ごとに 64 byte (16 個) の buffer を持ち、要素はまずその buffer に溜め、書き先の 1 行 (64 byte 境界) 分がたまったら non-temporal
// store (vmovntdq) でその行にまとめて書く。書き先の行を書く前に読み込む手間 (RFO) が要らず、書き先で L2 と L3 が埋まらない。行の
// 区切りは書き先の番地の 64 byte 境界に合わせるので、64 byte 境界に揃っていない a にも同じ形で書ける。桁の値の塊の最初の行は前の
// 塊と共有しうるので、そこだけは塊の分をふつうに書く。ヒストグラムを作る走査で a を作業用の配列へ写すところも non-temporal store
// にする。buffer は 2048 個で 128 KB になり、L1 には載らず L2 に載る。全要素が同じ値になる桁は飛ばす。256 個以下は std::sort に任せる。
#ifdef USE_SIMDE
#include <simde/x86/avx2.h>
#else
#include <immintrin.h>
#endif
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>
namespace sort_lsd11_swwc {
using u32= unsigned;
struct FreeDeleter {
 void operator()(void* p) const { std::free(p); }
};
// 2 MB 境界で確保し、huge page を頼んでから、触る前にまとめて用意させる。
inline std::unique_ptr<u32, FreeDeleter> alloc_huge(size_t n) {
 constexpr size_t H= size_t(1) << 21;
 const size_t bytes= (n * sizeof(u32) + H - 1) & ~(H - 1);
 void* p= std::aligned_alloc(H, bytes);
#ifdef __linux__
 madvise(p, bytes, MADV_HUGEPAGE);
 madvise(p, bytes, 23);  // MADV_POPULATE_WRITE
#endif
 return std::unique_ptr<u32, FreeDeleter>(static_cast<u32*>(p));
}
// src[0, n) を桁 (x >> sh & mk) の値で dst へ振り分ける (software write-combining)。c[d] は桁の値 d の塊の書き始めの位置で、振り分けた
// あとは書き終わりの位置になる。buf は桁の値ごとの 16 個 (64 byte) の buffer で、64 byte 境界に置く。要素 e の行の中の位置を
// (e + off) & 15 とし、0 の要素が dst の番地の 64 byte 境界に来るようにする。行の最後の位置まで溜まったら、行がまるごとその塊の
// ものなら non-temporal store で書き、塊の最初の行 (前の塊と共有しうる) なら塊の分だけをふつうに書く。最後に、buffer に残った
// 行の途中までの分をふつうに書いて sfence する。
inline void scatter_swwc(const u32* src, u32* dst, size_t n, int sh, u32 mk, u32* c, u32 (*buf)[16]) {
 const size_t nb= size_t(mk) + 1;
 const u32 off= u32(reinterpret_cast<uintptr_t>(dst) >> 2) & 15;
 u32 start[2048];
 std::copy(c, c + nb, start);
 for(size_t i= 0; i < n; ++i) {
  const u32 x= src[i], d= x >> sh & mk, p= c[d]++, slot= (p + off) & 15;
  buf[d][slot]= x;
  if(slot == 15) {
   if(p - start[d] >= 15) {  // 行がまるごとこの塊のもの
    const __m256i* s= reinterpret_cast<const __m256i*>(buf[d]);
    _mm256_stream_si256(reinterpret_cast<__m256i*>(dst + p - 15), s[0]);
    _mm256_stream_si256(reinterpret_cast<__m256i*>(dst + p - 7), s[1]);
   } else {  // 塊の最初の行は前の塊と共有しうるので、塊の分だけをふつうに書く
    for(u32 e= start[d]; e <= p; ++e) dst[e]= buf[d][(e + off) & 15];
   }
  }
 }
 for(size_t d= 0; d < nb; ++d) {  // buffer に残った、行の途中までの分
  const u32 end= c[d], fill= (end + off) & 15, from= end - start[d] >= fill ? end - fill : start[d];
  for(u32 e= from; e < end; ++e) dst[e]= buf[d][(e + off) & 15];
 }
 _mm_sfence();
}
inline void sort(std::vector<u32>& v) {
 const size_t n= v.size();
 if(n <= 256) return std::sort(v.begin(), v.end());
 static constexpr int SH[3]= {0, 11, 22};
 static constexpr u32 MK[3]= {2047, 2047, 1023};
 u32 c0[2048]= {}, c1[2048]= {}, c2[1024]= {};
 auto bbuf= alloc_huge(n);
 u32 *a= v.data(), *b= bbuf.get();
 // ヒストグラムを作りながら a を b へ写す。b は 2 MB 境界なので、8 個ずつ non-temporal store で書ける。
 size_t i= 0;
 for(; i + 8 <= n; i+= 8) {
  _mm256_stream_si256(reinterpret_cast<__m256i*>(b + i), _mm256_loadu_si256(reinterpret_cast<const __m256i*>(a + i)));
  for(int j= 0; j < 8; ++j) {
   const u32 x= a[i + j];
   ++c0[x & 2047], ++c1[x >> 11 & 2047], ++c2[x >> 22];
  }
 }
 for(; i < n; ++i) {
  const u32 x= a[i];
  b[i]= x;
  ++c0[x & 2047], ++c1[x >> 11 & 2047], ++c2[x >> 22];
 }
 _mm_sfence();
 u32* cs[3]= {c0, c1, c2};
 int use[3], k= 0;
 for(int d= 0; d < 3; ++d) {
  u32* c= cs[d];
  bool trivial= false;
  for(u32 t= 0; t <= MK[d]; ++t)
   if(c[t]) {
    trivial= c[t] == n;
    break;
   }
  if(trivial) continue;
  u32 s= 0;
  for(u32 t= 0; t <= MK[d]; ++t) {
   const u32 x= c[t];
   c[t]= s, s+= x;
  }
  use[k++]= d;
 }
 alignas(64) static u32 buf[2048][16];
 u32 *src= a, *dst= b;
 if(k & 1) src= b, dst= a;
 for(int j= 0; j < k; ++j) {
  const int d= use[j];
  scatter_swwc(src, dst, n, SH[d], MK[d], cs[d], buf);
  std::swap(src, dst);
 }
}
}  // namespace sort_lsd11_swwc
inline void run(std::vector<unsigned>& a) { sort_lsd11_swwc::sort(a); }
