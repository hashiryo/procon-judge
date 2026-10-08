#pragma once
// LSD 基数ソート、8 bit × 4 回。4 つの桁のヒストグラムを 1 回の走査でまとめて作り、作業用の配列と
// 行き来しながら下の桁から振り分ける。4 回 (偶数回) なので最後は a に戻り、入れ替えも書き戻しも要らない。
// 作業用の配列は 0 で埋めない (new u32[n])。256 個以下は std::sort に任せる。
#include <algorithm>
#include <memory>
#include <vector>
namespace sort_lsd8 {
using u32= unsigned;
inline void sort(std::vector<u32>& v) {
 const size_t n= v.size();
 if(n <= 256) return std::sort(v.begin(), v.end());
 u32 cnt[4][256]= {};
 const u32* a= v.data();
 for(size_t i= 0; i < n; ++i) {
  u32 x= a[i];
  ++cnt[0][x & 255], ++cnt[1][x >> 8 & 255], ++cnt[2][x >> 16 & 255], ++cnt[3][x >> 24];
 }
 for(auto& c: cnt) {
  u32 s= 0;
  for(auto& e: c) {
   u32 t= e;
   e= s, s+= t;
  }
 }
 std::unique_ptr<u32[]> buf(new u32[n]);
 u32 *src= v.data(), *dst= buf.get();
 for(int d= 0; d < 4; ++d) {
  u32* c= cnt[d];
  const int sh= 8 * d;
  for(size_t i= 0; i < n; ++i) {
   u32 x= src[i];
   dst[c[x >> sh & 255]++]= x;
  }
  std::swap(src, dst);
 }
}
}  // namespace sort_lsd8
inline void run(std::vector<unsigned>& a) { sort_lsd8::sort(a); }
