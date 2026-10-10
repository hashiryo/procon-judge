#pragma once
// 節点を 512 bit (u64 が 8 個で 64 byte、cache line 1 本) にした 512 分木。段 h の i 番目の bit は、段 h - 1 の i 番目の
// 節点が 0 でないことを表す。N = 10^7 なら 3 段 (葉 1.25 MB、その上 2.4 KB、根 64 byte) で、64 分木より 1 段少ない。
// 節点の中で後ろの 1 を探すときは、8 個の語が 0 かどうかを AVX2 の 2 回の比較で調べ、最初の 0 でない語を選ぶ。
// 段は 1 本の配列に根から順に並べ、節点を 64 byte 境界に揃える。
#ifdef USE_SIMDE
#include <simde/x86/avx2.h>
#else
#include <immintrin.h>
#endif
#include <algorithm>
#include <new>
#include <vector>
namespace pp_fs512 {
using u64= unsigned long long;
struct Set {
 int lg, nn[8];  // 段 h (0 が葉) の節点の数
 size_t off[8];  // 段 h の先頭の語
 size_t words;
 u64* a;
 Set(int n, const std::vector<u64>& bits): lg(0) {
  for(int m= n;; m= (m + 511) / 512) {
   nn[lg++]= (m + 511) / 512;
   if(m <= 512) break;
  }
  words= 0;
  for(int h= lg; h--;) off[h]= words, words+= size_t(nn[h]) * 8;
  a= static_cast<u64*>(::operator new[](words * sizeof(u64), std::align_val_t(64)));
  std::fill(a, a + words, 0);
  std::copy(bits.begin(), bits.end(), a + off[0]);
  for(int h= 1; h < lg; ++h)
   for(int i= 0; i < nn[h - 1]; ++i)
    if(nz(h - 1, i)) a[off[h] + i / 64]|= u64(1) << (i % 64);
 }
 ~Set() { ::operator delete[](a, std::align_val_t(64)); }
 Set(const Set&)= delete;
 Set& operator=(const Set&)= delete;
 // 段 h の節点 p の語のうち、0 でないものの印 (8 bit)。
 unsigned nzmask(int h, int p) const {
  const u64* q= a + off[h] + size_t(p) * 8;
  const __m256i z= _mm256_setzero_si256();
  const unsigned m0= _mm256_movemask_pd(_mm256_castsi256_pd(_mm256_cmpeq_epi64(_mm256_load_si256(reinterpret_cast<const __m256i*>(q)), z)));
  const unsigned m1= _mm256_movemask_pd(_mm256_castsi256_pd(_mm256_cmpeq_epi64(_mm256_load_si256(reinterpret_cast<const __m256i*>(q + 4)), z)));
  return ~(m0 | m1 << 4) & 0xff;
 }
 bool nz(int h, int p) const { return nzmask(h, p) != 0; }
 // 段 h の bit i が指す節点 (段 h - 1 の i 番目) から、いちばん前の 1 まで下りる。
 int down_first(int h, int i) const {
  for(int g= h - 1; g >= 0; --g) {
   const int w= __builtin_ctz(nzmask(g, i));
   i= i * 512 + w * 64 + __builtin_ctzll(a[off[g] + size_t(i) * 8 + w]);
  }
  return i;
 }
 int down_last(int h, int i) const {
  for(int g= h - 1; g >= 0; --g) {
   const int w= 31 - __builtin_clz(nzmask(g, i));
   i= i * 512 + w * 64 + 63 - __builtin_clzll(a[off[g] + size_t(i) * 8 + w]);
  }
  return i;
 }
 bool contains(int i) const { return a[off[0] + i / 64] >> (i % 64) & 1; }
 void insert(int i) {
  a[off[0] + i / 64]|= u64(1) << (i % 64);
  for(int h= 1; h < lg; ++h) {
   i/= 512;
   u64& w= a[off[h] + i / 64];
   if(w >> (i % 64) & 1) return;
   w|= u64(1) << (i % 64);
  }
 }
 void erase(int i) {
  for(int h= 0; h < lg; ++h, i/= 512) {
   a[off[h] + i / 64]&= ~(u64(1) << (i % 64));
   if(nz(h, i / 512)) return;
  }
 }
 // k 以上で最小の要素。無ければ -1。
 int next(int i) const {
  for(int h= 0; h < lg; ++h) {
   const int p= i / 512;
   if(p == nn[h]) break;
   const int w= i / 64 % 8;
   if(const u64 d= a[off[h] + i / 64] >> (i % 64)) return down_first(h, i + __builtin_ctzll(d));
   if(const unsigned m= nzmask(h, p) & (0xfe << w) & 0xff) {
    const int v= __builtin_ctz(m);
    return down_first(h, p * 512 + v * 64 + __builtin_ctzll(a[off[h] + size_t(p) * 8 + v]));
   }
   i= p + 1;
  }
  return -1;
 }
 // k 以下で最大の要素。無ければ -1。
 int prev(int i) const {
  for(int h= 0; h < lg; ++h) {
   if(i == -1) break;
   const int p= i / 512, w= i / 64 % 8;
   if(const u64 d= a[off[h] + i / 64] << (63 - i % 64)) return down_last(h, i - __builtin_clzll(d));
   if(const unsigned m= nzmask(h, p) & ((1u << w) - 1)) {
    const int v= 31 - __builtin_clz(m);
    return down_last(h, p * 512 + v * 64 + 63 - __builtin_clzll(a[off[h] + size_t(p) * 8 + v]));
   }
   i= p - 1;
  }
  return -1;
 }
};
}
struct Solver {
 pp_fs512::Set s;
 Solver(int n, const vector<u64>& bits): s(n, bits) {}
 void insert(int k) { s.insert(k); }
 void erase(int k) { s.erase(k); }
 bool contains(int k) const { return s.contains(k); }
 int next(int k) const { return s.next(k); }
 int prev(int k) const { return s.prev(k); }
};
