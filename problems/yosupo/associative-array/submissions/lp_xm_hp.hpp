#pragma once
// lp_xm (乱数と xor してから黄金比の奇数を掛けるハッシュ) の表を、lp_hp と同じく huge page に置くもの。掛け算 1 回で
// 済み、連続する整数と 2 の冪の倍数では衝突しない。公差がフィボナッチ数の等差数列では、まれに探す長さが延びる。
// 表の形と探し方は lp と同じ。
#include <chrono>
#include <cstdint>
#include <cstdlib>
#ifdef __linux__
#include <sys/mman.h>
#endif
namespace aa_lp_xm_hp {
using u64= unsigned long long;
constexpr size_t H= size_t(1) << 21;
// 0 で埋まった領域を取る。
inline void* alloc_zero(size_t bytes) {
#ifdef __linux__
 if(bytes >= H) {
  char* p= static_cast<char*>(mmap(nullptr, bytes + H, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
  char* a= reinterpret_cast<char*>((reinterpret_cast<uintptr_t>(p) + H - 1) & ~(H - 1));
  if(a != p) munmap(p, a - p);
  munmap(a + bytes, p + H - a);
  madvise(a, bytes, MADV_HUGEPAGE);
  return a;
 }
#endif
 return std::calloc(bytes, 1);
}
inline void free_zero(void* p, [[maybe_unused]] size_t bytes) {
#ifdef __linux__
 if(bytes >= H) return void(munmap(p, bytes));
#endif
 std::free(p);
}
inline u64 splitmix64(u64 x) {
 x+= 0x9e3779b97f4a7c15ull;
 x= (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ull;
 x= (x ^ (x >> 27)) * 0x94d049bb133111ebull;
 return x ^ (x >> 31);
}
struct Map {
 struct Slot {
  u64 k, v;
 };
 Slot* t;
 int shift;
 size_t mask, n, lim;
 u64 r, top;  // top はキー 2^64 - 1 の値
 Map(): t(static_cast<Slot*>(alloc_zero(16 * sizeof(Slot)))), shift(60), mask(15), n(0), lim(8), r(splitmix64(std::chrono::steady_clock::now().time_since_epoch().count())), top(0) {}
 ~Map() { free_zero(t, (mask + 1) * sizeof(Slot)); }
 Map(const Map&)= delete;
 Map& operator=(const Map&)= delete;
 u64 get(u64 k) const {
  const u64 c= ~k;
  if(c == 0) [[unlikely]]
   return top;
  for(size_t i= ((k ^ r) * 0x9e3779b97f4a7c15ull) >> shift;; i= (i + 1) & mask) {
   if(t[i].k == c) return t[i].v;
   if(t[i].k == 0) return 0;
  }
 }
 void set(u64 k, u64 v) {
  const u64 c= ~k;
  if(c == 0) [[unlikely]] {
   top= v;
   return;
  }
  size_t i= ((k ^ r) * 0x9e3779b97f4a7c15ull) >> shift;
  for(; t[i].k != 0; i= (i + 1) & mask)
   if(t[i].k == c) {
    t[i].v= v;
    return;
   }
  t[i]= {c, v};
  if(++n > lim) grow();
 }
 void grow() {
  Slot* old= t;
  const size_t oc= mask + 1, nc= oc * 2;
  t= static_cast<Slot*>(alloc_zero(nc * sizeof(Slot)));
  mask= nc - 1, --shift, lim= nc / 2;
  for(size_t j= 0; j < oc; ++j)
   if(old[j].k) {
    size_t i= ((~old[j].k ^ r) * 0x9e3779b97f4a7c15ull) >> shift;
    while(t[i].k) i= (i + 1) & mask;
    t[i]= old[j];
   }
  free_zero(old, oc * sizeof(Slot));
 }
};
}
struct Solver {
 aa_lp_xm_hp::Map mp;
 void set(u64 k, u64 v) { mp.set(k, v); }
 u64 get(u64 k) const { return mp.get(k); }
};
