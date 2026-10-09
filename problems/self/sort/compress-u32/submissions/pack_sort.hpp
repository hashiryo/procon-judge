#pragma once
// 値を上位 32 bit、添字を下位 32 bit に詰めた u64 を std::sort で並べ、先頭から走査する。値が変わるたびに順位を 1 増やして xs に
// 足し、添字の位置に順位を書く。
#include <algorithm>
#include <vector>
inline std::vector<unsigned> run(std::vector<unsigned>& a) {
 using u32= unsigned;
 using u64= unsigned long long;
 const size_t n= a.size();
 std::vector<u32> xs;
 if(!n) return xs;
 std::vector<u64> b(n);
 for(size_t i= 0; i < n; ++i) b[i]= (u64(a[i]) << 32) | i;
 std::sort(b.begin(), b.end());
 xs.reserve(n);
 u32 prev= u32(b[0] >> 32), r= 0;
 xs.push_back(prev);
 for(size_t i= 0; i < n; ++i) {
  const u64 x= b[i];
  const u32 v= u32(x >> 32);
  if(v != prev) xs.push_back(v), prev= v, ++r;
  a[u32(x)]= r;
 }
 return xs;
}
