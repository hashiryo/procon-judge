#pragma once
// 診断用。整列付きの operator new と delete を差し替えて、alignas(64) の型の配列のうち 2 MB 以上のものが載るページを選ぶ。
// NeoLibrary の OrderedSet の葉と節点 (alignas(64)) の vector はこの new で取られる。ハーネスと提出のほかの vector は
// 既定の整列なので使わない。PAGES_HP なら 2 MB 境界の mmap に置いて MADV_HUGEPAGE を頼み、PAGES_4K なら mmap に置いて
// MADV_NOHUGEPAGE で 4 KB のページに留める。x64-gcc で OrderedSet<false> が <true> より遅く出たのが、透過的な huge page の
// 当たり外れから来ているかを見るために置いた。Linux でなければ何もしない。
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <new>
#ifdef __linux__
#include <sys/mman.h>
#endif
namespace pages_internal {
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
}
void* operator new(std::size_t n, std::align_val_t al) {
 if(n >= pages_internal::H)
  if(void* p= pages_internal::map(n)) return p;
 const std::size_t a= std::size_t(al);
 if(void* p= std::aligned_alloc(a, (n + a - 1) / a * a)) return p;
 throw std::bad_alloc();
}
void operator delete(void* p, std::align_val_t) noexcept {
 if(!pages_internal::unmap(p)) std::free(p);
}
void operator delete(void* p, std::size_t, std::align_val_t) noexcept {
 if(!pages_internal::unmap(p)) std::free(p);
}
