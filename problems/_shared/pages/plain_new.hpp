#pragma once
// 診断用。整列の指定の無い operator new と delete を差し替えて、2 MB 以上の確保が載るページを選ぶ。PAGES_HP なら 2 MB 境界の
// mmap に置いて MADV_HUGEPAGE を頼み、PAGES_4K なら mmap に置いて MADV_NOHUGEPAGE で 4 KB のページに留める。どちらも同じ
// mmap の道を通るので、2 つの差はページの大きさの分になる。2 MB 未満は malloc に回す。Linux でなければ何もしない。
// atcoder-abc155-e の lib-neo-hp と lib-neo-4k が使う。NeoLibrary の DFA の表 (200 万状態 × 19 記号の int、152 MB) は
// vector を倍々に伸ばすので、確保し直すたびに新しい領域へ写して、その領域のページに初めて触る。その分を見るために置いた。
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
inline void release(void* p) {
 if(p && !unmap(p)) std::free(p);
}
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
