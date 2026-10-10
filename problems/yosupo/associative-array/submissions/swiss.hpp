#pragma once
// Swiss table の形の開番地法。組ごとに 1 byte の印 (0 は空き、使用中は 0x80 | ハッシュの下位 7 bit) を持ち、16 個の
// 組をまとめて 1 つの群にする。引くときは、群の 16 byte の印を SSE2 で 1 回に比べ、下位 7 bit が合う組だけキーを
// 比べる。群に空きがあれば、そこで無いと決まる。群はハッシュの上位 bit で選び、見つからなければ 1, 2, 3, ... と
// 間隔を広げて次の群へ進む (群の数が 2 の冪なら全部を回る)。埋まりが 7/8 を超えたら 2 倍にする。ハッシュは lp と同じ。
#ifdef USE_SIMDE
#include <simde/x86/sse2.h>
#else
#include <immintrin.h>
#endif
#include <chrono>
#include <cstdlib>
namespace aa_swiss {
using u64= unsigned long long;
using u8= unsigned char;
struct Map {
 struct Slot {
  u64 k, v;
 };
 static constexpr size_t G= 16;
 u8* ctrl;
 Slot* t;
 int shift;  // 群の番号はハッシュ >> shift
 size_t gmask, n, lim;
 u64 seed;
 Map(): ctrl(static_cast<u8*>(std::calloc(32, 1))), t(static_cast<Slot*>(std::malloc(32 * sizeof(Slot)))), shift(63), gmask(1), n(0), lim(28), seed(std::chrono::steady_clock::now().time_since_epoch().count()) {}
 ~Map() { std::free(ctrl), std::free(t); }
 Map(const Map&)= delete;
 Map& operator=(const Map&)= delete;
 u64 hash(u64 x) const {
  x+= seed;
  x= (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ull;
  x= (x ^ (x >> 27)) * 0x94d049bb133111ebull;
  return x ^ (x >> 31);
 }
 u64 get(u64 k) const {
  const u64 h= hash(k);
  const __m128i tag= _mm_set1_epi8(char(0x80 | (h & 0x7f)));
  for(size_t g= h >> shift, step= 0;; g= (g + ++step) & gmask) {
   const __m128i c= _mm_loadu_si128(reinterpret_cast<const __m128i*>(ctrl + g * G));
   for(unsigned m= _mm_movemask_epi8(_mm_cmpeq_epi8(c, tag)); m; m&= m - 1) {
    const Slot& s= t[g * G + __builtin_ctz(m)];
    if(s.k == k) return s.v;
   }
   if(_mm_movemask_epi8(c) != 0xffff) return 0;
  }
 }
 void set(u64 k, u64 v) {
  const u64 h= hash(k);
  const u8 tg= 0x80 | (h & 0x7f);
  const __m128i tag= _mm_set1_epi8(char(tg));
  for(size_t g= h >> shift, step= 0;; g= (g + ++step) & gmask) {
   const __m128i c= _mm_loadu_si128(reinterpret_cast<const __m128i*>(ctrl + g * G));
   for(unsigned m= _mm_movemask_epi8(_mm_cmpeq_epi8(c, tag)); m; m&= m - 1) {
    Slot& s= t[g * G + __builtin_ctz(m)];
    if(s.k == k) {
     s.v= v;
     return;
    }
   }
   if(const unsigned e= ~unsigned(_mm_movemask_epi8(c)) & 0xffff) {
    const size_t i= g * G + __builtin_ctz(e);
    ctrl[i]= tg, t[i]= {k, v};
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
    ctrl[i]= u8(0x80 | (h & 0x7f)), t[i]= s;
    return;
   }
  }
 }
 void grow() {
  u8* oc= ctrl;
  Slot* ot= t;
  const size_t ocap= (gmask + 1) * G, ncap= ocap * 2;
  ctrl= static_cast<u8*>(std::calloc(ncap, 1));
  t= static_cast<Slot*>(std::malloc(ncap * sizeof(Slot)));
  gmask= ncap / G - 1, --shift, lim= ncap / 8 * 7;
  for(size_t j= 0; j < ocap; ++j)
   if(oc[j]) insert(ot[j]);
  std::free(oc), std::free(ot);
 }
};
}
struct Solver {
 aa_swiss::Map mp;
 void set(u64 k, u64 v) { mp.set(k, v); }
 u64 get(u64 k) const { return mp.get(k); }
};
