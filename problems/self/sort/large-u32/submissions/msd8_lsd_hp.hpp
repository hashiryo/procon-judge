#pragma once
// 上の 8 bit で 1 回振り分けてから、塊ごとに L2 の中で残りの bit を LSD で並べる版。最初の走査で bit or と bit and を取りながら、上の
// 8 bit (x >> 24) のヒストグラムも作る。要素によって違う bit の上の 8 bit が x >> 24 と違えば (値の幅が狭いとき)、その 8 bit で
// ヒストグラムを作り直す。振り分けは a から huge page の作業用の配列へ、lsd11_swwc と同じ software write-combining で書く (桁の値は
// 256 個なので buffer は 16 KB で L1 に載る)。塊 (一様な 10^7 個なら約 39000 個、156 KB) ごとに、残りの bit を 8 bit 以下の桁に
// 等分した LSD で並べる。塊は L2 に載るので、途中の振り分けは L2 の中で済む。最後の振り分けは a の塊の位置へ software
// write-combining で書く。主記憶を読み書きするのは、最初の走査、振り分け、塊を読んで a へ書く分だけになる。塊の中でも全要素が
// 同じ値になる桁は飛ばし、32 個以下の塊は挿入ソートにする。256 個以下は std::sort に任せる。
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
namespace sort_msd8_lsd_hp {
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
inline void insertion(u32* a, size_t n) {
 for(size_t i= 1; i < n; ++i) {
  const u32 x= a[i];
  size_t j= i;
  for(; j > 0 && a[j - 1] > x; --j) a[j]= a[j - 1];
  a[j]= x;
 }
}
// src[0, m) を bit [lo, lo + w) について並べ、結果を dst[0, m) に書く。tmp は m 個の作業用の領域で、src も作業用に使ってよい。
// 途中の振り分けは src と tmp を行き来し (L2 の中)、最後の振り分けは dst へ software write-combining で書く。
inline void lsd_block(u32* src, u32* tmp, u32* dst, size_t m, int lo, int w, u32 (*buf)[16]) {
 if(w <= 0 || m <= 1) return (void)std::memcpy(dst, src, m * sizeof(u32));
 if(m <= 32) {
  std::memcpy(dst, src, m * sizeof(u32));
  return insertion(dst, m);
 }
 const int p= (w + 7) / 8;
 int sh[3]= {}, bits[3]= {};
 for(int d= 0, s= lo; d < p; ++d) {
  bits[d]= w / p + (d < w % p);
  sh[d]= s, s+= bits[d];
 }
 u32 cnt[3][256]= {};
 const u32 m0= (1u << bits[0]) - 1, m1= (1u << bits[1]) - 1, m2= (1u << bits[2]) - 1;
 for(size_t i= 0; i < m; ++i) {
  const u32 x= src[i];
  ++cnt[0][x >> sh[0] & m0], ++cnt[1][x >> sh[1] & m1], ++cnt[2][x >> sh[2] & m2];
 }
 int use[3], k= 0;
 for(int d= 0; d < p; ++d) {
  const u32 mk= (1u << bits[d]) - 1, x0= src[0];
  if(cnt[d][x0 >> sh[d] & mk] == m) continue;  // 全要素が同じ値になる桁
  u32 s= 0;
  for(u32 t= 0; t <= mk; ++t) {
   const u32 x= cnt[d][t];
   cnt[d][t]= s, s+= x;
  }
  use[k++]= d;
 }
 if(!k) return (void)std::memcpy(dst, src, m * sizeof(u32));
 u32 *from= src, *to= tmp;
 for(int j= 0; j < k - 1; ++j) {
  const int d= use[j], s= sh[d];
  const u32 mk= (1u << bits[d]) - 1;
  u32* c= cnt[d];
  for(size_t i= 0; i < m; ++i) {
   const u32 x= from[i];
   to[c[x >> s & mk]++]= x;
  }
  std::swap(from, to);
 }
 const int d= use[k - 1];
 scatter_swwc(from, dst, m, sh[d], (1u << bits[d]) - 1, cnt[d], buf);
}
inline void sort(std::vector<u32>& v) {
 const size_t n= v.size();
 if(n <= 256) return std::sort(v.begin(), v.end());
 u32* a= v.data();
 u32 o= 0, e= ~0u, cnt[256]= {};
 for(size_t i= 0; i < n; ++i) {
  const u32 x= a[i];
  o|= x, e&= x, ++cnt[x >> 24];
 }
 const u32 diff= o ^ e;
 if(!diff) return;  // 全要素が同じ値
 const int lo= __builtin_ctz(diff), hi= 31 - __builtin_clz(diff), s0= std::max(lo, hi - 7);
 const u32 m0= (1u << (hi - s0 + 1)) - 1;
 if(s0 != 24 || hi != 31) {  // 桁が x >> 24 そのものでなければ作り直す
  std::fill(cnt, cnt + 256, 0u);
  for(size_t i= 0; i < n; ++i) ++cnt[a[i] >> s0 & m0];
 }
 u32 head[256], pos[256], maxc= 0;
 for(u32 d= 0, s= 0; d <= m0; ++d) head[d]= pos[d]= s, s+= cnt[d], maxc= std::max(maxc, cnt[d]);
 auto bb= alloc_huge(n);
 u32* b= bb.get();
 alignas(64) static u32 buf[256][16];
 scatter_swwc(a, b, n, s0, m0, pos, buf);
 std::unique_ptr<u32[]> tmp(new u32[maxc]);
 for(u32 d= 0; d <= m0; ++d)
  if(cnt[d]) lsd_block(b + head[d], tmp.get(), a + head[d], cnt[d], lo, s0 - lo, buf);
}
}  // namespace sort_msd8_lsd_hp
inline void run(std::vector<unsigned>& a) { sort_msd8_lsd_hp::sort(a); }
