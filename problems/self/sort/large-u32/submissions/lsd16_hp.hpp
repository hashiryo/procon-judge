#pragma once
// lsd16 (16 bit × 2 回) の振り分けを、huge page の作業用の配列 2 つ (b と c) に書き、最後に a へまとめて写す版。
// 16 bit の桁は書き先が 65536 か所に散らばるので、4 KB のページでは TLB の外れが 11 bit の桁よりずっと多い。
// lsd_adaptive_hp で huge page が 2 割効いたので、振り分けが 1 回少ない 16 bit の桁が逆転するかを見る。
// 4096 個以下は std::sort に任せる。Linux 以外では huge page を頼まない。
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>
namespace sort_lsd16_hp {
using u32= unsigned;
struct FreeDeleter {
 void operator()(void* p) const { std::free(p); }
};
// 2 MB 境界で確保し、huge page を頼んでから、触る前にまとめて用意させる。
inline std::unique_ptr<u32, FreeDeleter> alloc_huge(size_t n) {
 constexpr size_t H= size_t(1) << 21;
 const size_t bytes= (n * sizeof(u32) + H - 1) & ~(H - 1);
 void* p= std::aligned_alloc(H, bytes);
#ifdef __linux__
 madvise(p, bytes, MADV_HUGEPAGE);
 madvise(p, bytes, 23);  // MADV_POPULATE_WRITE
#endif
 return std::unique_ptr<u32, FreeDeleter>(static_cast<u32*>(p));
}
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
 auto bbuf= alloc_huge(n), cbuf= alloc_huge(n);
 u32 *b= bbuf.get(), *c= cbuf.get();
 for(size_t i= 0; i < n; ++i) {
  u32 x= a[i];
  b[c0[x & 0xFFFF]++]= x;
 }
 for(size_t i= 0; i < n; ++i) {
  u32 x= b[i];
  c[c1[x >> 16]++]= x;
 }
 std::memcpy(a, c, n * sizeof(u32));
}
}  // namespace sort_lsd16_hp
inline void run(std::vector<unsigned>& a) { sort_lsd16_hp::sort(a); }
