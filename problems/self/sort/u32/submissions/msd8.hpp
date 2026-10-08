#pragma once
// MSD 基数ソート、8 bit ずつ上の桁から。振り分けはその場で置換を巡回させる American flag sort で、作業用の配列を
// 確保しない (ページフォールトが起きない)。全要素が同じ桁になる塊はその桁を飛ばし、32 個以下の塊は挿入ソートにする。
#include <cstddef>
#include <utility>
#include <vector>
namespace sort_msd8 {
using u32= unsigned;
inline void insertion(u32* a, size_t n) {
 for(size_t i= 1; i < n; ++i) {
  u32 x= a[i];
  size_t j= i;
  for(; j > 0 && a[j - 1] > x; --j) a[j]= a[j - 1];
  a[j]= x;
 }
}
inline void rec(u32* a, size_t n, int sh) {
 for(;;) {
  if(n <= 32) return insertion(a, n);
  size_t cnt[256]= {};
  for(size_t i= 0; i < n; ++i) ++cnt[a[i] >> sh & 255];
  if(cnt[a[0] >> sh & 255] == n) {  // この桁は全要素で同じ
   if(sh == 0) return;
   sh-= 8;
   continue;
  }
  size_t head[256], tail[256];
  for(size_t s= 0, d= 0; d < 256; ++d) head[d]= s, s+= cnt[d], tail[d]= s;
  for(int d= 0; d < 256; ++d) {
   while(head[d] < tail[d]) {
    u32 x= a[head[d]];
    u32 e= x >> sh & 255;
    while(e != u32(d)) {
     std::swap(x, a[head[e]++]);
     e= x >> sh & 255;
    }
    a[head[d]++]= x;
   }
  }
  if(sh == 0) return;
  for(size_t s= 0, d= 0; d < 256; s+= cnt[d], ++d)
   if(cnt[d] > 1) rec(a + s, cnt[d], sh - 8);
  return;
 }
}
inline void sort(std::vector<u32>& v) { rec(v.data(), v.size(), 24); }
}  // namespace sort_msd8
inline void run(std::vector<unsigned>& a) { sort_msd8::sort(a); }
