#pragma once
// 診断用。整列の指定の無い operator new と delete を差し替えて、2 MB 以上の確保が載るページを選ぶ。PAGES_HP なら 2 MB 境界の
// mmap に置いて MADV_HUGEPAGE を頼み、PAGES_4K なら mmap に置いて MADV_NOHUGEPAGE で 4 KB のページに留める。2 MB 未満は
// malloc に回す。Linux でなければ何もしない。atcoder-abc155-e の診断 (9c7b4e18、208682d0 で消した) で使った形を戻したもの。
// NeoLibrary の MaxFlow、MinCostFlow、PrimalDual の網の配列を 2 MB の区画に置くと速くなるかの上限を見るため、メモリの大きい
// 6 問 (cf-1630-f、luogu-P4001、luogu-P4177、self-flow-maxflow-shapes、self-flow-mincostflow、self-flow-mincostflow-slope) の
// lib-neo-hp が使う。CI の x64 のランナーは透過的な huge page が madvise なので、頼まない配列は 4 KB のページで、初めて触る
// たびに OS がページを用意し、TLB にも収まらない。入力を読む配列など、ライブラリの外の 2 MB 以上の確保も同じく載せ替わる。
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <new>
#ifdef __linux__
#include <sys/mman.h>
#endif
namespace pages_plain_internal {
constexpr std::size_t H= std::size_t(1) << 21, P= 4096;
struct Block {
 void* p;
 std::size_t bytes;
};
inline Block blocks[64];
inline int nb= 0;
inline void* map([[maybe_unused]] std::size_t n) {
#ifdef __linux__
 const std::size_t bytes= (n + P - 1) & ~(P - 1);
 if(nb == 64) return nullptr;
#if defined(PAGES_HP)
 void* m= mmap(nullptr, bytes + H, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
 if(m == MAP_FAILED) return nullptr;
 char *p= static_cast<char*>(m), *a= reinterpret_cast<char*>((reinterpret_cast<std::uintptr_t>(p) + H - 1) & ~(H - 1));
 if(a != p) munmap(p, a - p);
 munmap(a + bytes, p + H - a);
 madvise(a, bytes, MADV_HUGEPAGE);
#elif defined(PAGES_4K)
 void* m= mmap(nullptr, bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
 if(m == MAP_FAILED) return nullptr;
 char* a= static_cast<char*>(m);
 madvise(a, bytes, MADV_NOHUGEPAGE);
#else
 return nullptr;
#endif
 blocks[nb++]= {a, bytes};
 return a;
#else
 return nullptr;
#endif
}
inline bool unmap(void* p) {
 for(int i= 0; i < nb; ++i)
  if(blocks[i].p == p) {
#ifdef __linux__
   munmap(p, blocks[i].bytes);
#endif
   blocks[i]= blocks[--nb];
   return true;
  }
 return false;
}
// 差し替えた operator new は 2 MB 未満を malloc で取るので、free で返すのが正しい。g++ は operator new が差し替えられたことを
// 見ずに -Wmismatched-new-delete を出すので、ここだけ抑止する。
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmismatched-new-delete"
#endif
inline void release(void* p) {
 if(p && !unmap(p)) std::free(p);
}
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
}  // namespace pages_plain_internal
void* operator new(std::size_t n) {
 if(n >= pages_plain_internal::H)
  if(void* p= pages_plain_internal::map(n)) return p;
 if(void* p= std::malloc(n ? n : 1)) return p;
 throw std::bad_alloc();
}
void* operator new[](std::size_t n) { return ::operator new(n); }
void operator delete(void* p) noexcept { pages_plain_internal::release(p); }
void operator delete(void* p, std::size_t) noexcept { pages_plain_internal::release(p); }
void operator delete[](void* p) noexcept { pages_plain_internal::release(p); }
void operator delete[](void* p, std::size_t) noexcept { pages_plain_internal::release(p); }
