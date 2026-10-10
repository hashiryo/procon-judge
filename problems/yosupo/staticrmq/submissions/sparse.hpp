#pragma once
// sparse table を 1 本の配列に段ごとに並べたもの。段 k の i 番目は min(a[i], ..., a[i + 2^k - 1]) で、段 k + 1 は段 k と
// それを 2^k ずらしたものの min なので、8 個ずつ AVX2 の vpminud で作る。表は (floor(log2 N) + 1) * N 個で、
// N = 5 * 10^5 だと 38 MB になる。0 で埋める手間を省くため vector ではなく new で取る。クエリは長さの最上位の bit を
// k として、[l, l + 2^k) と [r - 2^k, r) の 2 つを引いて min を取る。分岐は無い。
#ifdef USE_SIMDE
#include <simde/x86/avx2.h>
#else
#include <immintrin.h>
#endif
#include <algorithm>
#include <bit>
#include <memory>
#include <vector>
namespace rmq_sparse {
using u32= unsigned;
struct Table {
 int n, lg;
 std::unique_ptr<u32[]> t;
 explicit Table(const std::vector<u32>& a): n(a.size()), lg(std::bit_width(unsigned(n)) - 1), t(new u32[size_t(lg + 1) * n]) {
  std::copy(a.begin(), a.end(), t.get());
  for(int k= 0; k < lg; ++k) {
   const u32* s= t.get() + size_t(k) * n;
   u32* d= t.get() + size_t(k + 1) * n;
   const int h= 1 << k, len= n - 2 * h + 1;
   int i= 0;
   for(; i + 8 <= len; i+= 8) _mm256_storeu_si256((__m256i*)(d + i), _mm256_min_epu32(_mm256_loadu_si256((const __m256i*)(s + i)), _mm256_loadu_si256((const __m256i*)(s + i + h))));
   for(; i < len; ++i) d[i]= std::min(s[i], s[i + h]);
  }
 }
 u32 query(int l, int r) const {
  const int k= std::bit_width(unsigned(r - l)) - 1;
  const u32* row= t.get() + size_t(k) * n;
  return std::min(row[l], row[r - (1 << k)]);
 }
};
}
struct Solver {
 rmq_sparse::Table tb;
 explicit Solver(const vector<u32>& a): tb(a) {}
 u32 query(int l, int r) const { return tb.query(l, r); }
};
