#pragma once
// 値と添字の組 (16 byte) を値の順に std::sort で並べ、先頭から走査する。値が変わるたびに順位を 1 増やして xs に足し、添字の位置に
// 順位を書く。等しい値どうしの順は問わない。
#include <algorithm>
#include <vector>
inline std::vector<long long> run(std::vector<long long>& a) {
 using i64= long long;
 struct P {
  i64 v;
  unsigned i;
 };
 const size_t n= a.size();
 std::vector<i64> xs;
 if(!n) return xs;
 std::vector<P> b(n);
 for(size_t i= 0; i < n; ++i) b[i]= {a[i], unsigned(i)};
 std::sort(b.begin(), b.end(), [](const P& x, const P& y) { return x.v < y.v; });
 xs.reserve(n);
 i64 prev= b[0].v, r= 0;
 xs.push_back(prev);
 for(size_t i= 0; i < n; ++i) {
  if(b[i].v != prev) xs.push_back(b[i].v), prev= b[i].v, ++r;
  a[b[i].i]= r;
 }
 return xs;
}
