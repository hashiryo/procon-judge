// 文字列の基本の道具の族 (self/string/) の提出が使う allocator。2 MB 以上の確保を Linux では 2 MB 境界の mmap に置いて
// MADV_HUGEPAGE を頼む (wavelet の HugeAlloc、NeoLibrary の OrderedSet と同じ形)。飛び飛びに読む大きい表で、4 KB のページの
// TLB の取り違えを減らすためのもの。
#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <vector>
#ifdef __linux__
#include <sys/mman.h>
#endif
namespace string_hp {
template <class T> struct HugeAlloc {
 using value_type= T;
 static constexpr std::size_t H= std::size_t(1) << 21, P= 4096;
 HugeAlloc()= default;
 template <class U> HugeAlloc(const HugeAlloc<U>&) {}
 T* allocate(std::size_t n) {
#ifdef __linux__
  if(n * sizeof(T) >= H) {
   const std::size_t bytes= (n * sizeof(T) + P - 1) & ~(P - 1);
   void* m= mmap(nullptr, bytes + H, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   if(m == MAP_FAILED) throw std::bad_alloc();
   char *p= static_cast<char*>(m), *a= reinterpret_cast<char*>((reinterpret_cast<std::uintptr_t>(p) + H - 1) & ~(H - 1));
   if(a != p) munmap(p, a - p);
   munmap(a + bytes, p + H - a);
   madvise(a, bytes, MADV_HUGEPAGE);
   return reinterpret_cast<T*>(a);
  }
#endif
  return std::allocator<T>().allocate(n);
 }
 void deallocate(T* p, std::size_t n) {
#ifdef __linux__
  if(n * sizeof(T) >= H) {
   munmap(p, (n * sizeof(T) + P - 1) & ~(P - 1));
   return;
  }
#endif
  std::allocator<T>().deallocate(p, n);
 }
 template <class U> bool operator==(const HugeAlloc<U>&) const { return true; }
 template <class U> bool operator!=(const HugeAlloc<U>&) const { return false; }
};
template <class T> using Vec= std::vector<T, HugeAlloc<T>>;
}  // namespace string_hp
