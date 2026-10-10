#pragma once
// lp_ms の 2 MB 以上の表を mmap で取り、MADV_NOHUGEPAGE で 4 KB のページに留めたまま、MADV_POPULATE_WRITE で触る前に
// まとめて用意させるもの。lp_hp の速さが、ページフォールトの回数の差 (4 KB のページなら 32 MB の表で 8192 回) から来るのか、
// TLB の外れの差から来るのかを分けるために置く。前者なら lp_ms_hp に近づき、後者なら lp_ms と変わらない。表の形と探し方は
// lp と同じ。
#include <chrono>
#include <cstdint>
#include <cstdlib>
#ifdef __linux__
#include <sys/mman.h>
#endif
namespace aa_lp_ms_pf {
using u64= unsigned long long;
constexpr size_t H= size_t(1) << 21;
// 0 で埋まった領域を取る。
inline void* alloc_zero(size_t bytes) {
#ifdef __linux__
 if(bytes >= H) {
  void* p= mmap(nullptr, bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  madvise(p, bytes, MADV_NOHUGEPAGE);
  madvise(p, bytes, 23);  // MADV_POPULATE_WRITE
  return p;
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
 u64 mul, top;  // top はキー 2^64 - 1 の値
 Map(): t(static_cast<Slot*>(alloc_zero(16 * sizeof(Slot)))), shift(60), mask(15), n(0), lim(8), mul(splitmix64(std::chrono::steady_clock::now().time_since_epoch().count()) | 1), top(0) {}
 ~Map() { free_zero(t, (mask + 1) * sizeof(Slot)); }
 Map(const Map&)= delete;
 Map& operator=(const Map&)= delete;
 u64 get(u64 k) const {
  const u64 c= ~k;
  if(c == 0) [[unlikely]]
   return top;
  for(size_t i= (k * mul) >> shift;; i= (i + 1) & mask) {
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
  size_t i= (k * mul) >> shift;
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
    size_t i= (~old[j].k * mul) >> shift;
    while(t[i].k) i= (i + 1) & mask;
    t[i]= old[j];
   }
  free_zero(old, oc * sizeof(Slot));
 }
};
}
struct Solver {
 aa_lp_ms_pf::Map mp;
 void set(u64 k, u64 v) { mp.set(k, v); }
 u64 get(u64 k) const { return mp.get(k); }
};
