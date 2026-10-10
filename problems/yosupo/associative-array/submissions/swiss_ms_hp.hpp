#pragma once
// swiss のハッシュを lp_ms と同じ掛け算 1 回の形にし、印と組の配列を lp_hp と同じく huge page に置くもの。群は積の上位 bit
// で選び、印の下位 7 bit には群の bit のすぐ下の 7 bit を使う (積の下位 bit はキーの下位 bit だけで決まるので使わない)。
// 探し方は swiss と同じ。
#ifdef USE_SIMDE
#include <simde/x86/sse2.h>
#else
#include <immintrin.h>
#endif
#include <chrono>
#include <cstdint>
#include <cstdlib>
#ifdef __linux__
#include <sys/mman.h>
#endif
namespace aa_swiss_ms_hp {
using u64= unsigned long long;
using u8= unsigned char;
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
 static constexpr size_t G= 16;
 u8* ctrl;
 Slot* t;
 int shift;  // 群の番号はハッシュ >> shift
 size_t gmask, n, lim;
 u64 mul;
 Map(): ctrl(static_cast<u8*>(alloc_zero(32))), t(static_cast<Slot*>(alloc_zero(32 * sizeof(Slot)))), shift(63), gmask(1), n(0), lim(28), mul(splitmix64(std::chrono::steady_clock::now().time_since_epoch().count()) | 1) {}
 ~Map() {
  const size_t cap= (gmask + 1) * G;
  free_zero(ctrl, cap), free_zero(t, cap * sizeof(Slot));
 }
 Map(const Map&)= delete;
 Map& operator=(const Map&)= delete;
 u64 hash(u64 x) const { return x * mul; }
 // 印の下位 7 bit。群の番号の bit のすぐ下を使う。
 u8 tag(u64 h) const { return u8(0x80 | ((h >> (shift - 7)) & 0x7f)); }
 u64 get(u64 k) const {
  const u64 h= hash(k);
  const __m128i tg= _mm_set1_epi8(char(tag(h)));
  for(size_t g= h >> shift, step= 0;; g= (g + ++step) & gmask) {
   const __m128i c= _mm_loadu_si128(reinterpret_cast<const __m128i*>(ctrl + g * G));
   for(unsigned m= _mm_movemask_epi8(_mm_cmpeq_epi8(c, tg)); m; m&= m - 1) {
    const Slot& s= t[g * G + __builtin_ctz(m)];
    if(s.k == k) return s.v;
   }
   if(_mm_movemask_epi8(c) != 0xffff) return 0;
  }
 }
 void set(u64 k, u64 v) {
  const u64 h= hash(k);
  const u8 tb= tag(h);
  const __m128i tg= _mm_set1_epi8(char(tb));
  for(size_t g= h >> shift, step= 0;; g= (g + ++step) & gmask) {
   const __m128i c= _mm_loadu_si128(reinterpret_cast<const __m128i*>(ctrl + g * G));
   for(unsigned m= _mm_movemask_epi8(_mm_cmpeq_epi8(c, tg)); m; m&= m - 1) {
    Slot& s= t[g * G + __builtin_ctz(m)];
    if(s.k == k) {
     s.v= v;
     return;
    }
   }
   if(const unsigned e= ~unsigned(_mm_movemask_epi8(c)) & 0xffff) {
    const size_t i= g * G + __builtin_ctz(e);
    ctrl[i]= tb, t[i]= {k, v};
    if(++n > lim) grow();
    return;
   }
  }
 }
 // 無いと分かっているキーを入れる。
 void insert(const Slot& s) {
  const u64 h= hash(s.k);
  for(size_t g= h >> shift, step= 0;; g= (g + ++step) & gmask) {
   const __m128i c= _mm_loadu_si128(reinterpret_cast<const __m128i*>(ctrl + g * G));
   if(const unsigned e= ~unsigned(_mm_movemask_epi8(c)) & 0xffff) {
    const size_t i= g * G + __builtin_ctz(e);
    ctrl[i]= tag(h), t[i]= s;
    return;
   }
  }
 }
 void grow() {
  u8* oc= ctrl;
  Slot* ot= t;
  const size_t ocap= (gmask + 1) * G, ncap= ocap * 2;
  ctrl= static_cast<u8*>(alloc_zero(ncap));
  t= static_cast<Slot*>(alloc_zero(ncap * sizeof(Slot)));
  gmask= ncap / G - 1, --shift, lim= ncap / 8 * 7;
  for(size_t j= 0; j < ocap; ++j)
   if(oc[j]) insert(ot[j]);
  free_zero(oc, ocap), free_zero(ot, ocap * sizeof(Slot));
 }
};
}
struct Solver {
 aa_swiss_ms_hp::Map mp;
 void set(u64 k, u64 v) { mp.set(k, v); }
 u64 get(u64 k) const { return mp.get(k); }
};
