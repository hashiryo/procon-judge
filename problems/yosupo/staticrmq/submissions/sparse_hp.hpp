#pragma once
// sparse の表を huge page に置いたもの。表は 2 MB 境界で確保して MADV_HUGEPAGE を頼み、MADV_POPULATE_WRITE で
// 触る前にまとめて用意させる。38 MB の表を 4 KB のページで持つと、段を作るときに 1 万回ほどページフォールトが起き、
// ばらばらの位置を引くクエリは TLB を外す。2 MB のページなら 19 枚で済む。ほかは sparse と同じ。
#ifdef USE_SIMDE
#include <simde/x86/avx2.h>
#else
#include <immintrin.h>
#endif
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <algorithm>
#include <bit>
#include <cstdlib>
#include <memory>
#include <vector>
namespace rmq_sparse_hp {
using u32= unsigned;
struct FreeDeleter {
 void operator()(void* p) const { std::free(p); }
};
using Buf= std::unique_ptr<u32[], FreeDeleter>;
// 2 MB 境界で確保し、huge page を頼んでから、触る前にまとめて用意させる。
inline Buf alloc_huge(size_t n) {
 constexpr size_t H= size_t(1) << 21;
 const size_t bytes= (n * sizeof(u32) + H - 1) & ~(H - 1);
 void* p= std::aligned_alloc(H, bytes);
#ifdef __linux__
 madvise(p, bytes, MADV_HUGEPAGE);
 madvise(p, bytes, 23);  // MADV_POPULATE_WRITE
#endif
 return Buf(static_cast<u32*>(p));
}
struct Table {
 int n, lg;
 Buf t;
 explicit Table(const std::vector<u32>& a): n(a.size()), lg(std::bit_width(unsigned(n)) - 1), t(alloc_huge(size_t(lg + 1) * n)) {
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
 rmq_sparse_hp::Table tb;
 explicit Solver(const vector<u32>& a): tb(a) {}
 u32 query(int l, int r) const { return tb.query(l, r); }
};
