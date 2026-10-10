#pragma once
// lp_hp のハッシュから、splitmix64 の最後の xorshift (x ^= x >> 31) を省いたもの。最後の xorshift が変えるのは
// 下位 33 bit だけで、位置は上位 bit で決めるので、表が 2^31 組以下の間は位置が変わらない。掛け算 2 回の鎖の後ろの
// 2 命令だけが減る。表の形と探し方、huge page の取り方は lp_hp と同じ。
#include <chrono>
#include <cstdint>
#include <cstdlib>
#ifdef __linux__
#include <sys/mman.h>
#endif
namespace aa_lp_hp_trim {
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
struct Map {
 struct Slot {
  u64 k, v;
 };
 Slot* t;
 int shift;
 size_t mask, n, lim;
 u64 seed, top;  // top はキー 2^64 - 1 の値
 Map(): t(static_cast<Slot*>(alloc_zero(16 * sizeof(Slot)))), shift(60), mask(15), n(0), lim(8), seed(std::chrono::steady_clock::now().time_since_epoch().count()), top(0) {}
 ~Map() { free_zero(t, (mask + 1) * sizeof(Slot)); }
 Map(const Map&)= delete;
 Map& operator=(const Map&)= delete;
 u64 hash(u64 x) const {
  x+= seed;
  x= (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ull;
  return (x ^ (x >> 27)) * 0x94d049bb133111ebull;
 }
 u64 get(u64 k) const {
  const u64 c= ~k;
  if(c == 0) [[unlikely]]
   return top;
  for(size_t i= hash(k) >> shift;; i= (i + 1) & mask) {
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
  size_t i= hash(k) >> shift;
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
    size_t i= hash(~old[j].k) >> shift;
    while(t[i].k) i= (i + 1) & mask;
    t[i]= old[j];
   }
  free_zero(old, oc * sizeof(Slot));
 }
};
}
struct Solver {
 aa_lp_hp_trim::Map mp;
 void set(u64 k, u64 v) { mp.set(k, v); }
 u64 get(u64 k) const { return mp.get(k); }
};
