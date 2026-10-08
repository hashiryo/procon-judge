#pragma once
// LSD 基数ソート、16 bit × 2 回。振り分けの回数は 11 bit × 3 回より 1 回少ないが、ヒストグラムが 65536 個 × 2 (512 KB) になり、
// 書き先も 65536 か所に散らばる。2 回 (偶数回) なので最後は a に戻る。作業用の配列は 0 で埋めない。4096 個以下は std::sort に任せる。
#include <algorithm>
#include <memory>
#include <vector>
namespace sort_lsd16 {
using u32= unsigned;
inline void sort(std::vector<u32>& v) {
 const size_t n= v.size();
 if(n <= 4096) return std::sort(v.begin(), v.end());
 std::vector<u32> cnt(2 << 16);
 u32 *c0= cnt.data(), *c1= c0 + (1 << 16);
 u32* a= v.data();
 for(size_t i= 0; i < n; ++i) {
  u32 x= a[i];
  ++c0[x & 0xFFFF], ++c1[x >> 16];
 }
 for(u32* c: {c0, c1}) {
  u32 s= 0;
  for(u32 i= 0; i < (1u << 16); ++i) {
   u32 t= c[i];
   c[i]= s, s+= t;
  }
 }
 std::unique_ptr<u32[]> buf(new u32[n]);
 u32* b= buf.get();
 for(size_t i= 0; i < n; ++i) {
  u32 x= a[i];
  b[c0[x & 0xFFFF]++]= x;
 }
 for(size_t i= 0; i < n; ++i) {
  u32 x= b[i];
  a[c1[x >> 16]++]= x;
 }
}
}  // namespace sort_lsd16
inline void run(std::vector<unsigned>& a) { sort_lsd16::sort(a); }
