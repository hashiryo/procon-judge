#pragma once
// 値を上位 32 bit、添字を下位 32 bit に詰めた u64 を std::sort で並べ、下位 32 bit を取り出す。u64 の順がそのまま (値, 添字) の順で、
// 比べるときに a を添字で引かずに済む。
#include <algorithm>
#include <vector>
inline std::vector<unsigned> run(const std::vector<unsigned>& a) {
 const size_t n= a.size();
 std::vector<unsigned long long> b(n);
 for(size_t i= 0; i < n; ++i) b[i]= (static_cast<unsigned long long>(a[i]) << 32) | i;
 std::sort(b.begin(), b.end());
 std::vector<unsigned> p(n);
 for(size_t i= 0; i < n; ++i) p[i]= static_cast<unsigned>(b[i]);
 return p;
}
