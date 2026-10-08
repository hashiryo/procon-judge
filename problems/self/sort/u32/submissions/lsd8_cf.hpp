#pragma once
// lsd8 (8 bit × 4 回の LSD) の振り分けで、数え上げの表への書き込みを、書き先の配列への書き込みより先に置かせる版。
// GCC は表が局所の配列で書き先と重ならないと分かると、書き先への書き込みを先に置く (clang は表が先)。7763 (Zen 3) では lsd8 が
// GCC で clang の 2 倍かかったので、この順が原因かを見る。表のポインタを空の asm に通し、書き先と重なるかもしれないと思わせて、
// 源に書いた順 (表を読む、表に書く、書き先に書く) を保たせる。ほかは lsd8 と同じ。
#include <algorithm>
#include <memory>
#include <vector>
namespace sort_lsd8_cf {
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
  asm("" : "+r"(c));  // c が dst と重なるかもしれないと思わせ、下の 2 つの書き込みの順を保たせる
  const int sh= 8 * d;
  for(size_t i= 0; i < n; ++i) {
   const u32 x= src[i], k= x >> sh & 255, pos= c[k];
   c[k]= pos + 1;
   dst[pos]= x;
  }
  std::swap(src, dst);
 }
}
}  // namespace sort_lsd8_cf
inline void run(std::vector<unsigned>& a) { sort_lsd8_cf::sort(a); }
