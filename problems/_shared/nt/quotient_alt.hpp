#pragma once
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <tuple>
#include <vector>
// enumerate_quotients の書き比べ用の版。区間を 1 つ作るのに割り算を 1 回だけ使う
// (neo/base/core/nt/quotient.hpp の loop は N / l と N / q で 2 回使う)。
// 小さい q の側は q を 1 から上げて N / q で区切り、大きい q の側は l を k から下げて N / l で区切る。
// 割り算は double で取る (N < 2^53 なら floor が正しく出る。yosupo の N は 10^12 まで)。
inline std::vector<std::tuple<unsigned long long, unsigned long long, unsigned long long>>
enumerate_quotients_alt(unsigned long long N) {
 using u64= unsigned long long;
 std::vector<std::tuple<u64, u64, u64>> ret;
 if(!N) return ret;
 u64 s= std::sqrt((long double)N);
 while(s > 0 && s * s > N) --s;
 while((s + 1) <= N / (s + 1)) ++s;
 ret.reserve(2 * s + 1);
 // 小さい q は 1 .. floor(N/(s+1)) で、区間は (floor(N/(q+1)), floor(N/q)]。
 const u64 n= N / (s + 1);
 u64 prev= N;
 for(u64 q= 1; q <= n; ++q) {
  const u64 x= (u64)((double)N / (q + 1));
  ret.emplace_back(q, x, prev);
  prev= x;
 }
 // 残りは i = 1 .. s の 1 個ずつの区間で、q = floor(N/i)、区間は (i-1, i]。
 for(u64 l= s; l >= 1; --l) {
  ret.emplace_back((u64)((double)N / l), l - 1, l);
  if(l == 1) break;
 }
 return ret;
}
